// Copyright (c) 2026 Bruce Yan. All Rights Reserved.

/**
 * @brief player と敵の接触処理 API を公開する
 */

#pragma once

#include "src/Core/Game/GameModel.h"

namespace core
{
    namespace game
    {
        /**
         * @brief プレイヤーと敵に衝突した後の処理
         */
        [[nodiscard]] bool runPlayerContact(GameState& gameState) noexcept;
    } // namespace game
} // namespace core
