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

            [[nodiscard]] bool canAttachLadderForStep(
                const GameState& gameState, const ports::StepDirection dir, const types::Vec2& playerFeetCell) noexcept
            {
                if (gameState.player_.movementState_ == PlayerMovementState::OnLadder)
                {
                    return true;
                }

                if (isLadderClimbable(gameState.assets_.gameScene_.tileMap_, playerFeetCell, gameState.mainCamera_,
                        gameState.light_))
                {
                    return true;
                }

                if (dir == ports::StepDirection::Down)
                {
                    const types::Vec2 feetCellBelow{playerFeetCell.x_, playerFeetCell.y_ + 1};

                    if (isLadderClimbable(gameState.assets_.gameScene_.tileMap_, feetCellBelow,
                            gameState.mainCamera_, gameState.light_))
                    {
                        return true;
                    }
                }

                return false;
            }

            [[nodiscard]] bool isPlayerSupportedForHorizontalStep(
                const GameState& gameState, const types::Vec2& currFeetCell) noexcept
            {
                const types::Vec2 belowFeetCell{currFeetCell.x_, currFeetCell.y_ + 1};

                if (belowFeetCell.x_ < 0 || belowFeetCell.x_ >= gameState.assets_.gameScene_.tileMap_.getWidth()
                    || belowFeetCell.y_ < 0 || belowFeetCell.y_ >= gameState.assets_.gameScene_.tileMap_.getHeight())
                {
                    return true;
                }

                const types::Vec2 currTopLeftCell = feetWorldPosToTopLeftCell(gameState.player_.feetWorldPos_);
                const types::Vec2 playerHeadCell{currTopLeftCell.x_, currTopLeftCell.y_};
                const types::Vec2 playerFeetCell{currTopLeftCell.x_, currTopLeftCell.y_ + 1};

                constexpr bool kTreatClimbableLadderAsBlocking = false;

                return isBlockingForPlayer(gameState.assets_.gameScene_.tileMap_, belowFeetCell, gameState.mainCamera_,
                    gameState.light_, playerHeadCell, playerFeetCell, kTreatClimbableLadderAsBlocking);
            }

            [[nodiscard]] LadderMoveResult tryConsumePlayerStepOneCell(
                GameState& gameState,
                PlayerMovement& movement,
                const ports::InputResult& inputResult,
                const types::Vec2& playerFeetCellBeforeMove
            ) noexcept
            {
                LadderMoveResult result{};

                const ports::StepDirection stepDir = inputResult.stepPrimaryDir_;
                result.stepDir_ = stepDir;

                if (stepDir == ports::StepDirection::None)
                {
                    return result;
                }

                if (gameState.player_.movementState_ == PlayerMovementState::OnLadder
                    && (stepDir == ports::StepDirection::Left || stepDir == ports::StepDirection::Right))
                {
                    return result;
                }

                int dx = 0;
                int dy = 0;

                switch (stepDir)
                {
                    case ports::StepDirection::Left:
                        dx = -1;
                        break;

                    case ports::StepDirection::Right:
                        dx = 1;
                        break;

                    case ports::StepDirection::Up:
                        dy = -1;
                        break;

                    case ports::StepDirection::Down:
                        dy = 1;
                        break;

                    default:
                        return result;
                }

                const types::Vec2 currFeetCell = getPlayerFeetCell(gameState.player_.feetWorldPos_);

                if (dx != 0)
                {
                    if (gameState.player_.movementState_ != PlayerMovementState::Normal)
                    {
                        return result;
                    }

                    if (!isPlayerSupportedForHorizontalStep(gameState, currFeetCell))
                    {
                        return result;
                    }
                }

                if (dy != 0)
                {
                    if (!canAttachLadderForStep(gameState, stepDir, playerFeetCellBeforeMove))
                    {
                        return result;
                    }

                    if (gameState.player_.movementState_ == PlayerMovementState::Normal)
                    {
                        gameState.player_.movementState_ = PlayerMovementState::OnLadder;
                        gameState.player_.velocity_.x_ = 0.0f;
                        gameState.player_.velocity_.y_ = 0.0f;
                    }
                }

                const types::Vec2f currFeetPos = gameState.player_.feetWorldPos_;
                const types::Vec2f candidateFeetPos{
                    currFeetPos.x_ + static_cast<float>(dx), currFeetPos.y_ + static_cast<float>(dy)};

                if (!canPlayerMoveToByFeet(gameState.assets_.gameScene_.tileMap_, gameState.assets_.player_.tileMap_,
                        currFeetPos, candidateFeetPos, gameState.light_, gameState.mainCamera_, false))
                {
                    return result;
                }

                gameState.player_.feetWorldPos_ = candidateFeetPos;

                if (dx != 0)
                {
                    gameState.player_.velocity_.x_ = 0.0f;
                    movement.moveIntentX_ = 0;
                    movement.bHasMoveInput_ = false;
                }

                if (dy != 0)
                {
                    result.bConsumedY_ = true;

                    gameState.player_.velocity_.y_ = 0.0f;
                    movement.climbIntentY_ = 0;
                }

                return result;
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

        [[nodiscard]] LadderMoveResult runLadderMoveMode(
            GameState& gameState,
            PlayerMovement& movement,
            const ports::InputResult& inputResult,
            const types::Vec2& playerFeetCellBeforeMove
        ) noexcept
        {
            LadderMoveResult result =
                tryConsumePlayerStepOneCell(gameState, movement, inputResult, playerFeetCellBeforeMove);

            const types::Vec2 playerFeetCellAfterStep = getPlayerFeetCell(gameState.player_.feetWorldPos_);

            if (movement.climbIntentY_ != 0)
            {
                (void)tryToAttachLadder(gameState, movement, playerFeetCellAfterStep);
            }

            return result;
        }

        void runLadderStateFinalize(GameState& gameState, const PlayerMovement& stateMovement) noexcept
        {
            updatePlayerMovementStateAfterMove(gameState, stateMovement);
        }
    } // namespace game
} // namespace core
