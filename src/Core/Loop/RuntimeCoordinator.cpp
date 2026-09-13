// Copyright (c) 2026 Bruce Yan. All Rights Reserved.

/**
 * @brief scene 更新と died goal snapshot 復元を調停する
 */

#include "src/Core/Game/GameScene.h"
#include "src/Core/Loop/FrameClock.h"
#include "src/Core/Loop/RuntimeCoordinator.h"

#include <chrono>
#include <cmath>
#include <cstddef>

namespace core
{
    namespace loop
    {
        void RuntimeCoordinator::prepareSceneUpdate(game::GameScene scene) noexcept
        {
            if (scene == game::GameScene::Game)
            {
                return;
            }

            clearGoalCheckpointTracking();
        }

        bool RuntimeCoordinator::updateScene(game::GameState& ioGameState, FrameClock& frameClock,
            const ports::InputResult& inputResult, core::types::Duration currFrameDeltaTime, int& ioFixedStepCount,
            int& diedRestorePressCount, bool& bIoRestoredFromDied) const
        {
            if (ioGameState.scene_.current_ == game::GameScene::Intro)
            {
                updateIntroScene(ioGameState, inputResult, currFrameDeltaTime);
                return false;
            }

            if (ioGameState.scene_.current_ == game::GameScene::Died)
            {
                updateDiedScene(ioGameState, inputResult, diedRestorePressCount, bIoRestoredFromDied);
                return false;
            }

            if (ioGameState.scene_.current_ == game::GameScene::Fin)
            {
                game::updateSceneByInput(ioGameState, inputResult);
                return false;
            }

            if (ioGameState.scene_.current_ == game::GameScene::Cutscene)
            {
                updateCutscene(ioGameState, inputResult, currFrameDeltaTime);
                return false;
            }

            if (ioGameState.scene_.current_ == game::GameScene::FinalCutscene)
            {
                updateFinalCutscene(ioGameState, frameClock, ioFixedStepCount);
                return false;
            }

            return true;
        }

        void RuntimeCoordinator::beginGameSceneUpdate(const game::GameState& gameState) noexcept
        {
            state_.prevLastLitGoalIndex_ = gameState.goal_.lastLitIndex_;
            state_.bHasPrevLastLitGoalIndex_ = true;
        }

        void RuntimeCoordinator::endGameSceneUpdate(const game::GameState& gameState) noexcept
        {
            if (!state_.bHasPrevLastLitGoalIndex_)
            {
                return;
            }

            const int currLastLitGoalIndex = gameState.goal_.lastLitIndex_;

            if (currLastLitGoalIndex != state_.prevLastLitGoalIndex_ && currLastLitGoalIndex >= 0)
            {
                markGoalCheckpointPending(currLastLitGoalIndex, gameState.goal_.wave_.bActive_);
            }

            state_.bHasPrevLastLitGoalIndex_ = false;
        }

        void RuntimeCoordinator::finalizeSceneUpdate(game::GameState& ioGameState,
            const game::GameState& initialSnapshot, game::GameScene prevScene, bool bRestoredFromDied,
            const game::GameState& preUpdateSnapshot, bool bHasPreUpdateSnapshot,
            int& diedRestorePressCount) noexcept
        {
            applySceneTransition(ioGameState, initialSnapshot, prevScene, bRestoredFromDied, preUpdateSnapshot,
                bHasPreUpdateSnapshot, diedRestorePressCount);

            finalizeGoalCheckpoint(ioGameState);
        }

        void RuntimeCoordinator::updateIntroScene(game::GameState& ioGameState, const ports::InputResult& inputResult,
            core::types::Duration currFrameDeltaTime) const
        {
            const float dtSeconds =
                std::chrono::duration_cast<std::chrono::duration<float>>(currFrameDeltaTime).count();

            ioGameState.scene_.introUiPhaseSeconds_ += dtSeconds;

            const float targetSeconds =
                ioGameState.scene_.bIntroUiVisible_ ? ioGameState.scene_.introUiShowSeconds_ : ioGameState.scene_.introUiHideSeconds_;

            if (ioGameState.scene_.introUiPhaseSeconds_ >= targetSeconds)
            {
                ioGameState.scene_.introUiPhaseSeconds_ = 0.0f;
                ioGameState.scene_.bIntroUiVisible_ = !ioGameState.scene_.bIntroUiVisible_;
            }

            game::updateSceneByInput(ioGameState, inputResult);
        }

