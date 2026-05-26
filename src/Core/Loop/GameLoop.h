// Copyright (c) 2026 Bruce Yan. All Rights Reserved.

/**
 * @brief 入力更新描画を統括する main loop class を定義する
 */

#pragma once

#include "src/Core/Game/GameModel.h"
#include "src/Core/Loop/FrameClock.h"
#include "src/Core/Loop/RuntimeCoordinator.h"
#include "src/Core/Ports/InputPort.h"
#include "src/Core/Ports/PresenterPort.h"
#include "src/Core/Ports/RenderPort.h"

namespace core
{
    namespace loop
    {
        class GameLoop final
        {
        public:
            GameLoop(core::game::GameState initial, core::game::GameState initialSnapshot);

            int runGameLoop(FrameClock& frameClock, ports::InputPort& inputPort, ports::RenderPort& renderPort,
                ports::PresenterPort& presenterPort);

        private:
            // 1 フレーム内の Tick 回数
            int fixedStepCount_{};

            game::GameState gameState_{};

            // 全体復元用
            game::GameState initialSnapshot_{};

            RuntimeCoordinator runtimeCoordinator_{};

            // Dev 用 Died scene で R を押した回数
            int diedRestorePressCount_ = 0;
            int gameICount_ = 0;

            // Dev 用 Goal checkpoint 復元で P を押した回数
            int gamePCount_ = 0;

            // 入力サンプル結果
            ports::InputResult inputResult_{};

            // TODO pendingStepDir_ の命名も一 cell 移動入力に合わせる
            // 瞬間的な一 cell 移動入力を次の Tick まで保持する
            ports::StepDirection pendingStepDir_{ports::StepDirection::None};

            // シーンより、入力サンプリングの文脈
            ports::InputContext inputContext_{};

            // 1 フレームにつき  1回サンプリングする
            [[nodiscard]] bool processInput(ports::InputPort& inputPort);

            void update(FrameClock& frameClock, core::types::Duration currFrameDeltaTime);

            void handleGameShortcutInput();

            void restoreInitialState();

            void restoreFromGoalCheckpoint();

            void runGameSceneFixedTicks(
                FrameClock& frameClock, core::game::GameState& preUpdateSnapshot, bool& bHasPreUpdateSnapshot);

            void render(ports::RenderPort& renderPort, ports::PresenterPort& presenterPort);
        };
    } // namespace loop
} // namespace core
