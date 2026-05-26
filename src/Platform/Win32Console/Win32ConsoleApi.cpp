// Copyright (c) 2026 Bruce Yan. All Rights Reserved.

/**
 * @brief Win32 console API を使った cursor color buffer 操作を実装する
 */

#include "src/Platform/Win32Console/Win32ConsoleApi.h"

// https://github.com/nemtrif/utfcpp
#include "utf8.h"

#include <cstdio>
#include <iterator>
#include <stdexcept>
#include <string>
#include <windows.h>

namespace platform
{
    namespace win32console
    {
        namespace
        {
            [[nodiscard]] HANDLE getStdoutHandle()
            {
                const HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);

                if (handle == INVALID_HANDLE_VALUE || handle == nullptr)
                {
                    throw std::runtime_error("GetStdHandle STD_OUTPUT_HANDLE failed.");
                }

                return handle;
            }

            [[nodiscard]] DWORD getDefaultCursorSize() noexcept
            {
                static DWORD defaultSize = 0;

                if (defaultSize != 0)
                {
                    return defaultSize;
                }

                CONSOLE_CURSOR_INFO cursorInfo{};
                if (GetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo) != FALSE)
                {
                    defaultSize = cursorInfo.dwSize;
                    return defaultSize;
                }

                defaultSize = 25;
                return defaultSize;
            }
        } // namespace

        void clearScreen()
        {
            const HANDLE handle = getStdoutHandle();

            CONSOLE_SCREEN_BUFFER_INFO screenInfo{};
            if (GetConsoleScreenBufferInfo(handle, &screenInfo) == FALSE)
            {
                throw std::runtime_error("GetConsoleScreenBufferInfo failed.");
            }

            const COORD origin{0, 0};
            const DWORD cellCount =
                static_cast<DWORD>(screenInfo.dwSize.X) * static_cast<DWORD>(screenInfo.dwSize.Y);

            DWORD written = 0;

            if (FillConsoleOutputAttribute(handle, screenInfo.wAttributes, cellCount, origin, &written) == FALSE)
            {
                throw std::runtime_error("FillConsoleOutputAttribute failed.");
            }

            if (FillConsoleOutputCharacterW(handle, L' ', cellCount, origin, &written) == FALSE)
            {
                throw std::runtime_error("FillConsoleOutputCharacterW failed.");
            }

            if (SetConsoleCursorPosition(handle, origin) == FALSE)
            {
                throw std::runtime_error("SetConsoleCursorPosition failed.");
            }
        }

        void setCursor(CursorMode cursorMode)
        {
            CONSOLE_CURSOR_INFO cursorInfo{};

            cursorInfo.dwSize = getDefaultCursorSize();
            cursorInfo.bVisible = TRUE;

            switch (cursorMode)
            {
                case CursorMode::Hidden:
                    cursorInfo.dwSize = 100;
                    cursorInfo.bVisible = FALSE;
                    break;

                case CursorMode::Normal:
                    cursorInfo.dwSize = getDefaultCursorSize();
                    cursorInfo.bVisible = TRUE;
                    break;

                case CursorMode::Solid:
                    cursorInfo.dwSize = 100;
                    cursorInfo.bVisible = TRUE;
                    break;
            }

            if (SetConsoleCursorInfo(getStdoutHandle(), &cursorInfo) == FALSE)
            {
                throw std::runtime_error("SetConsoleCursorInfo failed.");
            }
        }

        void moveCursorTo(core::types::Vec2 viewportCell) noexcept
        {
            if (viewportCell.x_ < 0 || viewportCell.x_ >= core::types::kLogicalViewportWidth)
            {
                return;
            }

            if (viewportCell.y_ < 0 || viewportCell.y_ >= core::types::kLogicalViewportHeight)
            {
                return;
            }

            const COORD consolePos{
                static_cast<SHORT>(viewportCell.x_ * 2),
                static_cast<SHORT>(viewportCell.y_),
            };

            const HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
            if (handle == INVALID_HANDLE_VALUE || handle == nullptr)
            {
                return;
            }

            (void)SetConsoleCursorPosition(handle, consolePos);
        }

        void setTextAttr(std::uint8_t attr)
        {
            if (SetConsoleTextAttribute(getStdoutHandle(), static_cast<WORD>(attr)) == FALSE)
            {
                throw std::runtime_error("SetConsoleTextAttribute failed.");
            }
        }

        std::uint8_t createTextAttr(core::render::Color fg, core::render::Color bg) noexcept
        {
            return static_cast<std::uint8_t>((static_cast<int>(fg) & 0x0F) | ((static_cast<int>(bg) & 0x0F) << 4));
        }

        void write(std::u32string_view text)
        {
            if (text.empty())
            {
                return;
            }

            std::u16string buffer;
            buffer.reserve(text.size());

            for (const char32_t ch : text)
            {
                utf8::append16(static_cast<utf8::utfchar32_t>(ch), std::back_inserter(buffer));
            }

            DWORD charactersNum = 0;

            const BOOL result = WriteConsoleW(getStdoutHandle(), reinterpret_cast<const wchar_t*>(buffer.data()),
                static_cast<DWORD>(buffer.size()), &charactersNum, nullptr);

            if (result == FALSE)
            {
                throw std::runtime_error("WriteConsoleW failed.");
            }
        }

        void flushStdout() noexcept
        {
            (void)std::fflush(stdout);
        }
    } // namespace win32console
} // namespace platform