        void RuntimeCoordinator::updateCutscene(game::GameState& ioGameState, const ports::InputResult& inputResult,
            core::types::Duration currFrameDeltaTime) const
        {
            const float dtSeconds =
                std::chrono::duration_cast<std::chrono::duration<float>>(currFrameDeltaTime).count();

            auto& cs = ioGameState.scene_.cutscene_;

            cs.inputBlockSecAccumulator_ += dtSeconds;

            cs.bIsInputAllowed = cs.inputBlockSecAccumulator_ >= game::kInputBlockSeconds;

            if (cs.bIsInputAllowed && inputResult.bAnyKeyPressed_)
            {
                ioGameState.scene_.current_ = game::GameScene::Game;
                return;
            }

            cs.cutsceneSecAccumulator_ += dtSeconds;

            const int shouldShowChars =
                static_cast<int>(std::floor(cs.cutsceneSecAccumulator_ * static_cast<float>(game::kCharsPerSecond)));

            const int totalChars = game::getCutsceneTextTotalChars();

            cs.shownCharCount_ = (shouldShowChars < totalChars) ? shouldShowChars : totalChars;

            if (cs.shownCharCount_ >= totalChars)
            {
                cs.bFinished_ = true;

                cs.holdSecAccumulator_ += dtSeconds;

                if (cs.holdSecAccumulator_ >= game::kHoldSeconds)
                {
                    ioGameState.scene_.current_ = game::GameScene::Game;
                    return;
                }
            }
        }

        void RuntimeCoordinator::updateFinalCutscene(
            game::GameState& ioGameState, FrameClock& frameClock, int& ioFixedStepCount) const noexcept
        {
            const auto fixedDeltaTime = frameClock.getFixedStep();

            ioFixedStepCount = frameClock.consumeFixedSteps();

            const float dtSeconds = std::chrono::duration_cast<std::chrono::duration<float>>(fixedDeltaTime).count();

            auto& fc = ioGameState.scene_.finalCutscene_;

            const int bgWidth = ioGameState.assets_.gameSceneBackground_.tileMap_.getWidth();
            const int bgHeight = ioGameState.assets_.gameSceneBackground_.tileMap_.getHeight();

            const core::types::Vec2 bgWorldSize{bgWidth, bgHeight};

            for (int i = 0; i < ioFixedStepCount; ++i)
            {
                if (fc.phase_ == game::FinalCutscenePhase::PanningUp)
                {
                    fc.ticksUntilNextCell_ -= 1;

                    if (fc.ticksUntilNextCell_ <= 0)
                    {
                        fc.ticksUntilNextCell_ = game::kFinalCutsceneTicksPerCell;

                        if (fc.cameraOriginY_ > fc.endCameraOriginY_)
                        {
                            fc.cameraOriginY_ -= 1;
                        }

                        if (fc.cameraOriginY_ <= fc.endCameraOriginY_)
                        {
                            fc.cameraOriginY_ = fc.endCameraOriginY_;
                            fc.phase_ = game::FinalCutscenePhase::Hold;
                        }

                        const int currentOriginX = ioGameState.mainCamera_.getViewportOriginXAtWorld();

                        const int cameraCenterX = currentOriginX + (world::Camera::kViewportWidth / 2);
                        const int cameraCenterY = fc.cameraOriginY_ + (world::Camera::kViewportHeight / 2);

                        int desiredOriginX = currentOriginX;

                        bool bFoundTargetGoal = false;
                        int  bestDeltaY = 0;
                        int  bestAbsDx = 0;

                        for (std::size_t gi = 0; gi < ioGameState.goal_.goalRegions_.size(); ++gi)
                        {
                            const auto& goalCell = ioGameState.goal_.goalRegions_[gi].goalWorldCell_;

                            if (goalCell.y_ >= cameraCenterY)
                            {
                                continue;
                            }

                            const int deltaY = cameraCenterY - goalCell.y_;
                            const int absDx = std::abs(goalCell.x_ - cameraCenterX);

                            if (!bFoundTargetGoal)
                            {
                                bFoundTargetGoal = true;
                                bestDeltaY = deltaY;
                                bestAbsDx = absDx;
                                desiredOriginX = goalCell.x_ - (world::Camera::kViewportWidth / 2);
                                continue;
                            }

                            if (deltaY < bestDeltaY)
                            {
                                bestDeltaY = deltaY;
                                bestAbsDx = absDx;
                                desiredOriginX = goalCell.x_ - (world::Camera::kViewportWidth / 2);
                                continue;
                            }

                            if ((deltaY == bestDeltaY) && (absDx < bestAbsDx))
                            {
                                bestAbsDx = absDx;
                                desiredOriginX = goalCell.x_ - (world::Camera::kViewportWidth / 2);
                                continue;
                            }
                        }

                        fc.targetCameraOriginX_ = desiredOriginX;
                    }

                    int currentOriginX = ioGameState.mainCamera_.getViewportOriginXAtWorld();

                    if (currentOriginX < fc.targetCameraOriginX_)
                    {
                        currentOriginX += 1;
                    }
                    else if (currentOriginX > fc.targetCameraOriginX_)
                    {
                        currentOriginX -= 1;
                    }

                    ioGameState.mainCamera_.setViewportOriginAtWorld({currentOriginX, fc.cameraOriginY_}, bgWorldSize);
                }
                else
                {
                    fc.holdRemainingSeconds_ -= dtSeconds;

                    if (fc.holdRemainingSeconds_ <= 0.0f)
                    {
                        ioGameState.scene_.current_ = game::GameScene::Fin;
                        return;
                    }
                }
            }
        }

