// Copyright (c) 2026 Bruce Yan. All Rights Reserved.

/**
 * @brief goal への接近、点灯、点灯後の色変更を処理する
 */

#include "src/Core/Game/GoalActivation.h"

#include "src/Core/Ports/InputPort.h"
#include "src/Core/World/TileMap.h"

#include <algorithm>
#include <cmath>

namespace core
{
    namespace game
    {
        namespace
        {
            constexpr int kNearGoalHalfRangeX = 2;
            constexpr int kNearGoalHalfRangeY = 1;
            constexpr float kFallbackGoalWaveDurationSeconds = 1.5f;

            [[nodiscard]] int findGoalRegionIndexByPlayerFeetY(
                const std::vector<GoalActivationRegion>& goalRegions, int playerFeetY) noexcept
            {
                for (std::size_t i = 0; i < goalRegions.size(); ++i)
                {
                    const auto& goalRegion = goalRegions[i];

                    if (goalRegion.beginY_ <= playerFeetY && playerFeetY <= goalRegion.endY_)
                    {
                        return static_cast<int>(i);
                    }
                }

                return -1;
            }

            [[nodiscard]] bool isNearGoalByPlayerFeet(
                const GoalActivationRegion& goalRegion, const types::Vec2& playerFeetCell) noexcept
            {
                const int dx = std::abs(playerFeetCell.x_ - goalRegion.goalWorldCell_.x_);
                const int dy = std::abs(playerFeetCell.y_ - goalRegion.goalWorldCell_.y_);

                return dx <= kNearGoalHalfRangeX && dy <= kNearGoalHalfRangeY;
            }

            [[nodiscard]] float calculateGoalWaveSpeedCellsPerSec(
                const GameState& gameState, const GoalActivationRegion& goalRegion, const int thickness) noexcept
            {
                const int mapWidth = gameState.assets_.gameSceneBackground_.tileMap_.getWidth();
                const int mapHeight = gameState.assets_.gameSceneBackground_.tileMap_.getHeight();
                if (mapWidth <= 0 || mapHeight <= 0)
                {
                    return 0.0f;
                }

                const int cx = goalRegion.goalWorldCell_.x_;
                const int cy = goalRegion.goalWorldCell_.y_;
                const int beginY = std::clamp(goalRegion.beginY_, 0, mapHeight - 1);
                const int endY = std::clamp(goalRegion.endY_, 0, mapHeight - 1);

                const int maxDx = std::max(cx, (mapWidth - 1) - cx);
                const int maxDy = std::max(cy - beginY, endY - cy);
                const int maxManhattan = maxDx + maxDy;

                const float durationSeconds = (gameState.goal_.wave_.durationSeconds_ > 0.0f)
                    ? gameState.goal_.wave_.durationSeconds_
                    : kFallbackGoalWaveDurationSeconds;

                return static_cast<float>(maxManhattan + thickness + 1) / durationSeconds;
            }

            void paintActivatedGoalRegion(GameState& gameState) noexcept
            {
                const auto& currGoalRegion = gameState.goal_.goalRegions_[gameState.goal_.currGoalRegionIndex_];

                constexpr render::Color kTargetSunAndGoalFg = render::Color::Yellow;
                constexpr render::Color kTargetSunAndGoalBg = render::Color::LightRed;

                constexpr render::Color kTargetLeafFg = render::Color::LightGreen;
                constexpr render::Color kTargetLeafBg = render::Color::Yellow;

                for (int y = currGoalRegion.beginY_; y <= currGoalRegion.endY_; ++y)
                {
                    for (int x = 0; x < gameState.assets_.gameSceneBackground_.tileMap_.getWidth(); ++x)
                    {
                        const world::TileId gameMapTile =
                            gameState.assets_.gameScene_.tileMap_.getTileIdByCell({x, y});
                        const world::TileId backgroundTile =
                            gameState.assets_.gameSceneBackground_.tileMap_.getTileIdByCell({x, y});

                        auto& paintOverride =
                            gameState.goal_.paintOverrides_[y * gameState.assets_.gameSceneBackground_.tileMap_.getWidth()
                                + x];

                        if (gameMapTile == world::tileid::kGoal || backgroundTile == world::tileid::kSun)
                        {
                            paintOverride.bEnabled_ = true;
                            paintOverride.bHasFg_ = true;
                            paintOverride.bHasBg_ = true;

                            paintOverride.fg_ = kTargetSunAndGoalFg;
                            paintOverride.bg_ = kTargetSunAndGoalBg;

                            continue;
                        }

                        if (backgroundTile == world::tileid::kRayTop || backgroundTile == world::tileid::kRayDown)
                        {
                            paintOverride.bEnabled_ = true;
                            paintOverride.bHasFg_ = true;
                            paintOverride.bHasBg_ = true;

                            paintOverride.fg_ = kTargetLeafFg;
                            paintOverride.bg_ = kTargetLeafBg;
                        }
                    }
                }
            }

