// Copyright (c) 2026 Bruce Yan. All Rights Reserved.

/**
 * @brief 組み立て済み runtime でゲームを実行する class を定義する
 */

#pragma once

#include "src/Core/Ports/ClockPort.h"
#include "src/Core/Ports/InputPort.h"
#include "src/Core/Ports/PresenterPort.h"
#include "src/Core/Ports/RenderPort.h"

namespace app
{
    namespace runtime
    {
        class GameApp final
        {
        public:
            GameApp(
                core::ports::ClockPort& clockPort,
                core::ports::InputPort& inputPort,
                core::ports::RenderPort& renderPort,
                core::ports::PresenterPort& presenterPort
            );

            int runGameLoop(core::game::GameState initial);

        private:
            core::ports::ClockPort& clockPort_;
            core::ports::InputPort& inputPort_;
            core::ports::RenderPort& renderPort_;
            core::ports::PresenterPort& presenterPort_;
        };
    } // namespace runtime
} // namespace app
