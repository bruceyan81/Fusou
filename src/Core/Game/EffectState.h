// Copyright (c) 2026 Bruce Yan. All Rights Reserved.

/**
 * @brief 表示 effect の runtime state を定義する
 */

#pragma once

#include "src/Core/Types/Types.h"

namespace core
{
    namespace game
    {
        /**
         * @brief Runtime-only presentation timers shared by scene effects
         */
        struct EffectRuntimeState final
        {
            types::Duration bgTime_{};
        };
    } // namespace game
} // namespace core
