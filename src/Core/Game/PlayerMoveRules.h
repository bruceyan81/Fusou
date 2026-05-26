// Copyright (c) 2026 Bruce Yan. All Rights Reserved.

/**
 * @brief player 移動に必要な当たり判定 API を公開する
 */

#pragma once

#include "src/Core/Game/LightState.h"
#include "src/Core/Types/Types.h"
#include "src/Core/World/Camera.h"
#include "src/Core/World/TileMap.h"

namespace core
{
    namespace game
    {
        /**
         * @brief Player の Feet の float 座標を左上の world cell へ変換する
         */
        [[nodiscard]] types::Vec2 feetWorldPosToTopLeftCell(const types::Vec2f& playerFeetWorldPos) noexcept;

        /**
         * @brief Player の Feet の float 座標を world cell へ変換する
         */
        [[nodiscard]] types::Vec2 getPlayerFeetCell(const types::Vec2f& playerFeetWorldPos) noexcept;

        /**
         * @brief Player 自身が占有する 1 x 2 のセルを使って移動可能か判定する
         */
        [[nodiscard]] bool canPlayerMoveTo(const world::TileMap& map, const world::TileMap& player,
            const types::Vec2& currPlayerTopLeftCell, const types::Vec2& candidateTopLeftCell,
            const LightState& lightState, const world::Camera& camera, bool bTreatClimbableLadderAsBlocking) noexcept;

        /**
         * @brief Feet 座標を使って移動可能か判定する
         */
        [[nodiscard]] bool canPlayerMoveToByFeet(const world::TileMap& map, const world::TileMap& player,
            const types::Vec2f& currPlayerFeetWorldPos, const types::Vec2f& candidatePlayerFeetWorldPos,
            const LightState& lightState, const world::Camera& camera, bool bTreatClimbableLadderAsBlocking) noexcept;
    } // namespace game
} // namespace core
