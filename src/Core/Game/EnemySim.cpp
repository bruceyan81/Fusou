// Copyright (c) 2026 Bruce Yan. All Rights Reserved.

/**
 * @brief 敵の速度と world position を固定 Tick で更新する
 */

#include "src/Core/Game/EnemySim.h"

#include "src/Core/Game/TileRules.h"

#include <algorithm> // std::clamp

namespace core
{
    namespace game
    {
        namespace
        {
            [[nodiscard]] bool canEnemyMoveTo(const world::TileMap& map, const types::Vec2f& candidateWorldPos,
                const world::Camera& camera, const LightState& lightState,
                const bool bTreatClimbableLadderAsBlocking) noexcept
            {
                const types::Vec2 cell = types::vec2fFloorToVec2(candidateWorldPos);

                // Enemy のサイズは 1 * 1
                if (cell.x_ < 0 || cell.x_ >= map.getWidth() || cell.y_ < 0 || cell.y_ >= map.getHeight())
                {
                    return false;
                }

                return !isBlockingForEnemy(map, cell, camera, lightState, bTreatClimbableLadderAsBlocking);
            }

            void integrateEnemyVelocity(Enemy& enemy, int moveIntentX, float dt, float enemyMoveSpeed) noexcept
            {
                if (moveIntentX == 0)
                {
                    enemy.velocity_.x_ = 0.0f;
                }
                else
                {
                    enemy.velocity_.x_ = enemyMoveSpeed * static_cast<float>(moveIntentX);
                    enemy.velocity_.x_ = std::clamp(enemy.velocity_.x_, -kMaxHorizontalSpeed, kMaxHorizontalSpeed);
                }

                const float nextVy = enemy.velocity_.y_ + (kGravity * dt);
                enemy.velocity_.y_ = std::clamp(nextVy, -kMaxFallSpeed, kMaxFallSpeed);
            }

            void resolveMoveEnemyAxisX(Enemy& enemy, float dt, const world::TileMap& gameMap,
                const world::Camera& camera, const LightState& lightState) noexcept
            {
                const float dx = enemy.velocity_.x_ * dt;
                if (dx == 0.0f)
                {
                    return;
                }

                const types::Vec2f currPos = enemy.worldPos_;
                const types::Vec2f candidatePos{currPos.x_ + dx, currPos.y_};

                if (canEnemyMoveTo(gameMap, candidatePos, camera, lightState, false))
                {
                    enemy.worldPos_ = candidatePos;
                    return;
                }

                // X 軸衝突は位置を維持し速度 X を 0 にする
                enemy.bHitWallX_ = true;
                enemy.velocity_.x_ = 0.0f;
            }

            [[nodiscard]] bool applyEnemyGroundingBySupportIfNeeded(Enemy& enemy, const world::TileMap& gameMap,
                const world::Camera& camera, const LightState& lightState) noexcept
            {
                // Enemy のサイズは 1 * 1
                const types::Vec2 enemyCell = types::vec2fFloorToVec2(enemy.worldPos_);

                const types::Vec2 supportCell{enemyCell.x_, enemyCell.y_ + 1};

                if (!gameMap.isCellInBounds(supportCell))
                {
                    return false;
                }

                if (!isSupportingForEnemy(gameMap, supportCell, camera, lightState))
                {
                    return false;
                }

                enemy.worldPos_.y_ = static_cast<float>(enemyCell.y_);
                enemy.velocity_.y_ = 0.0f;
                enemy.bGrounded_ = true;

                return true;
            }

            void resolveMoveEnemyAxisY(Enemy& enemy, float dt, const world::TileMap& gameMap,
                const world::Camera& camera, const LightState& lightState) noexcept
            {
                const float dy = enemy.velocity_.y_ * dt;
                if (dy == 0.0f)
                {
                    return;
                }

                const types::Vec2f currPos = enemy.worldPos_;
                const types::Vec2f candidatePos{currPos.x_, currPos.y_ + dy};

                const bool bTreatClimbableLadderAsBlocking = (dy > 0.0f);

                if (canEnemyMoveTo(gameMap, candidatePos, camera, lightState, bTreatClimbableLadderAsBlocking))
                {
                    enemy.worldPos_ = candidatePos;

                    // 落下中は床で着陸できる
                    if (dy > 0.0f)
                    {
                        (void)applyEnemyGroundingBySupportIfNeeded(enemy, gameMap, camera, lightState);
                    }

                    return;
                }

                // 下方向の Cell と衝突の場合、着陸する
                if (dy > 0.0f)
                {
                    enemy.bGrounded_ = true;
                }

                enemy.velocity_.y_ = 0.0f;
            }

        } // namespace

        void tickEnemySim(
            Enemy& enemy,
            const int moveIntentX,
            const float dt,
            const float enemyMoveSpeed,
            const world::TileMap& gameMap,
            const world::Camera& camera,
            const LightState& lightState
        ) noexcept
        {
            const bool bWasGrounded = enemy.bGrounded_;

            enemy.bHitWallX_ = false;

            // 地面に立ってるのか Tick ごとにチェックする
            enemy.bGrounded_ = false;

            // 敵が空中の場合、x が変えない
            const int resolvedMoveIntentX = bWasGrounded ? moveIntentX : 0;
            integrateEnemyVelocity(enemy, resolvedMoveIntentX, dt, enemyMoveSpeed);

            // X と Y を分けて解決し、横衝突と着地を別々に確定する
            resolveMoveEnemyAxisX(enemy, dt, gameMap, camera, lightState);
            resolveMoveEnemyAxisY(enemy, dt, gameMap, camera, lightState);
        }
    } // namespace game
} // namespace core
