// Copyright (c) 2026 Bruce Yan. All Rights Reserved.
/**
 * @brief 敵の視認判定と巡回と追跡状態を更新する
 */

#include "src/Core/Game/EnemyAi.h"
#include "src/Core/Game/TileRules.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>

namespace core
{
    namespace game
    {
        namespace
        {
            constexpr int kEnemyPatrolTurnCooldownMinTicks = 5;
            constexpr int kEnemyPatrolTurnCooldownVariationTicks = 4;

            [[nodiscard]] int signInt(const int v) noexcept
            {
                return (v > 0) ? 1 : (v < 0) ? -1 : 0;
            }

            [[nodiscard]] int calcEnemyPatrolTurnCooldownTicks(
                std::size_t tickIndex, const types::Vec2& enemyCurrCell) noexcept
            {
                const std::size_t seed = tickIndex + static_cast<std::size_t>(enemyCurrCell.x_)
                    + static_cast<std::size_t>(enemyCurrCell.y_);

                return kEnemyPatrolTurnCooldownMinTicks
                    + static_cast<int>(seed % kEnemyPatrolTurnCooldownVariationTicks);
            }

            /**
             * @brief 次の移動先が立てられるかどうか判断する
             */
            [[nodiscard]] bool canEnemyStepTo(const world::TileMap& gameMap, const world::Camera& camera,
                const LightState& lightState, const types::Vec2& enemyCurrCell, int dirX) noexcept
            {
                if (dirX == 0)
                {
                    return false;
                }

                const types::Vec2 enemyNextCell{ enemyCurrCell.x_ + dirX, enemyCurrCell.y_ };
                const types::Vec2 enemyNextSupportCell{ enemyCurrCell.x_ + dirX, enemyCurrCell.y_ + 1 };

                if (!gameMap.isCellInBounds(enemyNextCell) || !gameMap.isCellInBounds(enemyNextSupportCell))
                {
                    return false;
                }

                // 移動先は Blocking
                if (isBlockingForEnemyOccupy(gameMap, enemyNextCell, camera, lightState))
                {
                    return false;
                }

                // 移動先は立てる可能かどうか判断
                if (!isSupportingForEnemy(gameMap, enemyNextSupportCell, camera, lightState))
                {
                    return false;
                }

                return true;
            }
        } // namespace

        int tickEnemyAi(
            Enemy& enemy,
            const types::Vec2f& playerFeetWorldPos,
            const int chaseRange,
            const int alertCooldownDurationTicks,
            const std::size_t tickIndex,
            const world::TileMap& gameMap,
            const world::Camera& camera,
            const LightState& lightState
        ) noexcept
        {
            const types::Vec2 enemyCurrCell = types::vec2fFloorToVec2(enemy.worldPos_);
            const types::Vec2 playerCurrCell = types::vec2fFloorToVec2(playerFeetWorldPos);

            const int dx = playerCurrCell.x_ - enemyCurrCell.x_;

            // Alert 中は状態遷移を止め、表示用の警戒状態を一定 Tick 維持する
            if (enemy.alertRemainingTicks_ > 0)
            {
                --enemy.alertRemainingTicks_;

                if (enemy.alertRemainingTicks_ <= 0)
                {
                    enemy.alertRemainingTicks_ = 0;
                    enemy.alert_ = EnemyAlert::None;
                }

                // Cooldown の状態の場合、そのまま止まっている
                if (enemy.state_ == EnemyState::Patrol)
                {
                    return 0;
                }

                const int dirX = signInt(dx);
                if (enemy.bGrounded_ && dirX != 0)
                {
                    if (!canEnemyStepTo(gameMap, camera, lightState, enemyCurrCell, dirX))
                    {
                        return 0;
                    }
                }
                return dirX;
            }

            enemy.alert_ = EnemyAlert::None;

            // 追跡の条件は同じ Y かつ 追跡範囲内
            const bool bShouldChase = (
                (enemyCurrCell.y_ == playerCurrCell.y_)
                &&
                (std::abs(dx) <= chaseRange)
            );

            const EnemyState newState = bShouldChase ? EnemyState::Chase : EnemyState::Patrol;

            // 状態が切り替わった Tick は alert を出し、Patrol へ戻る場合は一時停止する
            if (newState != enemy.state_)
            {
                enemy.prevState_ = enemy.state_;
                enemy.state_ = newState;
                enemy.alert_ = (newState == EnemyState::Chase) ? EnemyAlert::Spotted : EnemyAlert::Lost;

                enemy.alertRemainingTicks_ = (alertCooldownDurationTicks > 0) ? alertCooldownDurationTicks : 1;

                if (enemy.state_ == EnemyState::Patrol)
                {
                    return 0;
                }

                const int dirX = signInt(dx);
                if (enemy.bGrounded_ && dirX != 0)
                {
                    if (!canEnemyStepTo(gameMap, camera, lightState, enemyCurrCell, dirX))
                    {
                        return 0;
                    }
                }
                return dirX;
            }

            // 状態が変わらない場合
            if (enemy.state_ == EnemyState::Chase)
            {
                const int dirX = signInt(dx);
                if (enemy.bGrounded_ && dirX != 0)
                {
                    if (!canEnemyStepTo(gameMap, camera, lightState, enemyCurrCell, dirX))
                    {
                        return 0;
                    }
                }
                return dirX;
            }

            // 方向が変更した後は一時止まる
            if (enemy.patrolTurnCooldownTicks_ > 0)
            {
                --enemy.patrolTurnCooldownTicks_;
                if (enemy.patrolTurnCooldownTicks_ < 0)
                {
                    enemy.patrolTurnCooldownTicks_ = 0;
                }
                return 0;
            }

            // パトロール範囲の下限と上限
            const int left = enemy.patrolAnchorX_ - kEnemyPatrolHalfRange;
            const int right = enemy.patrolAnchorX_ + kEnemyPatrolHalfRange;

            bool bTurnRequested = false;

            const int prevDir = enemy.patrolDirX_;

            if (enemyCurrCell.x_ <= left)
            {
                enemy.patrolDirX_ = 1;
            }
            else if (enemyCurrCell.x_ >= right)
            {
                enemy.patrolDirX_ = -1;
            }

            enemy.patrolDirX_ = std::clamp(enemy.patrolDirX_, -1, 1);

            if (enemy.patrolDirX_ != prevDir)
            {
                bTurnRequested = true;
            }

            if (enemy.bHitWallX_)
            {
                if (enemy.patrolDirX_ == 0)
                {
                    enemy.patrolDirX_ = 1;
                }
                enemy.patrolDirX_ = -enemy.patrolDirX_;
                bTurnRequested = true;
            }

            if (enemy.bGrounded_ && enemy.patrolDirX_ != 0)
            {
                const bool bCanStep =
                    canEnemyStepTo(gameMap, camera, lightState, enemyCurrCell, enemy.patrolDirX_);

                if (!bCanStep)
                {
                    bTurnRequested = true;
                    const int  reversed = -enemy.patrolDirX_;
                    const bool bCanStepReversed =
                        canEnemyStepTo(gameMap, camera, lightState, enemyCurrCell, reversed);

                    if (bCanStepReversed)
                    {
                        enemy.patrolDirX_ = reversed;
                    }
                    else
                    {
                        // 移動できない場合、止まる
                        return 0;
                    }
                }
            }

            if (bTurnRequested)
            {
                enemy.patrolTurnCooldownTicks_ = calcEnemyPatrolTurnCooldownTicks(tickIndex, enemyCurrCell);
                return 0;
            }

            return enemy.patrolDirX_;
        }
    } // namespace game
} // namespace core
