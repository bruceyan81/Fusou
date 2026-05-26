// Copyright (c) 2026 Bruce Yan. All Rights Reserved.

/**
 * @brief プレイヤーと light に追従する camera 更新 API を公開する
 */

#pragma once

#include "src/Core/Game/LightState.h"
#include "src/Core/Game/PlayerState.h"
#include "src/Core/Types/Types.h"
#include "src/Core/World/Camera.h"

namespace core
{
    namespace game
    {
        /**
         * @brief Camera の追従とライトの後処理
         */
        void runCameraFollow(world::Camera& camera, const PlayerState& playerState, LightState& lightState,
            const types::Vec2& worldSize) noexcept;
    } // namespace game
} // namespace core
