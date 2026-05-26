// Copyright (c) 2026 Bruce Yan. All Rights Reserved.

/**
 * @brief light の入力更新 API を公開する
 */

#pragma once

#include "src/Core/Game/LightState.h"
#include "src/Core/Ports/InputPort.h"
#include "src/Core/Types/Types.h"

namespace core
{
    namespace game
    {
        /**
         * @brief ライトの ON OFF を切り替える処理
         */
        void runLightToggle(
            LightState& lightState, const ports::InputResult& inputResult, const types::Vec2& cameraOriginWorld) noexcept;

        /**
         * @brief ライトの移動を更新する処理
         */
        void runLightMove(
            LightState& lightState, const ports::InputResult& inputResult, const types::Vec2& cameraOriginWorld) noexcept;
    } // namespace game
} // namespace core
