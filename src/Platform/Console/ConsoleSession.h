// Copyright (c) 2026 Bruce Yan. All Rights Reserved.

/**
 * @brief console session の初期化と終了処理を管理する class を定義する
 */

#pragma once

#include <windows.h>

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
            struct ConsoleSnapshot final
            {
                HANDLE                     inputHandle_{nullptr};
                HANDLE                     outputHandle_{nullptr};
                DWORD                      inputMode_{0};
                DWORD                      outputMode_{0};
                CONSOLE_CURSOR_INFO        cursorInfo_{};
                CONSOLE_SCREEN_BUFFER_INFO screenBufferInfo_{};
                CONSOLE_FONT_INFOEX        fontInfo_{};
                HWND                       windowHandle_{nullptr};
                LONG_PTR                   windowStyle_{0};
                RECT                       windowRect_{};
            };

            void saveConsoleSnapshot();
            void configureConsoleInputMode();
            void configureConsoleFont();
            void configureConsoleWindowStyle();
            void centerConsoleWindow();
            void configureConsoleScreen();
            void restoreConsoleSnapshot() noexcept;

            ConsoleSnapshot snapshot_{};
        };
    } // namespace console
} // namespace platform
