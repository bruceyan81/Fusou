// Copyright (c) 2026 Bruce Yan. All Rights Reserved.

/**
 * @brief Game scene の固定 Tick 更新 API を公開する
 */

#pragma once

#include "src/Core/Game/GameModel.h"
#include "src/Core/Ports/InputPort.h"
#include "src/Core/Types/Types.h"

namespace core
{
    namespace game
    {
        /**
         * @brief Game scene の固定 Tick シミュレーションを 1 回進める
         * @param fixedDeltaTime 固定 Tick の時間幅
         * @param inputResult この Tick に反映する入力
         */
        void tickGameStateByFixedStep(
            GameState& gameState, types::Duration fixedDeltaTime, const ports::InputResult& inputResult);
    }
}
