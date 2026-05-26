// Copyright (c) 2026 Bruce Yan. All Rights Reserved.

/**
 * @brief light の runtime state と操作補助関数を定義する
 */

#pragma once

#include "src/Core/Game/GameConstants.h"
#include "src/Core/Types/Types.h"
#include "src/Core/World/Camera.h"
#include "src/Core/World/TileMap.h"

namespace core
{
    namespace game
    {
        struct LightState final
        {
            bool bEnabled_{false};

            types::Vec2 lightWorldCell_{};

            types::Vec2 lightViewportCell_{};

            types::Vec2 dir_{};

            int repeatMoveTickAccumulator_{};

            bool bIsRepeatingMove_{false};

            bool bIsMoveHeldForRepeat_{false};

            bool isLit(types::Vec2 viewportCell) const noexcept;

            void resetRepeatMoveState(types::Vec2 newDir = {}) noexcept;

            bool isHiddenTile(const world::TileId tileId) const noexcept
            {
                return ((tileId == world::tileid::kInvisibleBlackBlock)
                    || (tileId == world::tileid::kInvisibleLadder));
            }

        };
    } // namespace game
} // namespace core
