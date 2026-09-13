// Copyright (c) 2026 Bruce Yan. All Rights Reserved.

/**
 * @brief Game loop の frame 処理と Game scene 固定 Tick を実行する
 */

#include "src/Core/Game/GameScene.h"
#include "src/Core/Game/GameTick.h"
#include "src/Core/Loop/GameLoop.h"

namespace core
{
    namespace loop
    {
        GameLoop::GameLoop(core::game::GameState initial, core::game::GameState initialSnapshot)
            : gameState_(std::move(initial)), initialSnapshot_(std::move(initialSnapshot))
        {
        }

        int GameLoop::runGameLoop(FrameClock& frameClock, ports::InputPort& inputPort, ports::RenderPort& renderPort,
            ports::PresenterPort& presenterPort)
        {
            renderPort.setCursorVisible(false);

            while (true)
            {
                // 前フレームからの実時間で固定 Tick の時間ではない
                const auto currFrameDeltaTime = frameClock.beginFrame();

                // 1 フレームに何回の Tick を行っても、1 回目の入力結果を使う
                inputContext_ = game::getInputContextByScene(gameState_.scene_.current_);

                // 1. Input
                if (processInput(inputPort))
                {
                    // ESC キーを押すと
                    break;
                }

                // 2. Update
                update(frameClock, currFrameDeltaTime);

                // 3. Render
                render(renderPort, presenterPort);

                frameClock.endFrame();
            }

            return 0;
        }

        bool GameLoop::processInput(ports::InputPort& inputPort)
        {
            inputResult_ = inputPort.sample(inputContext_);

            // GoalWave 演出中は入力を遮断
            if (gameState_.scene_.current_ == game::GameScene::Game && gameState_.goal_.wave_.bActive_)
            {
                inputResult_ = ports::InputResult{};
                return false;
            }

            return inputResult_.bQuitRequested_;
        }

        void GameLoop::update(FrameClock& frameClock, core::types::Duration currFrameDeltaTime)
        {
            const auto prevScene = gameState_.scene_.current_;

            // Died から Game へ戻った時に初期状態復元を避ける
            bool bRestoredFromDied = false;

            // TODO EffectRuntimeState を外してより直接的な表現時間へ移す
            gameState_.effect_.bgTime_ += currFrameDeltaTime;

            game::GameState preUpdateSnapshot{};
            bool            bHasPreUpdateSnapshot = false;

            runtimeCoordinator_.prepareSceneUpdate(gameState_.scene_.current_);

            const bool bShouldUpdateGameScene = runtimeCoordinator_.updateScene(gameState_, frameClock, inputResult_,
                currFrameDeltaTime, fixedStepCount_, diedRestorePressCount_, bRestoredFromDied);

            // Game scene だけ固定 Tick のゲームシミュレーションを行う
            if (bShouldUpdateGameScene)
            {
                runGameSceneFixedTicks(frameClock, preUpdateSnapshot, bHasPreUpdateSnapshot);
            }

            if (gameState_.scene_.current_ != game::GameScene::Died)
            {
                diedRestorePressCount_ = 0;
            }

            handleGameShortcutInput();

            runtimeCoordinator_.finalizeSceneUpdate(gameState_, initialSnapshot_, prevScene, bRestoredFromDied,
                preUpdateSnapshot, bHasPreUpdateSnapshot, diedRestorePressCount_);
        }

        void GameLoop::handleGameShortcutInput()
        {
            if (gameState_.scene_.current_ != game::GameScene::Game)
            {
                gameICount_ = 0;
                gamePCount_ = 0;
                return;
            }

            // Game から Intro へ戻す
            if (inputResult_.bIPressed_)
            {
                gameICount_ += 1;
            }

            if (gameICount_ >= 3)
            {
                gameICount_ = 0;
                restoreInitialState();
            }

#if FUSOU_DEV
            // Dev 用 Goal checkpoint 復元
            if (inputResult_.bPPressed_)
            {
                gamePCount_ += 1;
            }

            if (gamePCount_ >= 3)
            {
                gamePCount_ = 0;

                if (runtimeCoordinator_.hasGoalRestoreSnapshot())
                {
                    restoreFromGoalCheckpoint();
                }
            }
#else
            gamePCount_ = 0;
#endif
        }

        void GameLoop::restoreInitialState()
        {
            runtimeCoordinator_.restoreFromInitialSnapshot(gameState_, initialSnapshot_);
        }

        void GameLoop::restoreFromGoalCheckpoint()
        {
            if (!runtimeCoordinator_.restoreFromGoalCheckpoint(gameState_))
            {
                return;
            }

            gameICount_ = 0;
            diedRestorePressCount_ = 0;
        }

        void GameLoop::runGameSceneFixedTicks(
            FrameClock& frameClock, core::game::GameState& preUpdateSnapshot, bool& bHasPreUpdateSnapshot)
        {
            runtimeCoordinator_.beginGameSceneUpdate(gameState_);

            preUpdateSnapshot = gameState_;
            bHasPreUpdateSnapshot = true;

            const auto fixedDeltaTime = frameClock.getFixedStep();

            fixedStepCount_ = frameClock.consumeFixedSteps();

            for (int i = 0; i < fixedStepCount_; ++i)
            {
                game::tickGameStateByFixedStep(gameState_, fixedDeltaTime, inputResult_);
                game::updateSceneByInput(gameState_, inputResult_);

                if (gameState_.scene_.current_ != game::GameScene::Game)
                {
                    break;
                }
            }

            runtimeCoordinator_.endGameSceneUpdate(gameState_);
        }

        void GameLoop::render(ports::RenderPort& renderPort, ports::PresenterPort& presenterPort)
        {
            core::render::CellBuffer frame{};

            presenterPort.composeFrame(gameState_, frame);

            renderPort.present(frame);

            renderPort.flush();
        }
    } // namespace loop
} // namespace core
