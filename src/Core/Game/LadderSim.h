// Copyright (c) 2026 Bruce Yan. All Rights Reserved.

/**
 * @brief ladder 移動判定と状態更新 API を公開する
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
         * @brief 1 Tick 内の水平移動意図と Ladder 移動意図をまとめて持つ
         */
        struct PlayerMovement final
        {
            // moveIntentX_ ∈ [1, 0, -1]
            int moveIntentX_{0};

            // climbIntentY_ ∈ [1, 0, -1]
            int climbIntentY_{0};

            bool bHasMoveInput_{false};
        };

        /**
         * @brief Ladder step consume の結果
         * @note GameTick 側は move 後に stateMovement を補正するため、最小限の結果だけ受け取る
         */
        struct LadderMoveResult final
        {
            bool                 bConsumedY_{false};
            ports::StepDirection stepDir_{ports::StepDirection::None};
        };

        /**
         * @brief Ladder の事前チェックを行う
         * @note 主に hidden ladder を失ったとき、OnLadder から脱離するために使う
         */
        void runLadderPrecheck(GameState& gameState, const types::Vec2& playerFeetCellBeforeMove) noexcept;

        /**
         * @brief PlayerSim に協力して、move 前の Ladder モード処理を行う
         * この入口は以下を扱う
         * - step による 1 Cell 移動の消費
         * - 上下入力による Ladder への attach
         */
        [[nodiscard]] LadderMoveResult runLadderMoveMode(GameState& gameState, PlayerMovement& movement,
            const ports::InputResult& inputResult, const types::Vec2& playerFeetCellBeforeMove) noexcept;

        /**
         * @brief move 後に Ladder 状態を収束させる
         */
        void runLadderStateFinalize(GameState& gameState, const PlayerMovement& stateMovement) noexcept;
    } // namespace game
} // namespace core
