// Copyright (c) 2026 Bruce Yan. All Rights Reserved.

/**
 * @brief Win32 console API を使った cursor color buffer 操作を実装する
 */

#include "src/Platform/Win32Console/Win32ConsoleApi.h"

// https://github.com/nemtrif/utfcpp
#include "utf8.h"

#include <algorithm>
#include <cstdio>
#include <iterator>
#include <stdexcept>
#include <string>
#include <vector>
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

        std::uint8_t createTextAttr(core::render::Color fg, core::render::Color bg) noexcept
        {
            return static_cast<std::uint8_t>((static_cast<int>(fg) & 0x0F) | ((static_cast<int>(bg) & 0x0F) << 4));
        }

        /**
         * @brief writePos から右方向へ並ぶ Glyph 列を console buffer の絶対座標へ書き込む
         * @param writePos 書き込みを開始する論理ポジション
         * @param text writePos から右方向へ並ぶ Glyph 列
         * @param attr Win32 console の前景色と背景色の 8 bit 属性値
         */
        void writeAt(core::types::Vec2 writePos, std::u32string_view text, std::uint8_t attr)
        {
            if (text.empty())
            {
                return;
            }

            if (
                writePos.x_ < 0
                ||
                writePos.x_ >= core::types::kLogicalViewportWidth
                ||
                writePos.y_ < 0
                ||
                writePos.y_ >= core::types::kLogicalViewportHeight
                )
            {
                return;
            }

            const int writableCellCount = core::types::kLogicalViewportWidth - writePos.x_;

            const int writeCellCount = std::min(static_cast<int>(text.size()), writableCellCount);

            const COORD consolePos{
                static_cast<SHORT>(writePos.x_ * 2),
                static_cast<SHORT>(writePos.y_),
            };

            std::wstring buffer;
            buffer.reserve(static_cast<std::size_t>(writeCellCount));

            for (int i = 0; i < writeCellCount; ++i)
            {
                const char32_t ch = text[static_cast<std::size_t>(i)];
                utf8::append16(static_cast<utf8::utfchar32_t>(ch), std::back_inserter(buffer));
            }

            const HANDLE handle = getStdoutHandle();
            DWORD charactersNum = 0;

            // cursor を進めずに指定座標へ文字を書き込む
            const BOOL characterRes = WriteConsoleOutputCharacterW(
                handle,
                buffer.data(),
                static_cast<DWORD>(buffer.size()),
                consolePos,
                &charactersNum
            );

            std::vector<WORD> attrs(static_cast<std::size_t>(writeCellCount * 2), static_cast<WORD>(attr));

            const BOOL attrRes = WriteConsoleOutputAttribute(
                handle,
                attrs.data(),
                static_cast<DWORD>(attrs.size()),
                consolePos,
                &charactersNum
            );

            if (characterRes == FALSE || attrRes == FALSE)
            {
                throw std::runtime_error("WriteConsoleOutputCharacterW or WriteConsoleOutputAttribute failed.");
            }
        }

        void flushStdout() noexcept
        {
            (void)std::fflush(stdout);
        }
    } // namespace win32console
} // namespace platform
