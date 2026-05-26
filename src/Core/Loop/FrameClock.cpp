// Copyright (c) 2026 Bruce Yan. All Rights Reserved.

/**
 * @brief clock port から frame delta と固定 Tick 数を計算する
 */

#include "src/Core/Loop/FrameClock.h"

namespace core
{
    namespace loop
    {
        FrameClock::FrameClock(ports::ClockPort& clockPort, FrameClockConfig config)
            : clockPort_(clockPort), config_(config)
        {
            previousTime_ = clockPort_.getNow();
            nextFrameTime_ = previousTime_;
        }

        types::Duration FrameClock::beginFrame() noexcept
        {
            const auto currentTime = clockPort_.getNow();
            auto       frameTime = currentTime - previousTime_;

            previousTime_ = currentTime;

            accumulator_ += frameTime;

            // spiral of death を避ける
            if (accumulator_ > config_.maxAccumulatedTime_)
            {
                accumulator_ = config_.maxAccumulatedTime_;
            }

            return frameTime;
        }

        int FrameClock::consumeFixedSteps() noexcept
        {
            int stepCount{0};

            while (accumulator_ >= config_.fixedTimestep_)
            {
                accumulator_ -= config_.fixedTimestep_;
                ++stepCount;
            }

            // 今回のフレームに行う Tick 数
            return stepCount;
        }

        void FrameClock::endFrame() noexcept
        {
            nextFrameTime_ += config_.targetFrameTime_;

            const auto currentTime = clockPort_.getNow();

            if (currentTime < nextFrameTime_)
            {
                clockPort_.sleepUntil(nextFrameTime_);
                return;
            }

            nextFrameTime_ = currentTime;
        }
    } // namespace loop
} // namespace core