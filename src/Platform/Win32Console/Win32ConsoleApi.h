// Copyright (c) 2026 Bruce Yan. All Rights Reserved.

/**
 * @brief Win32 console API helper を公開する
 */

#pragma once

#include "src/Core/Render/CellBuffer.h"
#include "src/Core/Types/Types.h"

#include <cstdint>
#include <string_view>

namespace platform
{
    namespace win32console
    {
        enum class CursorMode
        {
            Hidden,
            Normal,
            Solid,
        };

        void clearScreen();

        void setCursor(CursorMode cursorMode);

        void moveCursorTo(core::types::Vec2 viewportCell) noexcept;

        void setTextAttr(std::uint8_t attr);

        [[nodiscard]] std::uint8_t createTextAttr(core::render::Color fg, core::render::Color bg) noexcept;

        void write(std::u32string_view text);

        void flushStdout() noexcept;
    } // namespace win32console
} // namespace platform
