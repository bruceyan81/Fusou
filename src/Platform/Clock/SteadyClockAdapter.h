// Copyright (c) 2026 Bruce Yan. All Rights Reserved.

/**
 * @brief steady clock を core clock port へ適配する class を定義する
 */

#pragma once

#include "src/Core/Ports/ClockPort.h"

namespace platform
{
    namespace clock
    {
        class SteadyClockAdapter final : public core::ports::ClockPort
        {
        public:
            core::types::TimePoint getNow() noexcept override;
            void                   sleepUntil(core::types::TimePoint timePoint) noexcept override;
        };
    } // namespace clock
} // namespace platform