            void tickGoalActivation(GameState& gameState, const ports::InputResult& inputResult) noexcept
            {
                const auto playerFeetCell = types::vec2fFloorToVec2(gameState.player_.feetWorldPos_);
                const int  currGoalRegionIndex =
                    findGoalRegionIndexByPlayerFeetY(gameState.goal_.goalRegions_, playerFeetCell.y_);

                gameState.goal_.currGoalRegionIndex_ = currGoalRegionIndex;
                gameState.goal_.bIsNear_ = false;

                if (currGoalRegionIndex < 0)
                {
                    return;
                }

                const auto& goalRegion = gameState.goal_.goalRegions_[currGoalRegionIndex];
                gameState.goal_.bIsNear_ = isNearGoalByPlayerFeet(goalRegion, playerFeetCell);

                if (!gameState.goal_.bIsNear_)
                {
                    return;
                }

                if (!inputResult.bInteracted_)
                {
                    return;
                }

                if (gameState.goal_.goalLitFlags_[currGoalRegionIndex])
                {
                    return;
                }

                const bool bIsFinalGoal =
                    (currGoalRegionIndex == static_cast<int>(gameState.goal_.goalLitFlags_.size()) - 1);

                if (bIsFinalGoal)
                {
                    bool bAllPreviousGoalsLit = true;

                    for (int i = 0; i < currGoalRegionIndex; ++i)
                    {
                        if (!gameState.goal_.goalLitFlags_[i])
                        {
                            bAllPreviousGoalsLit = false;
                            break;
                        }
                    }

                    if (!bAllPreviousGoalsLit)
                    {
                        return;
                    }

                    gameState.goal_.bIsClear_ = true;
                    gameState.goal_.bPendingFinalCutscene_ = true;
                }

                // goal 点灯を確定し、復活位置として使う最後の goal index を保存する
                // 永続色と wave 演出の runtime state もここで初期化する
                gameState.goal_.goalLitFlags_[currGoalRegionIndex] = true;
                gameState.goal_.lastLitIndex_ = currGoalRegionIndex;

                paintActivatedGoalRegion(gameState);

                gameState.goal_.wavePaintOverrides_.clear();
                gameState.goal_.wavePaintOverrides_.resize(gameState.goal_.paintOverrides_.size());

                gameState.goal_.wave_.bActive_ = true;
                gameState.goal_.wave_.centerWorldCell_ = goalRegion.goalWorldCell_;
                gameState.goal_.wave_.beginY_ = goalRegion.beginY_;
                gameState.goal_.wave_.endY_ = goalRegion.endY_;
                gameState.goal_.wave_.elapsedSeconds_ = 0.0f;
                gameState.goal_.wave_.ringThicknessCells_ = 2;
                gameState.goal_.wave_.speedCellsPerSec_ = calculateGoalWaveSpeedCellsPerSec(
                    gameState, goalRegion, gameState.goal_.wave_.ringThicknessCells_);
            }
        } // namespace

        void runGoalActivation(GameState& gameState, const ports::InputResult& inputResult) noexcept
        {
            tickGoalActivation(gameState, inputResult);
        }

    } // namespace game
} // namespace core
