// Copyright (c) 2026 Bruce Yan. All Rights Reserved.

/**
 * @brief Windows runtime 用 steady clock adapter を実装する
 */

#include "src/Platform/Clock/SteadyClockAdapter.h"

#include <thread>

namespace platform
{
    namespace clock
    {
        core::types::TimePoint SteadyClockAdapter::getNow() noexcept
        {
            return core::types::Clock::now();
        }

        void SteadyClockAdapter::sleepUntil(core::types::TimePoint timePoint) noexcept
        {
            std::this_thread::sleep_until(timePoint);
        }
    } // namespace clock
} // namespace platform