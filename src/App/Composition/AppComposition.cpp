// Copyright (c) 2026 Bruce Yan. All Rights Reserved.

/**
 * @brief GameState と platform adapter を組み立てて実行可能なアプリを作る
 */

#include "src/App/Assets/MapJsonLoader.h"
#include "src/App/Composition/AppComposition.h"
#include "src/App/Runtime/GameApp.h"

#include "src/Core/Game/GameModel.h"
#include "src/Core/Ports/PresenterPort.h"
#include "src/Core/Presentation/Presenter.h"
#include "src/Core/Render/CellBuffer.h"
#include "src/Core/World/TileMap.h"

#include "src/Platform/Clock/SteadyClockAdapter.h"
#include "src/Platform/Console/ConsoleSession.h"
#include "src/Platform/Win32Console/Win32ConsoleInputAdapter.h"
#include "src/Platform/Win32Console/Win32ConsoleRenderAdapter.h"

#include <algorithm>
#include <cstddef>
#include <exception>
#include <filesystem> // std::filesystem::path
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace app
{
    namespace composition
    {
        namespace
        {
            const std::filesystem::path kPlayerFile = "Player.json";
            const std::filesystem::path kIntroFile = "Intro.json";
            const std::filesystem::path kGameMapFile = "Map.json";
            const std::filesystem::path kGameMapBackgroundFile = "MapBackground.json";
            const std::filesystem::path kDiedFile = "Died.json";
            const std::filesystem::path kFinFile = "Fin.json";

            constexpr int kPlayerMaxLives = 5;
            constexpr int kEnemyChaseRange = 8;
            constexpr float kEnemyMoveSpeed = 2.0f;

            enum class LayerMarkerKind
            {
                Start,
                End,
            };

            struct LayerMarker final
            {
                LayerMarkerKind kind_{ LayerMarkerKind::Start };
                int             y_{ 0 };
            };

            void initCoreDefaults(core::game::GameState& initial) noexcept
            {
                initial.scene_.current_ = core::game::GameScene::Intro;
                initial.goal_.bIsClear_ = false;

                initial.player_.maxLives_ = kPlayerMaxLives;
                initial.player_.currLives_ = initial.player_.maxLives_;

                initial.goal_.lastLitIndex_ = -1;
            }

            std::vector<LayerMarker> collectLayerMarkers(core::world::TileMap& tileMap)
            {
                std::vector<LayerMarker> markers{};

                const int width = tileMap.getWidth();
                const int height = tileMap.getHeight();

                for (int worldY = 0; worldY < height; ++worldY)
                {
                    for (int worldX = 0; worldX < width; ++worldX)
                    {
                        const core::types::Vec2 currCell{ worldX, worldY };
                        const auto              tileId = tileMap.getTileIdByCell(currCell);

                        if (tileId == core::world::tileid::kLayerStart)
                        {
                            markers.push_back(LayerMarker{ LayerMarkerKind::Start, worldY });
                            tileMap.setTileIdAtCell(currCell, core::world::tileid::kSpace);
                        }
                        else if (tileId == core::world::tileid::kLayerEnd)
                        {
                            markers.push_back(LayerMarker{ LayerMarkerKind::End, worldY });
                            tileMap.setTileIdAtCell(currCell, core::world::tileid::kSpace);
                        }
                    }
                }

                return markers;
            }

            /**
             * @brief 1 レイヤにとって、スタートが下、エンドが上
             * マップの座標系には 0 座標が一番上ので
             */
            void sortLayerMarkersForBottomUpPairing(std::vector<LayerMarker>& markers)
            {
                std::sort(
                    markers.begin(),
                    markers.end(),
                    [](const LayerMarker& lhs, const LayerMarker& rhs)
                    {
                        return lhs.y_ > rhs.y_;
                    });
            }

            std::vector<core::game::GoalActivationRegion> buildGoalActivationRegions(
                const std::vector<LayerMarker>& markers)
            {
                const std::size_t expectedLayerCount = markers.size() / std::size_t{ 2 };

                if (markers.size() != expectedLayerCount * std::size_t{ 2 })
                {
                    throw std::runtime_error("Layer marker count mismatch.");
                }

                std::vector<core::game::GoalActivationRegion> goalRegions{};
                goalRegions.reserve(expectedLayerCount);

                bool bHasStart = false;
                int  startY = 0;

                for (const LayerMarker& marker : markers)
                {
                    if (marker.kind_ == LayerMarkerKind::Start)
                    {
                        if (bHasStart)
                        {
                            throw std::runtime_error("Layer start marker is not closed.");
                        }

                        bHasStart = true;
                        startY = marker.y_;
                        continue;
                    }

                    if (!bHasStart)
                    {
                        throw std::runtime_error("Layer end marker appears before a start marker.");
                    }

                    if (marker.y_ >= startY)
                    {
                        throw std::runtime_error("Layer end marker is below its start marker.");
                    }

                    core::game::GoalActivationRegion goalRegion{ marker.y_, startY, {} };

                    if (!goalRegions.empty())
                    {
                        const auto& previousRegion = goalRegions.back();
                        if (goalRegion.endY_ >= previousRegion.beginY_)
                        {
                            throw std::runtime_error("Layer ranges must not overlap.");
                        }
                    }

                    goalRegions.push_back(goalRegion);
                    bHasStart = false;
                }

                if (bHasStart)
                {
                    throw std::runtime_error("Layer start marker is missing an end marker.");
                }

                if (goalRegions.size() != expectedLayerCount)
                {
                    throw std::runtime_error("Goal activation region count mismatch.");
                }

                return goalRegions;
            }

            void initGoalActivationRegions(core::game::GameState& initial)
            {
                auto markers = collectLayerMarkers(initial.assets_.gameScene_.tileMap_);
                sortLayerMarkersForBottomUpPairing(markers);

                initial.goal_.goalRegions_ = buildGoalActivationRegions(markers);
                initial.goal_.goalLitFlags_.assign(initial.goal_.goalRegions_.size(), false);
            }

            std::vector<core::types::Vec2> collectGoalCells(const core::world::TileMap& tileMap) noexcept
            {
                std::vector<core::types::Vec2> goalCells{};

                const int width = tileMap.getWidth();
                const int height = tileMap.getHeight();

                for (int worldY = 0; worldY < height; ++worldY)
                {
                    for (int worldX = 0; worldX < width; ++worldX)
                    {
                        const core::types::Vec2 currCell{ worldX, worldY };

                        if (tileMap.getTileIdByCell(currCell) == core::world::tileid::kGoal)
                        {
                            goalCells.push_back(currCell);
                        }
                    }
                }

                return goalCells;
            }

            void initGoals(core::game::GameState& initial)
            {
                const std::vector<core::types::Vec2> goalCells = collectGoalCells(initial.assets_.gameScene_.tileMap_);

                std::size_t goalRegionIndex = 0;
                auto& goalRegions = initial.goal_.goalRegions_;

                for (; goalRegionIndex < goalRegions.size(); ++goalRegionIndex)
                {
                    auto& goalRegion = goalRegions[goalRegionIndex];

                    core::types::Vec2 foundGoalCell{ 0, 0 };
                    int               matchCount = 0;

                    for (const auto& goalCell : goalCells)
                    {
                        if ((goalCell.y_ >= goalRegion.beginY_) && (goalCell.y_ <= goalRegion.endY_))
                        {
                            foundGoalCell = goalCell;
                            ++matchCount;
                        }
                    }

                    // 最後の goal region は見た目のため複数の Goal tile を許可する
                    // その場合は走査で最後に見つかった Goal tile を操作位置にする
                    if (matchCount != 1 && !(goalRegionIndex == goalRegions.size() - 1))
                    {
                        throw std::runtime_error(
                            std::string{ "Goal count mismatch goals=" }
                            +
                            std::to_string(goalCells.size())
                            +
                            " goalRegions="
                            + std::to_string(goalRegions.size())
                        );
                    }

                    goalRegion.goalWorldCell_ = foundGoalCell;
                }
            }

            core::types::Vec2 findPlayerSpawnCell(const core::world::TileMap& tileMap)
            {
                const int width = tileMap.getWidth();
                const int height = tileMap.getHeight();

                core::types::Vec2 found{ 0, 0 };
                int               matchCount = 0;

                for (int worldY = 0; worldY < height; ++worldY)
                {
                    for (int worldX = 0; worldX < width; ++worldX)
                    {
                        const core::types::Vec2 currCell{ worldX, worldY };

                        if (tileMap.getTileIdByCell(currCell) == core::world::tileid::kPlayerSpawn)
                        {
                            found = currCell;
                            ++matchCount;
                        }
                    }
                }

                if (matchCount != 1)
                {
                    throw std::runtime_error("PlayerSpawn mismatch.");
                }

                return found;
            }

            void initPlayerPosition(core::game::GameState& initial)
            {
                initial.player_.spawnCell_ = findPlayerSpawnCell(initial.assets_.gameScene_.tileMap_);
                initial.assets_.gameScene_.tileMap_.setTileIdAtCell(initial.player_.spawnCell_, core::world::tileid::kSpace);
                initial.player_.feetWorldPos_ = core::types::Vec2f{
                    static_cast<float>(initial.player_.spawnCell_.x_), static_cast<float>(initial.player_.spawnCell_.y_) };
            }

            void initEnemies(core::game::GameState& initial) noexcept
            {
                initial.enemyRuntime_.chaseRange_ = kEnemyChaseRange;
                initial.enemyRuntime_.moveSpeed_ = kEnemyMoveSpeed;

                std::vector<core::types::Vec2> enemySpawnCells{};

                const int width = initial.assets_.gameScene_.tileMap_.getWidth();
                const int height = initial.assets_.gameScene_.tileMap_.getHeight();

                for (int worldY = 0; worldY < height; ++worldY)
                {
                    for (int worldX = 0; worldX < width; ++worldX)
                    {
                        const core::types::Vec2 currCell{ worldX, worldY };

                        if (initial.assets_.gameScene_.tileMap_.getTileIdByCell(currCell)
                            == core::world::tileid::kEnemySpawn)
                        {
                            enemySpawnCells.push_back(currCell);

                            initial.assets_.gameScene_.tileMap_.setTileIdAtCell(currCell, core::world::tileid::kSpace);
                        }
                    }
                }

                initial.enemyRuntime_.enemies_.clear();
                initial.enemyRuntime_.enemies_.reserve(enemySpawnCells.size());

                for (std::size_t i = 0; i < enemySpawnCells.size(); ++i)
                {
                    const auto& enemySpawnCell = enemySpawnCells[i];

                    core::game::Enemy enemy{};

                    enemy.worldPos_ =
                        core::types::Vec2f{ static_cast<float>(enemySpawnCell.x_), static_cast<float>(enemySpawnCell.y_) };
                    enemy.velocity_ = core::types::Vec2f{ 0.0f, 0.0f };

                    enemy.state_ = core::game::EnemyState::Patrol;
                    enemy.prevState_ = enemy.state_;

                    enemy.alertRemainingTicks_ = 0;

                    enemy.bGrounded_ = false;

                    enemy.alert_ = core::game::EnemyAlert::None;

                    enemy.patrolAnchorX_ = enemySpawnCell.x_;

                    enemy.patrolDirX_ = 1;

                    initial.enemyRuntime_.enemies_.push_back(enemy);
                }
            }

            bool tryLoadMapAsset(const std::filesystem::path& fileName,
                core::world::TileMap& loadTargetTileMap,
                core::game::TileColorPalette& loadTargetPalette)
            {
                try
                {
                    const auto asset = app::assets::loadMapAssetFromJson(fileName);
                    loadTargetTileMap = asset.tileMap_;
                    loadTargetPalette = asset.colorPalette_;
                    return true;
                }
                catch (const std::exception& e)
                {
                    std::cerr << "Failed to load. File: " << fileName.string() << '\n';
                    std::cerr << "Error: " << e.what() << "\n";
                    std::cerr << "CWD: " << std::filesystem::current_path().string() << "\n";
                    return false;
                }
            }

            bool tryLoadGameAssets(core::game::GameState& initial) noexcept
            {
                if (!tryLoadMapAsset(kPlayerFile, initial.assets_.player_.tileMap_, initial.assets_.player_.colorPalette_))
                {
                    return false;
                }

                if (!tryLoadMapAsset(
                    kIntroFile, initial.assets_.introScene_.tileMap_, initial.assets_.introScene_.colorPalette_))
                {
                    return false;
                }

                if (!tryLoadMapAsset(
                    kGameMapFile, initial.assets_.gameScene_.tileMap_, initial.assets_.gameScene_.colorPalette_))
                {
                    return false;
                }

                if (!tryLoadMapAsset(kGameMapBackgroundFile, initial.assets_.gameSceneBackground_.tileMap_,
                    initial.assets_.gameSceneBackground_.colorPalette_))
                {
                    return false;
                }

                if (!tryLoadMapAsset(
                    kDiedFile, initial.assets_.diedScene_.tileMap_, initial.assets_.diedScene_.colorPalette_))
                {
                    return false;
                }

                if (!tryLoadMapAsset(
                    kFinFile, initial.assets_.finScene_.tileMap_, initial.assets_.finScene_.colorPalette_))
                {
                    return false;
                }

                return true;
            }

            void goalWaveEffectInitialization(core::game::GameState& initial) noexcept
            {
                initial.goal_.wave_.durationSeconds_ = 1.5f;
                initial.goal_.wave_.ringThicknessCells_ = 2;
            }

            void goalWaveEffectRulesInitialization(core::game::GameState& initial) noexcept
            {
                using core::game::GoalWaveRule;

                initial.goal_.waveRules_.clear();
                initial.goal_.waveRules_.reserve(16);

                initial.goal_.waveRules_.push_back(GoalWaveRule{ 
                    core::world::tileid::kTrunk, true, true,
                    core::render::Color::LightRed,
                    core::render::Color::Yellow });

                initial.goal_.waveRules_.push_back(GoalWaveRule{ 
                    core::world::tileid::kBranch, true, true,
                    core::render::Color::LightRed, 
                    core::render::Color::Yellow });

                initial.goal_.waveRules_.push_back(GoalWaveRule{ 
                    core::world::tileid::kRayTop, true, true,
                    core::render::Color::LightRed, 
                    core::render::Color::Yellow });

                initial.goal_.waveRules_.push_back(GoalWaveRule{ 
                    core::world::tileid::kRayDown, true, true,
                    core::render::Color::LightRed, 
                    core::render::Color::Yellow });

                initial.goal_.waveRules_.push_back(GoalWaveRule{
                    core::world::tileid::kSun, true, true,
                    core::render::Color::LightRed, core::render::Color::Yellow });

                initial.goal_.waveRules_.push_back(GoalWaveRule{ 
                    core::world::tileid::kSpace, true, true,
                    core::render::Color::LightRed, 
                    core::render::Color::Yellow });
            }

            void initGoalWaveEffect(core::game::GameState& initial) noexcept
            {
                const auto& backgroundTileMap = initial.assets_.gameSceneBackground_.tileMap_;

                // Goal がアクティブになると、色の変化
                initial.goal_.paintOverrides_.assign(
                    static_cast<std::size_t>(backgroundTileMap.getWidth() * backgroundTileMap.getHeight()),
                    core::game::PaintOverride{}
                );

                goalWaveEffectInitialization(initial);

                goalWaveEffectRulesInitialization(initial);

                initial.goal_.wavePaintOverrides_.assign(
                    static_cast<std::size_t>(backgroundTileMap.getWidth()
                        *
                        backgroundTileMap.getHeight()),
                    core::game::PaintOverride{}
                );
            }

            int runGameApp(core::game::GameState initial)
            {
                try
                {
                    platform::console::ConsoleSession consoleSession{};

                    platform::clock::SteadyClockAdapter              clock{};
                    platform::win32console::Win32ConsoleInputAdapter  input{};
                    platform::win32console::Win32ConsoleRenderAdapter render{};

                    core::presentation::CellBufferPresenter presenter{};

                    app::runtime::GameApp gameApp{ clock, input, render, presenter };

                    return gameApp.runGameLoop(std::move(initial));
                }
                catch (const std::exception& e)
                {
                    std::cerr << "Error from AppComposition.cpp.\n";
                    std::cerr << e.what();
                    return 1;
                }
                catch (...)
                {
                    return 2;
                }
            }

        } // namespace

        int run()
        {
            core::game::GameState initial{};

            initCoreDefaults(initial);

            if (!tryLoadGameAssets(initial))
            {
                return 1;
            }

            initGoalActivationRegions(initial);

            initGoals(initial);

            initPlayerPosition(initial);

            initEnemies(initial);

            initGoalWaveEffect(initial);

            return runGameApp(std::move(initial));
        }
    } // namespace composition
} // namespace app
