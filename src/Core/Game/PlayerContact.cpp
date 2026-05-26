// Copyright (c) 2026 Bruce Yan. All Rights Reserved.

/**
 * @brief player と敵の接触による damage respawn died 遷移を処理する
 */

#include "src/Core/Game/Enemy.h"
#include "src/Core/Game/GameModel.h"
#include "src/Core/Game/PlayerContact.h"

#include <cstddef>
#include <cmath>

namespace core
{
    namespace
    {
        /**
         * @brief プレイヤーと敵の接触判定
         * Player は head と feet の 2 cell として扱い、敵の worldPos を cell に落として重なりを判定する
         */
        [[nodiscard]] bool isPlayerEnemyContacted(const game::GameState& gameState) noexcept
        {
            const types::Vec2 playerTopLeft{static_cast<int>(std::floor(gameState.player_.feetWorldPos_.x_)),
                static_cast<int>(std::floor(gameState.player_.feetWorldPos_.y_)) - 1};

            const types::Vec2 playerHead{playerTopLeft.x_, playerTopLeft.y_};
            const types::Vec2 playerFeet{playerTopLeft.x_, playerTopLeft.y_ + 1};

            for (const game::Enemy& enemy : gameState.enemyRuntime_.enemies_)
            {
                const types::Vec2 enemyCell{
                    static_cast<int>(std::floor(enemy.worldPos_.x_)), static_cast<int>(std::floor(enemy.worldPos_.y_))};

                if ((enemyCell == playerHead) || (enemyCell == playerFeet))
                {
                    return true;
                }
            }

            return false;
        }

        [[nodiscard]] types::Vec2 resolveRespawnWorldCell(const game::GameState& gameState) noexcept
        {
            if (gameState.goal_.lastLitIndex_ >= 0)
            {
                const game::GoalActivationRegion& goalRegion =
                    gameState.goal_.goalRegions_[static_cast<std::size_t>(gameState.goal_.lastLitIndex_)];

                return goalRegion.goalWorldCell_;
            }

            return gameState.player_.spawnCell_;
        }

        void resetPlayerRuntimeStateAfterContact(
            game::GameState& gameState, const types::Vec2& respawnWorldCell) noexcept
        {
            gameState.player_.feetWorldPos_ = respawnWorldCell.toVec2f();
            gameState.player_.velocity_ = {0.0f, 0.0f};
            gameState.player_.movementState_ = game::PlayerMovementState::Normal;
            gameState.player_.bGrounded_ = false;
        }
    } // namespace

    namespace game
    {
        bool runPlayerContact(GameState& gameState) noexcept
        {
            if (!isPlayerEnemyContacted(gameState))
            {
                return false;
            }

            gameState.player_.currLives_ -= 1;

            if (gameState.player_.currLives_ <= 0)
            {
                gameState.goal_.bIsClear_ = false;
                gameState.scene_.current_ = GameScene::Died;
                return true;
            }

            const types::Vec2 respawnWorldCell = resolveRespawnWorldCell(gameState);
            resetPlayerRuntimeStateAfterContact(gameState, respawnWorldCell);

            return true;
        }
    } // namespace game
} // namespace core
