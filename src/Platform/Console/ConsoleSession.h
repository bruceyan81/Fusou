// Copyright (c) 2026 Bruce Yan. All Rights Reserved.

/**
 * @brief console session の初期化と終了処理を管理する class を定義する
 */

#pragma once

namespace platform
{
    namespace console
    {
        /**
         * @brief Win32 Console のセッション境界を RAII で管理するクラス
         */
        class ConsoleSession final
        {
        public:
            /**
             * @brief Win32 Console を初期化する
             * @throws std::runtime_error
             */
            ConsoleSession();

            // コピーおよびムーブを禁止し、ConsoleSession のライフタイムを保証する
            ConsoleSession(const ConsoleSession&) = delete;
            ConsoleSession& operator=(const ConsoleSession&) = delete;
            ConsoleSession(ConsoleSession&&) = delete;
            ConsoleSession& operator=(ConsoleSession&&) = delete;

            /**
             * @brief Win32 Console を cleanup する
             */
            ~ConsoleSession() noexcept;

        private:
            bool bIsStarted_ = false;
        };
    } // namespace console
} // namespace platform
