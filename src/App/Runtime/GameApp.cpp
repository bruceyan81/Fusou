// Copyright (c) 2026 Bruce Yan. All Rights Reserved.

/**
 * @brief GameLoop と platform adapter を接続してゲームを実行する
 */

#include "src/App/Runtime/GameApp.h"

#include "src/Core/Loop/FrameClock.h"
#include "src/Core/Loop/GameLoop.h"

namespace app
{
    namespace runtime
    {
        GameApp::GameApp(
            core::ports::ClockPort& clockPort, 
            core::ports::InputPort& inputPort,
            core::ports::RenderPort& renderPort, 
            core::ports::PresenterPort& presenterPort
        ) : clockPort_(clockPort), inputPort_(inputPort), renderPort_(renderPort), presenterPort_(presenterPort)
        {
        }

        int GameApp::runGameLoop(core::game::GameState initial)
        {
            core::loop::FrameClockConfig frameClockConfig{};

            core::loop::FrameClock frameClock{clockPort_, frameClockConfig};

            // シーンの復旧ため
            const core::game::GameState initialSnapshot = initial;

            core::loop::GameLoop loop{std::move(initial), initialSnapshot};

            return loop.runGameLoop(frameClock, inputPort_, renderPort_, presenterPort_);
        }
    } // namespace runtime
} // namespace app