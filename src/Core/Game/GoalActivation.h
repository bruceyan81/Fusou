// Copyright (c) 2026 Bruce Yan. All Rights Reserved.

/**
 * @brief goal への接近、点灯、色変更の更新 API を公開する
 */

#pragma once

#include "src/Core/Game/GameModel.h"

namespace core
{
    namespace ports
    {
        struct InputResult;
    }

    namespace game
    {
        /**
         * @brief 現在の goal 領域、接近状態、点灯状態を更新する
         */
        void runGoalActivation(GameState& gameState, const ports::InputResult& inputResult) noexcept;
    } // namespace game
} // namespace core
