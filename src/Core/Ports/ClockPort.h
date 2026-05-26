// Copyright (c) 2026 Bruce Yan. All Rights Reserved.

/**
 * @brief platform clock の抽象 interface を定義する
 */

#pragma once

#include "src/Core/Types/Types.h"

namespace core
{
    namespace ports
    {
        /**
         * @brief ゲームループの時間を制御する抽象化ポート
         */
        class ClockPort
        {
        public:
            virtual ~ClockPort() noexcept = default;

            /**
             * @brief 現時点を取得
             * @return std::chrono::time_point
             */
            [[nodiscard]] virtual types::TimePoint getNow() noexcept = 0;

            /**
             * @brief 指定時刻までスリープする
             * @param timePoint 復帰の時刻
             */
            virtual void sleepUntil(types::TimePoint timePoint) noexcept = 0;
        };
    } // namespace ports
} // namespace core
