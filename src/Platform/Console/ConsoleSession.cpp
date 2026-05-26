// Copyright (c) 2026 Bruce Yan. All Rights Reserved.

/**
 * @brief console mode font window buffer の設定と復元を管理する
 */

#include "src/Platform/Console/ConsoleSession.h"

#include "src/Platform/Win32Console/Win32ConsoleApi.h"

#include <cwchar>
#include <stdexcept>
#include <windows.h>

namespace platform
{
    namespace console
    {
        namespace
        {
            constexpr SHORT kConsoleWidth = 80;
            constexpr SHORT kConsoleHeight = 25;
            constexpr SHORT kConsoleFontWidth = 18;
            constexpr SHORT kConsoleFontHeight = 36;

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
                bool                       bHasInputMode_{false};
                bool                       bHasOutputMode_{false};
                bool                       bHasCursorInfo_{false};
                bool                       bHasScreenBufferInfo_{false};
                bool                       bHasFontInfo_{false};
                bool                       bHasWindowStyle_{false};
            };

            ConsoleSnapshot gConsoleSnapshot{};

            [[nodiscard]] HANDLE getConsoleHandle(DWORD stdHandle)
            {
                const HANDLE handle = GetStdHandle(stdHandle);

                if (handle == INVALID_HANDLE_VALUE || handle == nullptr)
                {
                    throw std::runtime_error("GetStdHandle failed.");
                }

                return handle;
            }

            void saveConsoleSnapshot()
            {
                gConsoleSnapshot = ConsoleSnapshot{};

                gConsoleSnapshot.inputHandle_ = getConsoleHandle(STD_INPUT_HANDLE);
                gConsoleSnapshot.outputHandle_ = getConsoleHandle(STD_OUTPUT_HANDLE);

                gConsoleSnapshot.bHasInputMode_ =
                    GetConsoleMode(gConsoleSnapshot.inputHandle_, &gConsoleSnapshot.inputMode_) != FALSE;

                gConsoleSnapshot.bHasOutputMode_ =
                    GetConsoleMode(gConsoleSnapshot.outputHandle_, &gConsoleSnapshot.outputMode_) != FALSE;

                gConsoleSnapshot.bHasCursorInfo_ =
                    GetConsoleCursorInfo(gConsoleSnapshot.outputHandle_, &gConsoleSnapshot.cursorInfo_) != FALSE;

                gConsoleSnapshot.bHasScreenBufferInfo_ = GetConsoleScreenBufferInfo(
                    gConsoleSnapshot.outputHandle_, &gConsoleSnapshot.screenBufferInfo_)
                    != FALSE;

                gConsoleSnapshot.fontInfo_.cbSize = sizeof(CONSOLE_FONT_INFOEX);
                gConsoleSnapshot.bHasFontInfo_ =
                    GetCurrentConsoleFontEx(gConsoleSnapshot.outputHandle_, FALSE, &gConsoleSnapshot.fontInfo_)
                    != FALSE;

                gConsoleSnapshot.windowHandle_ = GetConsoleWindow();
                if (gConsoleSnapshot.windowHandle_ != nullptr)
                {
                    gConsoleSnapshot.windowStyle_ = GetWindowLongPtrW(gConsoleSnapshot.windowHandle_, GWL_STYLE);
                    gConsoleSnapshot.bHasWindowStyle_ = true;
                }
            }

            void configureConsoleInputMode()
            {
                const HANDLE inputHandle = gConsoleSnapshot.inputHandle_;
                if (inputHandle == nullptr || inputHandle == INVALID_HANDLE_VALUE)
                {
                    return;
                }

                DWORD inputMode = 0;
                if (GetConsoleMode(inputHandle, &inputMode) == FALSE)
                {
                    return;
                }

                const DWORD configuredInputMode =
                    (inputMode | ENABLE_EXTENDED_FLAGS) & ~ENABLE_QUICK_EDIT_MODE & ~ENABLE_MOUSE_INPUT;

                (void)SetConsoleMode(inputHandle, configuredInputMode);
            }

            void configureConsoleFont()
            {
                CONSOLE_FONT_INFOEX fontInfo{};
                fontInfo.cbSize = sizeof(CONSOLE_FONT_INFOEX);

                if (GetCurrentConsoleFontEx(gConsoleSnapshot.outputHandle_, FALSE, &fontInfo) == FALSE)
                {
                    throw std::runtime_error("GetCurrentConsoleFontEx failed.");
                }

                fontInfo.dwFontSize.X = kConsoleFontWidth;
                fontInfo.dwFontSize.Y = kConsoleFontHeight;
                fontInfo.FontFamily = FF_DONTCARE;
                fontInfo.FontWeight = FW_NORMAL;
                (void)wcscpy_s(fontInfo.FaceName, L"Consolas");

                if (SetCurrentConsoleFontEx(gConsoleSnapshot.outputHandle_, FALSE, &fontInfo) == FALSE)
                {
                    throw std::runtime_error("SetCurrentConsoleFontEx failed.");
                }
            }

