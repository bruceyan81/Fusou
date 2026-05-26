// Copyright (c) 2026 Bruce Yan. All Rights Reserved.

/**
 * @brief player の移動先 cell に対する通行可否を判定する
 */

#include "src/Core/Game/PlayerMoveRules.h"

#include "src/Core/Game/TileRules.h"

#include <cmath>

namespace core
{
    namespace game
    {
        types::Vec2 feetWorldPosToTopLeftCell(const types::Vec2f& playerFeetWorldPos) noexcept
        {
            return types::Vec2{static_cast<int>(std::floor(playerFeetWorldPos.x_)),
                static_cast<int>(std::floor(playerFeetWorldPos.y_)) - 1};
        }

        types::Vec2 getPlayerFeetCell(const types::Vec2f& playerFeetWorldPos) noexcept
        {
            return types::Vec2{static_cast<int>(std::floor(playerFeetWorldPos.x_)),
                static_cast<int>(std::floor(playerFeetWorldPos.y_))};
        }

        bool canPlayerMoveTo(const world::TileMap& map, const world::TileMap& player,
            const types::Vec2& currPlayerTopLeftCell, const types::Vec2& candidateTopLeftCell,
            const LightState& lightState, const world::Camera& camera,
            const bool bTreatClimbableLadderAsBlocking) noexcept
        {
            const types::Vec2 playerHeadCell{currPlayerTopLeftCell.x_, currPlayerTopLeftCell.y_};
            const types::Vec2 playerFeetCell{currPlayerTopLeftCell.x_, currPlayerTopLeftCell.y_ + 1};

            for (int playerY = 0; playerY < player.getHeight(); ++playerY)
            {
                for (int playerX = 0; playerX < player.getWidth(); ++playerX)
                {
                    const int nextX = candidateTopLeftCell.x_ + playerX;
                    const int nextY = candidateTopLeftCell.y_ + playerY;

                    if (nextX < 0 || nextX >= map.getWidth() || nextY < 0 || nextY >= map.getHeight())
                    {
                        return false;
                    }

                    const types::Vec2 worldCell{nextX, nextY};

                    if (isBlockingForPlayer(
                            map, worldCell, camera, lightState, playerHeadCell, playerFeetCell, bTreatClimbableLadderAsBlocking))
                    {
                        return false;
                    }
                }
            }

            return true;
        }

        bool canPlayerMoveToByFeet(const world::TileMap& map, const world::TileMap& player,
            const types::Vec2f& currPlayerFeetWorldPos, const types::Vec2f& candidatePlayerFeetWorldPos,
            const LightState& lightState, const world::Camera& camera,
            const bool bTreatClimbableLadderAsBlocking) noexcept
        {
            const types::Vec2 currTopLeftCell = feetWorldPosToTopLeftCell(currPlayerFeetWorldPos);
            const types::Vec2 candidateTopLeftCell = feetWorldPosToTopLeftCell(candidatePlayerFeetWorldPos);

            return canPlayerMoveTo(
                map, player, currTopLeftCell, candidateTopLeftCell, lightState, camera, bTreatClimbableLadderAsBlocking);
        }
    } // namespace game
} // namespace core
