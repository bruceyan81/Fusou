// Copyright (c) 2026 Bruce Yan. All Rights Reserved.

/**
 * @brief frame 時間と固定 Tick 蓄積を管理する class を定義する
 */

#pragma once

#include "src/Core/Ports/ClockPort.h"
#include "src/Core/Types/Types.h"

namespace core
{
    namespace loop
    {
        /**
         * @note chrono_literals
         * operator""h は hour
         * operator""min は minute
         * operator""s は seconds
         * operator""ms は milliseconds
         */
        using namespace std::chrono_literals;

        /**
         * @brief 固定更新とフレーム制御のため
         */
        struct FrameClockConfig final
        {
            // Tick を進める固定な時間
            types::Duration fixedTimestep_{std::chrono::duration_cast<types::Duration>(16ms)};

            // 長い間に停止の場合、短時間に激しく更新するのを避けるため、累積可能な最大時間
            types::Duration maxAccumulatedTime_{std::chrono::duration_cast<types::Duration>(160ms)};

            // endFrame にとって、次フレームまで待機する基準時間
            types::Duration targetFrameTime_{std::chrono::duration_cast<types::Duration>(16ms)};
        };

        /**
         * @brief フレーム進行を管理するクラス
         */
        class FrameClock final
        {
        public:
            FrameClock(ports::ClockPort& clockPort, FrameClockConfig config);

            /**
             * @brief フレーム開始の処理関数
             * 経過時間を accumulator_ に加算し、状況よりクランプする
             * @return 前フレームからの経過時間
             */
            types::Duration beginFrame() noexcept;

            [[nodiscard]] types::Duration getFixedStep() const noexcept
            {
                return config_.fixedTimestep_;
            }

            /**
             * @brief accumulator_ に溜まる時間を fixedStep_ で分けて、
             * Tick と Update の回数を計算する
             * @return 実行回数（>= 0）
             */
            int consumeFixedSteps() noexcept;

            /**
             * @brief 1 フレームの終了処理
             * 現時点の時間は加算した結果より早い場合 sleep_until で nextFrameTime_ まで待たせる
             */
            void endFrame() noexcept;

        private:
            ports::ClockPort& clockPort_;
            FrameClockConfig  config_;

            types::TimePoint previousTime_{};
            types::TimePoint nextFrameTime_{};
            types::Duration  accumulator_{types::Duration::zero()};
        };
    } // namespace loop
} // namespace core