            void configureConsoleWindowStyle()
            {
                const HWND windowHandle = gConsoleSnapshot.windowHandle_;
                if (windowHandle == nullptr)
                {
                    return;
                }

                const LONG_PTR style = GetWindowLongPtrW(windowHandle, GWL_STYLE);
                const LONG_PTR fixedStyle = style & ~static_cast<LONG_PTR>(WS_MAXIMIZEBOX | WS_SIZEBOX);

                (void)SetWindowLongPtrW(windowHandle, GWL_STYLE, fixedStyle);

                HMENU systemMenu = GetSystemMenu(windowHandle, FALSE);
                if (systemMenu != nullptr)
                {
                    (void)DeleteMenu(systemMenu, SC_MAXIMIZE, MF_BYCOMMAND);
                    (void)DeleteMenu(systemMenu, SC_SIZE, MF_BYCOMMAND);
                }

                (void)SetWindowPos(windowHandle, nullptr, 0, 0, 0, 0,
                    SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
            }

            void centerConsoleWindow()
            {
                const HWND windowHandle = gConsoleSnapshot.windowHandle_;
                if (windowHandle == nullptr)
                {
                    return;
                }

                RECT windowRect{};
                if (GetWindowRect(windowHandle, &windowRect) == FALSE)
                {
                    return;
                }

                const HMONITOR monitorHandle = MonitorFromWindow(windowHandle, MONITOR_DEFAULTTONEAREST);
                if (monitorHandle == nullptr)
                {
                    return;
                }

                MONITORINFO monitorInfo{};
                monitorInfo.cbSize = sizeof(MONITORINFO);
                if (GetMonitorInfoW(monitorHandle, &monitorInfo) == FALSE)
                {
                    return;
                }

                const int windowWidth = windowRect.right - windowRect.left;
                const int windowHeight = windowRect.bottom - windowRect.top;
                const int workAreaWidth = monitorInfo.rcWork.right - monitorInfo.rcWork.left;
                const int workAreaHeight = monitorInfo.rcWork.bottom - monitorInfo.rcWork.top;

                const int windowX = monitorInfo.rcWork.left + (workAreaWidth - windowWidth) / 2;
                const int windowY = monitorInfo.rcWork.top + (workAreaHeight - windowHeight) / 2;

                (void)SetWindowPos(windowHandle, nullptr, windowX, windowY, 0, 0,
                    SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
            }

            void configureConsoleScreen()
            {
                const HANDLE outputHandle = gConsoleSnapshot.outputHandle_;

                const SMALL_RECT minimalWindow{0, 0, 1, 1};
                (void)SetConsoleWindowInfo(outputHandle, TRUE, &minimalWindow);

                const COORD bufferSize{kConsoleWidth, kConsoleHeight};
                if (SetConsoleScreenBufferSize(outputHandle, bufferSize) == FALSE)
                {
                    throw std::runtime_error("SetConsoleScreenBufferSize failed.");
                }

                configureConsoleFont();

                const SMALL_RECT windowRect{0, 0, kConsoleWidth - 1, kConsoleHeight - 1};
                if (SetConsoleWindowInfo(outputHandle, TRUE, &windowRect) == FALSE)
                {
                    throw std::runtime_error("SetConsoleWindowInfo failed.");
                }

                configureConsoleWindowStyle();
                centerConsoleWindow();
            }

            void restoreConsoleSnapshot() noexcept
            {
                const auto& snapshot = gConsoleSnapshot;

                if (snapshot.outputHandle_ != nullptr && snapshot.outputHandle_ != INVALID_HANDLE_VALUE)
                {
                    if (snapshot.bHasCursorInfo_)
                    {
                        (void)SetConsoleCursorInfo(snapshot.outputHandle_, &snapshot.cursorInfo_);
                    }

                    if (snapshot.bHasFontInfo_)
                    {
                        CONSOLE_FONT_INFOEX fontInfo = snapshot.fontInfo_;
                        (void)SetCurrentConsoleFontEx(snapshot.outputHandle_, FALSE, &fontInfo);
                    }

                    if (snapshot.bHasScreenBufferInfo_)
                    {
                        const auto& info = snapshot.screenBufferInfo_;
                        const COORD restoreBufferSize{info.dwSize.X, info.dwSize.Y};
                        const SMALL_RECT restoreWindow = info.srWindow;
                        const SMALL_RECT minimalWindow{0, 0, 1, 1};

                        (void)SetConsoleWindowInfo(snapshot.outputHandle_, TRUE, &minimalWindow);
                        (void)SetConsoleScreenBufferSize(snapshot.outputHandle_, restoreBufferSize);
                        (void)SetConsoleWindowInfo(snapshot.outputHandle_, TRUE, &restoreWindow);
                        (void)SetConsoleTextAttribute(snapshot.outputHandle_, info.wAttributes);
                    }

                    if (snapshot.bHasOutputMode_)
                    {
                        (void)SetConsoleMode(snapshot.outputHandle_, snapshot.outputMode_);
                    }
                }

                if (snapshot.windowHandle_ != nullptr && snapshot.bHasWindowStyle_)
                {
                    (void)SetWindowLongPtrW(snapshot.windowHandle_, GWL_STYLE, snapshot.windowStyle_);
                    (void)GetSystemMenu(snapshot.windowHandle_, TRUE);
                    (void)SetWindowPos(snapshot.windowHandle_, nullptr, 0, 0, 0, 0,
                        SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
                }

                if (snapshot.inputHandle_ != nullptr && snapshot.inputHandle_ != INVALID_HANDLE_VALUE
                    && snapshot.bHasInputMode_)
                {
                    (void)SetConsoleMode(snapshot.inputHandle_, snapshot.inputMode_);
                }
            }
        }

        ConsoleSession::ConsoleSession()
        {
            saveConsoleSnapshot();

            configureConsoleInputMode();

            configureConsoleScreen();

            bIsStarted_ = true;

            platform::win32console::clearScreen();
        }

        ConsoleSession::~ConsoleSession() noexcept
        {
            if (!bIsStarted_)
            {
                return;
            }

            try
            {
                platform::win32console::setCursor(platform::win32console::CursorMode::Normal);

                restoreConsoleSnapshot();
            }
            catch (...)
            {
            }

            bIsStarted_ = false;
        }
    }
}