        void RuntimeCoordinator::updateDiedScene(game::GameState& ioGameState, const ports::InputResult& inputResult,
            int& diedRestorePressCount, bool& bIoRestoredFromDied) const noexcept
        {
            game::updateSceneByInput(ioGameState, inputResult);

#if FUSOU_DEV
            if (inputResult.bRPressed_)
            {
                diedRestorePressCount += 1;
            }

            if (diedRestorePressCount < 3)
            {
                return;
            }

            diedRestorePressCount = 0;

            if (!restoreFromDiedSnapshot(ioGameState))
            {
                return;
            }

            bIoRestoredFromDied = true;
#else
            diedRestorePressCount = 0;
#endif
        }

        void RuntimeCoordinator::applySceneTransition(game::GameState& ioGameState,
            const game::GameState& initialSnapshot, game::GameScene prevScene, bool bRestoredFromDied,
            const game::GameState& preUpdateSnapshot, bool bHasPreUpdateSnapshot,
            int& diedRestorePressCount) noexcept
        {
            if (!bRestoredFromDied && prevScene != game::GameScene::Game && ioGameState.scene_.current_ == game::GameScene::Game)
            {
                ioGameState = initialSnapshot;
                ioGameState.scene_.current_ = game::GameScene::Game;
                ioGameState.goal_.bIsClear_ = false;
            }

            if (prevScene == game::GameScene::Intro && ioGameState.scene_.current_ != game::GameScene::Intro)
            {
                ioGameState.scene_.introUiPhaseSeconds_ = 0.0f;
                ioGameState.scene_.bIntroUiVisible_ = true;
            }

            if (prevScene == game::GameScene::Game && ioGameState.scene_.current_ == game::GameScene::Died
                && bHasPreUpdateSnapshot)
            {
                saveDiedRestoreSnapshot(preUpdateSnapshot);
                diedRestorePressCount = 0;
            }
        }

        void RuntimeCoordinator::saveDiedRestoreSnapshot(const game::GameState& snapshot) noexcept
        {
            state_.diedRestoreSnapshot_ = snapshot;
            state_.bHasDiedRestoreSnapshot_ = true;
        }

        bool RuntimeCoordinator::hasGoalRestoreSnapshot() const noexcept
        {
            return state_.bHasGoalRestoreSnapshot_;
        }

        void RuntimeCoordinator::clearGoalCheckpointTracking() noexcept
        {
            clearPendingGoalSnapshot();
            state_.bPrevGoalWaveActive_ = false;
            state_.prevLastLitGoalIndex_ = -1;
            state_.bHasPrevLastLitGoalIndex_ = false;
        }

