// Copyright (c) 2026 Bruce Yan. All Rights Reserved.

/**
 * @brief ladder 上の移動と通常移動への切り替えを処理する
 */

#include "src/Core/Game/LadderSim.h"
#include "src/Core/Game/PlayerMoveRules.h"

#include "src/Core/Game/TileRules.h"

namespace core
{
    namespace game
    {
        namespace
        {
            /**
             * @brief Hidden Ladder から落ちる処理
             */
            void detachFromLadderToFall(GameState& gameState) noexcept
            {
                gameState.player_.movementState_ = PlayerMovementState::Normal;

                gameState.player_.velocity_.x_ = 0.0f;
                gameState.player_.velocity_.y_ = 0.0f;
            }

            [[nodiscard]] bool tryToAttachLadder(
                GameState& gameState, const PlayerMovement& movement, const types::Vec2& playerFeetCell) noexcept
            {
                if (gameState.player_.movementState_ != PlayerMovementState::Normal)
                {
                    return false;
                }

                if (isLadderClimbable(gameState.assets_.gameScene_.tileMap_, playerFeetCell, gameState.mainCamera_,
                        gameState.light_))
                {
                    gameState.player_.movementState_ = PlayerMovementState::OnLadder;

                    gameState.player_.velocity_.x_ = 0.0f;
                    gameState.player_.velocity_.y_ = 0.0f;

                    return true;
                }

                if (movement.climbIntentY_ > 0)
                {
                    const types::Vec2 feetCellBelow{playerFeetCell.x_, playerFeetCell.y_ + 1};

                    if (isLadderClimbable(gameState.assets_.gameScene_.tileMap_, feetCellBelow,
                            gameState.mainCamera_, gameState.light_))
                    {
                        gameState.player_.movementState_ = PlayerMovementState::OnLadder;

                        gameState.player_.velocity_.x_ = 0.0f;
                        gameState.player_.velocity_.y_ = 0.0f;

                        return true;
                    }
                }

                return false;
            }

            void updatePlayerMovementStateAfterMove(GameState& gameState, const PlayerMovement& movement) noexcept
            {
                const types::Vec2 feetCell = getPlayerFeetCell(gameState.player_.feetWorldPos_);

                const bool bClimbable = isLadderClimbable(
                    gameState.assets_.gameScene_.tileMap_, feetCell, gameState.mainCamera_, gameState.light_);

                const types::Vec2 playerHeadCell = feetWorldPosToTopLeftCell(gameState.player_.feetWorldPos_);
                const types::Vec2 playerFeetCell{playerHeadCell.x_, playerHeadCell.y_ + 1};
                const types::Vec2 underFeetCell{playerFeetCell.x_, playerFeetCell.y_ + 1};

                const bool bHasSupportBelow = isBlockingForPlayer(gameState.assets_.gameScene_.tileMap_, underFeetCell,
                    gameState.mainCamera_, gameState.light_, playerHeadCell, playerFeetCell, false);

                if (gameState.player_.movementState_ == PlayerMovementState::Normal)
                {
                    return;
                }

                if (!bClimbable)
                {
                    detachFromLadderToFall(gameState);
                    return;
                }

                if (bHasSupportBelow && (movement.climbIntentY_ == 0) && (movement.moveIntentX_ != 0))
                {
                    detachFromLadderToFall(gameState);
                    return;
                }

                if (bHasSupportBelow && (movement.climbIntentY_ == 0))
                {
                    detachFromLadderToFall(gameState);
                    return;
                }
            }
        } // namespace

        void runLadderPrecheck(GameState& gameState, const types::Vec2& playerFeetCellBeforeMove) noexcept
        {
            if (gameState.player_.movementState_ != PlayerMovementState::OnLadder)
            {
                return;
            }

            const bool bClimbable = isLadderClimbable(gameState.assets_.gameScene_.tileMap_, playerFeetCellBeforeMove,
                gameState.mainCamera_, gameState.light_);

            if (bClimbable)
            {
                return;
            }

            detachFromLadderToFall(gameState);
        }

        void runLadderMoveMode(
            GameState& gameState, const PlayerMovement& movement, const types::Vec2& playerFeetCellBeforeMove) noexcept
        {
            if (movement.climbIntentY_ != 0)
            {
                (void)tryToAttachLadder(gameState, movement, playerFeetCellBeforeMove);
            }
        }

        void runLadderStateFinalize(GameState& gameState, const PlayerMovement& movement) noexcept
        {
            updatePlayerMovementStateAfterMove(gameState, movement);
        }
    } // namespace game
} // namespace core
