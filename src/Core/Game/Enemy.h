// Copyright (c) 2026 Bruce Yan. All Rights Reserved.

/**
 * @brief 敵の状態と runtime data を定義する
 */

#pragma once

#include "src/Core/Types/Types.h"

namespace core
{
    namespace game
    {
        enum class EnemyState
        {
            Patrol,
            Chase,
        };

        enum class EnemyAlert
        {
            None,
            Spotted,
            Lost,
        };

        struct Enemy final
        {
            types::Vec2f worldPos_{};

            types::Vec2f velocity_{};

            EnemyState state_{EnemyState::Patrol};

            EnemyState prevState_{EnemyState::Patrol};

            EnemyAlert alert_{EnemyAlert::None};

            int alertRemainingTicks_{0};

            bool bGrounded_{false};

            // 敵の巡回範囲はスポーン地点の X 座標を基準にする
            int patrolAnchorX_{0};

            int patrolDirX_{1};

            // 移動方向が変更した後、少し止まってから移動する
            int patrolTurnCooldownTicks_{0};

            bool bHitWallX_{false};
        };
    } // namespace game
} // namespace core
