// Copyright (c) 2026 Bruce Yan. All Rights Reserved.

/**
 * @brief 敵の物理移動更新 API を公開する
 */

#pragma once

#include "src/Core/Game/Enemy.h"
#include "src/Core/Game/LightState.h"
#include "src/Core/World/Camera.h"
#include "src/Core/World/TileMap.h"

namespace core
{
    namespace game
    {
        void tickEnemySim(Enemy& enemy, int moveIntentX, float dt, float enemyMoveSpeed,
            const world::TileMap& gameMap, const world::Camera& camera, const LightState& lightState) noexcept;
    } // namespace game
} // namespace core
