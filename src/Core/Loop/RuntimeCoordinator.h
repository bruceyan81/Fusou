// Copyright (c) 2026 Bruce Yan. All Rights Reserved.

/**
 * @brief scene 更新と復元 snapshot を調停する class を定義する
 */

#pragma once

#include "src/Core/Game/GameModel.h"
#include "src/Core/Ports/InputPort.h"

namespace core
{
    namespace loop
    {
        class FrameClock;

        struct RuntimeCoordinatorState final
        {
            game::GameState diedRestoreSnapshot_{};

            bool bHasDiedRestoreSnapshot_ = false;

            game::GameState goalRestoreSnapshot_{};

            bool bHasGoalRestoreSnapshot_ = false;

            bool bPendingGoalSnapshot_ = false;

            int pendingGoalIndex_ = -1;

            bool bPrevGoalWaveActive_ = false;

            int prevLastLitGoalIndex_ = -1;

            bool bHasPrevLastLitGoalIndex_ = false;
        };

        /**
         * @brief scene 更新、scene 遷移後処理、復元 snapshot のライフサイクルをまとめて管理する
         * Game scene の固定 Tick は GameLoop と GameTick が実行する
         */
        class RuntimeCoordinator final
        {
        public:
            // Game scene 以外へ出た時に goal checkpoint 追跡状態を消す
            void prepareSceneUpdate(game::GameScene scene) noexcept;

            /**
             * @brief 現在の scene を更新して Game 固定 Tick が必要か返す
             */
            [[nodiscard]] bool updateScene(
                game::GameState& ioGameState,
                FrameClock& frameClock,
                const ports::InputResult& inputResult,
                core::types::Duration currFrameDeltaTime,
                int& fixedStepCount,
                int& diedRestorePressCount,
                bool& bRestoredFromDied
            ) const;

            /**
             * @brief Game 固定 Tick 前に goal checkpoint の比較元を保存する
             */
            void beginGameSceneUpdate(const game::GameState& gameState) noexcept;

            /**
             * @brief Game 固定 Tick 後に新しい goal 点灯を検出する
             */
            void endGameSceneUpdate(const game::GameState& gameState) noexcept;

            /**
             * @brief Scene 遷移後の復元状態と checkpoint 状態を確定する
             */
            void finalizeSceneUpdate(game::GameState& ioGameState, const game::GameState& initialSnapshot,
                game::GameScene prevScene, bool bRestoredFromDied, const game::GameState& preUpdateSnapshot,
                bool bHasPreUpdateSnapshot, int& ioDiedRestorePressCount) noexcept;

            /**
             * @brief Goal checkpoint 復元用 snapshot が存在するか返す
             */
            bool hasGoalRestoreSnapshot() const noexcept;

            /**
             * @brief 初期 snapshot から Intro scene へ戻す
             */
            void restoreFromInitialSnapshot(
                game::GameState& gameState, const game::GameState& initialSnapshot) const noexcept;

            /**
             * @brief Died scene 用 snapshot から Game scene へ戻す
             */
            bool restoreFromDiedSnapshot(game::GameState& gameState) const noexcept;

            /**
             * @brief Goal checkpoint snapshot から Game scene へ戻す
             */
            bool restoreFromGoalCheckpoint(game::GameState& ioGameState) noexcept;

        private:
            /**
             * @brief Intro scene の点滅 UI と入力遷移を更新する
             */
            void updateIntroScene(game::GameState& ioGameState, const ports::InputResult& inputResult,
                core::types::Duration currFrameDeltaTime) const;

            /**
             * @brief Cutscene の文字送りと入力許可を更新する
             */
            void updateCutscene(game::GameState& ioGameState, const ports::InputResult& inputResult,
                core::types::Duration currFrameDeltaTime) const;

            /**
             * @brief Final cutscene の camera 移動を固定 Tick で進める
             */
            void updateFinalCutscene(
                game::GameState& ioGameState, FrameClock& frameClock, int& ioFixedStepCount) const noexcept;

            /**
             * @brief Died scene の入力と Dev 用復元を処理する
             */
            void updateDiedScene(
                game::GameState& gameState, 
                const ports::InputResult& inputResult,
                int& diedRestorePressCount,
                bool& bRestoredFromDied
            ) const noexcept;

            void applySceneTransition(
                game::GameState& ioGameState, 
                const game::GameState& initialSnapshot,
                game::GameScene prevScene, 
                bool bRestoredFromDied, 
                const game::GameState& preUpdateSnapshot,
                bool bHasPreUpdateSnapshot, 
                int& diedRestorePressCount
            ) noexcept;

            void saveDiedRestoreSnapshot(const game::GameState& snapshot) noexcept;

            void clearGoalCheckpointTracking() noexcept;

            void markGoalCheckpointPending(int goalIndex, bool bGoalWaveActive) noexcept;

            void finalizeGoalCheckpoint(const game::GameState& gameState) noexcept;

            void applyDiedRestore(game::GameState& ioGameState) const noexcept;

            void applyGoalCheckpointRestore(bool bGoalWaveActive) noexcept;

            void clearPendingGoalSnapshot() noexcept;

        private:
            RuntimeCoordinatorState state_{};
        };
    } // namespace loop
} // namespace core
