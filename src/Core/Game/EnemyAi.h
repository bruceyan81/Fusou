// Copyright (c) 2026 Bruce Yan. All Rights Reserved.

/**
 * @brief 敵 AI の固定 Tick 更新 API を公開する
 */

#pragma once

#include "src/Core/Game/Enemy.h"
#include "src/Core/Game/LightState.h"
#include "src/Core/Types/Types.h"
#include "src/Core/World/Camera.h"
#include "src/Core/World/TileMap.h"

#include <cstddef>

namespace core
{
    namespace game
    {

        /**
         * @brief 敵の巡回追跡状態を更新し、この Tick の水平移動意図を返す
         */
        [[nodiscard]] int tickEnemyAi(Enemy& enemy, const types::Vec2f& playerFeetWorldPos, int chaseRange,
            int alertCooldownDurationTicks, std::size_t tickIndex, const world::TileMap& gameMap,
            const world::Camera& camera, const LightState& lightState) noexcept;

    } // namespace game
} // namespace core
