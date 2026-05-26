// Copyright (c) 2026 Bruce Yan. All Rights Reserved.

/**
 * @brief goal wave effect の更新 API を公開する
 */

#pragma once

#include "src/Core/Game/GameModel.h"
#include "src/Core/Types/Types.h"

namespace core
{
    namespace game
    {
        /**
         * @brief goal wave の描画上書き、敵消去、final cutscene 遷移を更新する
         */
        void runGoalWaveEffect(GameState& gameState, types::Duration fixedDeltaTime) noexcept;
    } // namespace game
} // namespace core
