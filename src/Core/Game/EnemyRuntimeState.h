// Copyright (c) 2026 Bruce Yan. All Rights Reserved.

/**
 * @brief 敵全体の runtime state を定義する
 */

#pragma once

#include "src/Core/Game/Enemy.h"

#include <vector>

namespace core
{
    namespace game
    {
        struct EnemyRuntimeState final
        {
            std::vector<Enemy> enemies_{};

            // 敵の alert 表示を維持する固定 Tick 数
            int alertCooldownDurationTicks_{0};

            int chaseRange_{0};

            float moveSpeed_{0.0f};
        };
    } // namespace game
} // namespace core
