// Copyright (c) 2026 Bruce Yan. All Rights Reserved.

/**
 * @brief player の runtime state を定義する
 */

#pragma once

#include "src/Core/Types/Types.h"

namespace core
{
    namespace game
    {
        enum class PlayerMovementState
        {
            Normal,
            OnLadder,
        };

        struct PlayerState final
        {
            types::Vec2 spawnCell_{};

            // プレイヤーが足を原点として扱う
            types::Vec2f feetWorldPos_{};

            types::Vec2f velocity_{};

            bool bGrounded_{false};

            PlayerMovementState movementState_{PlayerMovementState::Normal};

            int currLives_{};
            int maxLives_{};
        };
    } // namespace game
} // namespace core
