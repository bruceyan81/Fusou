// Copyright (c) 2026 Bruce Yan. All Rights Reserved.

/**
 * @brief goal wave の拡散と final cutscene 遷移を更新する
 */

#include "src/Core/Game/GoalWaveEffect.h"

#include "src/Core/World/Camera.h"

#include <algorithm>
#include <chrono>
#include <cmath>

namespace core
{
    namespace game
    {
        namespace
        {
            [[nodiscard]] bool tryApplyGoalWaveEffectOverride(
                GameState& gameState, const int x, const int y, const world::TileId tileId) noexcept
            {
                for (const auto& rule : gameState.goal_.waveRules_)
                {
                    if (rule.tileId_ != tileId)
                    {
                        continue;
                    }

                    const int mapWidth = gameState.assets_.gameSceneBackground_.tileMap_.getWidth();
                    auto&     paintOverride = gameState.goal_.wavePaintOverrides_[y * mapWidth + x];

                    paintOverride.bEnabled_ = true;

                    if (rule.bHasFg_)
                    {
                        paintOverride.bHasFg_ = true;
                        paintOverride.fg_ = rule.fg_;
                    }

                    if (rule.bHasBg_)
                    {
                        paintOverride.bHasBg_ = true;
                        paintOverride.bg_ = rule.bg_;
                    }

                    return true;
                }

                return false;
            }

            void initializeFinalCutsceneState(GameState& gameState) noexcept
            {
                auto& state = gameState.scene_.finalCutscene_;

                state.phase_ = FinalCutscenePhase::PanningUp;
                state.ticksUntilNextCell_ = kFinalCutsceneTicksPerCell;
                state.holdRemainingSeconds_ = kFinalCutsceneHoldSeconds;

                const int bgWidth = gameState.assets_.gameSceneBackground_.tileMap_.getWidth();
                const int bgHeight = gameState.assets_.gameSceneBackground_.tileMap_.getHeight();

                const int startXUnclamped = (bgWidth - world::Camera::kViewportWidth) / 2;
                const int startX = (startXUnclamped > 0) ? startXUnclamped : 0;

                const int startYUnclamped = bgHeight - world::Camera::kViewportHeight;
                const int startY = (startYUnclamped > 0) ? startYUnclamped : 0;

                state.startCameraOriginY_ = startY;
                state.endCameraOriginY_ = 0;
                state.cameraOriginY_ = startY;
                state.targetCameraOriginX_ = startX;

                const types::Vec2 bgWorldSize{bgWidth, bgHeight};
                gameState.mainCamera_.setViewportOriginAtWorld(types::Vec2{startX, startY}, bgWorldSize);
            }

            void tickGoalWaveEffect(GameState& gameState, const types::Duration fixedDeltaTime) noexcept
            {
                auto& wave = gameState.goal_.wave_;

                if (!wave.bActive_)
                {
                    return;
                }

                const float dtSeconds = std::chrono::duration<float, std::ratio<1>>(fixedDeltaTime).count();
                if (dtSeconds <= 0.0f)
                {
                    return;
                }

                wave.elapsedSeconds_ += dtSeconds;

                const float radiusF = wave.elapsedSeconds_ * wave.speedCellsPerSec_;
                const int   radius = (radiusF > 0.0f) ? static_cast<int>(std::floor(radiusF)) : 0;

                const int thickness = (wave.ringThicknessCells_ > 0) ? wave.ringThicknessCells_ : 1;
                const int outer = radius + thickness;

                const int mapWidth = gameState.assets_.gameSceneBackground_.tileMap_.getWidth();
                const int mapHeight = gameState.assets_.gameSceneBackground_.tileMap_.getHeight();

                const int cx = static_cast<int>(wave.centerWorldCell_.x_);
                const int cy = static_cast<int>(wave.centerWorldCell_.y_);

                const int beginY = std::clamp(wave.beginY_, 0, mapHeight - 1);
                const int endY = std::clamp(wave.endY_, 0, mapHeight - 1);

                const int maxDx = std::max(cx, (mapWidth - 1) - cx);
                const int maxDy = std::max(cy - beginY, endY - cy);

                // グリッド上で菱形に広がるためマンハッタン距離を使う
                const int maxManhattan = maxDx + maxDy;

                if (radius > (maxManhattan + thickness))
                {
                    wave.bActive_ = false;

                    std::fill(
                        gameState.goal_.wavePaintOverrides_.begin(),
                        gameState.goal_.wavePaintOverrides_.end(),
                        PaintOverride{}
                    );

                    if (gameState.goal_.bPendingFinalCutscene_)
                    {
                        initializeFinalCutsceneState(gameState);
                        gameState.scene_.current_ = GameScene::FinalCutscene;
                        gameState.goal_.bPendingFinalCutscene_ = false;
                    }

                    return;
                }

                std::fill(
                    gameState.goal_.wavePaintOverrides_.begin(),
                    gameState.goal_.wavePaintOverrides_.end(),
                    PaintOverride{}
                );

                {
                    auto& enemies = gameState.enemyRuntime_.enemies_;

                    for (auto it = enemies.begin(); it != enemies.end();)
                    {
                        const types::Vec2 enemyWorldCell = types::vec2fFloorToVec2(it->worldPos_);

                        const int enemyY = enemyWorldCell.y_;
                        if (enemyY < beginY || enemyY > endY)
                        {
                            ++it;
                            continue;
                        }

                        // グリッド上で菱形に広がるためマンハッタン距離を使う
                        const int dist = std::abs(enemyWorldCell.x_ - cx) + std::abs(enemyWorldCell.y_ - cy);
                        if (dist >= radius && dist < outer)
                        {
                            it = enemies.erase(it);
                            continue;
                        }

                        ++it;
                    }
                }

                const int minX = std::max(0, cx - outer);
                const int maxX = std::min(mapWidth - 1, cx + outer);

                for (int y = beginY; y <= endY; ++y)
                {
                    for (int x = minX; x <= maxX; ++x)
                    {
                        // グリッド上で菱形に広がるためマンハッタン距離を使う
                        const int dist = std::abs(x - cx) + std::abs(y - cy);
                        if (dist < radius || dist >= outer)
                        {
                            continue;
                        }

                        const world::TileId tileId =
                            gameState.assets_.gameSceneBackground_.tileMap_.getTileIdByCell(types::Vec2{x, y});

                        (void)tryApplyGoalWaveEffectOverride(gameState, x, y, tileId);
                    }
                }
            }
        } // namespace

        void runGoalWaveEffect(GameState& gameState, types::Duration fixedDeltaTime) noexcept
        {
            tickGoalWaveEffect(gameState, fixedDeltaTime);
        }
    } // namespace game
} // namespace core
