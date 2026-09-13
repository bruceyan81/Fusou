// Copyright (c) 2026 Bruce Yan. All Rights Reserved.

/**
 * @brief ladder 移動判定と状態更新 API を公開する
 */

#pragma once

#include "src/Core/Game/GameModel.h"
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
         * @brief Ladder の事前チェックを行う
         * @note 主に hidden ladder を失ったとき、OnLadder から脱離するために使う
         */
        void runLadderPrecheck(GameState& gameState, const types::Vec2& playerFeetCellBeforeMove) noexcept;

        /**
         * @brief PlayerSim に協力して、move 前の Ladder モード処理を行う
         * 上下入力による Ladder への attach を扱う
         */
        void runLadderMoveMode(
            GameState& gameState, const PlayerMovement& movement, const types::Vec2& playerFeetCellBeforeMove) noexcept;

        /**
         * @brief move 後に Ladder 状態を収束させる
         */
        void runLadderStateFinalize(GameState& gameState, const PlayerMovement& movement) noexcept;
    } // namespace game
} // namespace core
