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
            constexpr SHORT kMinimumConsoleFontWidth = 4;

            [[nodiscard]] HANDLE getConsoleHandle(DWORD stdHandle)
            {
                const HANDLE handle = GetStdHandle(stdHandle);

                if (handle == INVALID_HANDLE_VALUE || handle == nullptr)
                {
                    throw std::runtime_error("GetStdHandle failed.");
                }

                return handle;
            }

        } // namespace

        void ConsoleSession::saveConsoleSnapshot()
        {
            snapshot_.inputHandle_ = getConsoleHandle(STD_INPUT_HANDLE);
            snapshot_.outputHandle_ = getConsoleHandle(STD_OUTPUT_HANDLE);

            if (GetConsoleMode(snapshot_.inputHandle_, &snapshot_.inputMode_) == FALSE)
            {
                throw std::runtime_error("GetConsoleMode input failed.");
            }

            if (GetConsoleMode(snapshot_.outputHandle_, &snapshot_.outputMode_) == FALSE)
            {
                throw std::runtime_error("GetConsoleMode output failed.");
            }

            if (GetConsoleCursorInfo(snapshot_.outputHandle_, &snapshot_.cursorInfo_) == FALSE)
            {
                throw std::runtime_error("GetConsoleCursorInfo failed.");
            }

            if (GetConsoleScreenBufferInfo(snapshot_.outputHandle_, &snapshot_.screenBufferInfo_) == FALSE)
            {
                throw std::runtime_error("GetConsoleScreenBufferInfo failed.");
            }

            snapshot_.fontInfo_.cbSize = sizeof(CONSOLE_FONT_INFOEX);
            if (GetCurrentConsoleFontEx(snapshot_.outputHandle_, FALSE, &snapshot_.fontInfo_) == FALSE)
            {
                throw std::runtime_error("GetCurrentConsoleFontEx failed.");
            }

            snapshot_.windowHandle_ = GetConsoleWindow();
            if (snapshot_.windowHandle_ != nullptr)
            {
                SetLastError(ERROR_SUCCESS);
                snapshot_.windowStyle_ = GetWindowLongPtrW(snapshot_.windowHandle_, GWL_STYLE);
                if (snapshot_.windowStyle_ == 0 && GetLastError() != ERROR_SUCCESS)
                {
                    throw std::runtime_error("GetWindowLongPtrW failed.");
                }

                if (GetWindowRect(snapshot_.windowHandle_, &snapshot_.windowRect_) == FALSE)
                {
                    throw std::runtime_error("GetWindowRect failed.");
                }
            }
        }

        void ConsoleSession::configureConsoleInputMode()
        {
            const DWORD configuredInputMode =
                (snapshot_.inputMode_ | ENABLE_EXTENDED_FLAGS) & ~ENABLE_QUICK_EDIT_MODE & ~ENABLE_MOUSE_INPUT;

            if (SetConsoleMode(snapshot_.inputHandle_, configuredInputMode) == FALSE)
            {
                throw std::runtime_error("SetConsoleMode input failed.");
            }
        }

        void ConsoleSession::configureConsoleFont()
        {
            CONSOLE_FONT_INFOEX fontInfo = snapshot_.fontInfo_;

            fontInfo.FontFamily = FF_DONTCARE;
            fontInfo.FontWeight = FW_NORMAL;
            (void)wcscpy_s(fontInfo.FaceName, L"Consolas");

            for (SHORT fontWidth = kConsoleFontWidth; fontWidth >= kMinimumConsoleFontWidth; --fontWidth)
            {
                fontInfo.dwFontSize.X = fontWidth;
                fontInfo.dwFontSize.Y = static_cast<SHORT>(fontWidth * kConsoleFontHeight / kConsoleFontWidth);

                if (SetCurrentConsoleFontEx(snapshot_.outputHandle_, FALSE, &fontInfo) == FALSE)
                {
                    continue;
                }

                const COORD largestWindowSize = GetLargestConsoleWindowSize(snapshot_.outputHandle_);
                if (largestWindowSize.X >= kConsoleWidth && largestWindowSize.Y >= kConsoleHeight)
                {
                    return;
                }
            }

            throw std::runtime_error("Unable to configure a console font that fits the game window.");
        }

        void ConsoleSession::configureConsoleWindowStyle()
        {
            const HWND windowHandle = snapshot_.windowHandle_;
            if (windowHandle == nullptr)
            {
                return;
            }

            const LONG_PTR style = snapshot_.windowStyle_;
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

        void ConsoleSession::centerConsoleWindow()
        {
            const HWND windowHandle = snapshot_.windowHandle_;
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

        void ConsoleSession::configureConsoleScreen()
        {
            const HANDLE outputHandle = snapshot_.outputHandle_;

            const SMALL_RECT minimalWindow{0, 0, 1, 1};
            if (SetConsoleWindowInfo(outputHandle, TRUE, &minimalWindow) == FALSE)
            {
                throw std::runtime_error("SetConsoleWindowInfo minimal window failed.");
            }

            configureConsoleFont();

            const COORD bufferSize{kConsoleWidth, kConsoleHeight};
            if (SetConsoleScreenBufferSize(outputHandle, bufferSize) == FALSE)
            {
                throw std::runtime_error("SetConsoleScreenBufferSize failed.");
            }

            const SMALL_RECT windowRect{0, 0, kConsoleWidth - 1, kConsoleHeight - 1};
            if (SetConsoleWindowInfo(outputHandle, TRUE, &windowRect) == FALSE)
            {
                throw std::runtime_error("SetConsoleWindowInfo failed.");
            }

            configureConsoleWindowStyle();
            centerConsoleWindow();
        }

        void ConsoleSession::restoreConsoleSnapshot() noexcept
        {
            (void)SetConsoleCursorInfo(snapshot_.outputHandle_, &snapshot_.cursorInfo_);

            const auto& info = snapshot_.screenBufferInfo_;
            const SMALL_RECT minimalWindow{0, 0, 1, 1};
            (void)SetConsoleWindowInfo(snapshot_.outputHandle_, TRUE, &minimalWindow);

            CONSOLE_FONT_INFOEX fontInfo = snapshot_.fontInfo_;
            (void)SetCurrentConsoleFontEx(snapshot_.outputHandle_, FALSE, &fontInfo);

            (void)SetConsoleScreenBufferSize(snapshot_.outputHandle_, info.dwSize);
            (void)SetConsoleWindowInfo(snapshot_.outputHandle_, TRUE, &info.srWindow);
            (void)SetConsoleCursorPosition(snapshot_.outputHandle_, info.dwCursorPosition);
            (void)SetConsoleTextAttribute(snapshot_.outputHandle_, info.wAttributes);
            (void)SetConsoleMode(snapshot_.outputHandle_, snapshot_.outputMode_);

            if (snapshot_.windowHandle_ != nullptr)
            {
                (void)SetWindowLongPtrW(snapshot_.windowHandle_, GWL_STYLE, snapshot_.windowStyle_);
                (void)GetSystemMenu(snapshot_.windowHandle_, TRUE);
                (void)SetWindowPos(snapshot_.windowHandle_, nullptr,
                    snapshot_.windowRect_.left, snapshot_.windowRect_.top, 0, 0,
                    SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
            }

            (void)SetConsoleMode(snapshot_.inputHandle_, snapshot_.inputMode_);
        }

        ConsoleSession::ConsoleSession()
        {
            saveConsoleSnapshot();

            try
            {
                configureConsoleInputMode();
                configureConsoleScreen();
                platform::win32console::clearScreen();
            }
            catch (...)
            {
                restoreConsoleSnapshot();
                throw;
            }
        }

        ConsoleSession::~ConsoleSession() noexcept
        {
            restoreConsoleSnapshot();
        }
    }
}