        void RuntimeCoordinator::markGoalCheckpointPending(int goalIndex, bool bGoalWaveActive) noexcept
        {
            state_.bPendingGoalSnapshot_ = true;
            state_.pendingGoalIndex_ = goalIndex;
            state_.bPrevGoalWaveActive_ = bGoalWaveActive;
        }

        void RuntimeCoordinator::finalizeGoalCheckpoint(const game::GameState& gameState) noexcept
        {
            const bool bIsGameScene = (gameState.scene_.current_ == game::GameScene::Game);
            const bool bCurrGoalWaveActive = bIsGameScene ? gameState.goal_.wave_.bActive_ : false;
            const bool bWaveJustEnded = (state_.bPrevGoalWaveActive_ && !bCurrGoalWaveActive);

            state_.bPrevGoalWaveActive_ = bCurrGoalWaveActive;

            if (!bIsGameScene)
            {
                clearPendingGoalSnapshot();
                return;
            }

            if (!state_.bPendingGoalSnapshot_ || !bWaveJustEnded)
            {
                return;
            }

            if (gameState.goal_.goalRegions_.empty())
            {
                clearPendingGoalSnapshot();
                return;
            }

            const int finalGoalIndex = static_cast<int>(gameState.goal_.goalRegions_.size()) - 1;

            if (state_.pendingGoalIndex_ >= 0 && state_.pendingGoalIndex_ != finalGoalIndex)
            {
                state_.goalRestoreSnapshot_ = gameState;
                state_.bHasGoalRestoreSnapshot_ = true;
            }

            clearPendingGoalSnapshot();
        }

        void RuntimeCoordinator::restoreFromInitialSnapshot(
            game::GameState& ioGameState, const game::GameState& initialSnapshot) const noexcept
        {
            ioGameState = initialSnapshot;
            ioGameState.scene_.current_ = game::GameScene::Intro;
        }

        bool RuntimeCoordinator::restoreFromDiedSnapshot(game::GameState& ioGameState) const noexcept
        {
            if (!state_.bHasDiedRestoreSnapshot_)
            {
                return false;
            }

            ioGameState = state_.diedRestoreSnapshot_;
            ioGameState.scene_.current_ = game::GameScene::Game;

            applyDiedRestore(ioGameState);
            return true;
        }

        bool RuntimeCoordinator::restoreFromGoalCheckpoint(game::GameState& ioGameState) noexcept
        {
            if (!state_.bHasGoalRestoreSnapshot_)
            {
                return false;
            }

            ioGameState = state_.goalRestoreSnapshot_;
            ioGameState.scene_.current_ = game::GameScene::Game;

            applyGoalCheckpointRestore(ioGameState.goal_.wave_.bActive_);
            return true;
        }

        void RuntimeCoordinator::applyDiedRestore(game::GameState& ioGameState) const noexcept
        {
            ioGameState.player_.currLives_ = ioGameState.player_.maxLives_;

            core::types::Vec2 respawnWorldCell{};
            if (ioGameState.goal_.lastLitIndex_ >= 0)
            {
                const auto  instructionIndex = static_cast<std::size_t>(ioGameState.goal_.lastLitIndex_);
                const auto& goalRegion = ioGameState.goal_.goalRegions_[instructionIndex];
                respawnWorldCell = goalRegion.goalWorldCell_;
            }
            else
            {
                respawnWorldCell = ioGameState.player_.spawnCell_;
            }

            ioGameState.player_.feetWorldPos_ =
                core::types::Vec2f{static_cast<float>(respawnWorldCell.x_), static_cast<float>(respawnWorldCell.y_)};

            ioGameState.player_.velocity_ = core::types::Vec2f{0.0f, 0.0f};
            ioGameState.player_.movementState_ = game::PlayerMovementState::Normal;
            ioGameState.player_.bGrounded_ = false;

        }

        void RuntimeCoordinator::applyGoalCheckpointRestore(bool bGoalWaveActive) noexcept
        {
            clearPendingGoalSnapshot();
            state_.bPrevGoalWaveActive_ = bGoalWaveActive;
        }

        void RuntimeCoordinator::clearPendingGoalSnapshot() noexcept
        {
            state_.bPendingGoalSnapshot_ = false;
            state_.pendingGoalIndex_ = -1;
        }
    } // namespace loop
} // namespace core
