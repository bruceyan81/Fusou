// Copyright (c) 2026 Bruce Yan. All Rights Reserved.

/**
 * @brief console 描画用 cell buffer と color を定義する
 */

#pragma once

#include "src/Core/Types/Types.h"

#include <array>
#include <cstdint>
#include <string_view>

namespace core
{
    namespace render
    {
        /**
         * @note 1 つ Glyph を Unicode code point として扱う
         * 画面のズレを避けるため、1 Cell が 1 Glyph とする
         * UTF-8 の場合、 1 code point が 1〜4 byte である
         * UTF-16 の場合、 surrogate pair が存在しているので、固定ではない
         */
        using Glyph = char32_t;

        inline constexpr Glyph kSpace = U'\u3000';

        enum class Color : std::uint8_t
        {
            Black = 0x0,
            Blue = 0x1,
            Green = 0x2,
            Cyan = 0x3,
            Red = 0x4,
            Magenta = 0x5,
            Brown = 0x6,
            LightGray = 0x7,
            DarkGray = 0x8,
            LightBlue = 0x9,
            LightGreen = 0xA,
            LightCyan = 0xB,
            LightRed = 0xC,
            LightMagenta = 0xD,
            Yellow = 0xE,
            White = 0xF,
        };

        struct TextStyle final
        {
            Color fg_{Color::White};
            Color bg_{Color::Black};
        };

        struct Cell final
        {
            Glyph     glyph_{kSpace};
            TextStyle style_{};

            [[nodiscard]] constexpr bool operator==(const Cell& other) const noexcept
            {
                return glyph_ == other.glyph_ && style_.fg_ == other.style_.fg_ && style_.bg_ == other.style_.bg_;
            }
        };

        class CellBuffer final
        {
        public:
            static constexpr int kWidth = core::types::kLogicalViewportWidth;
            static constexpr int kHeight = core::types::kLogicalViewportHeight;

            [[nodiscard]] static constexpr bool isInBounds(const core::types::Vec2 bufferCell) noexcept
            {
                return (bufferCell.x_ >= 0) && (bufferCell.x_ < kWidth) && (bufferCell.y_ >= 0)
                    && (bufferCell.y_ < kHeight);
            }

            void fill(const Cell cell) noexcept
            {
                cells_.fill(cell);
            }

            [[nodiscard]] Cell& at(const core::types::Vec2 bufferCell) noexcept
            {
                return cells_[toIndex(bufferCell.x_, bufferCell.y_)];
            }

            [[nodiscard]] const Cell& at(const core::types::Vec2 bufferCell) const noexcept
            {
                return cells_[toIndex(bufferCell.x_, bufferCell.y_)];
            }

            void drawText(const core::types::Vec2 bufferCell, const std::u32string_view text) noexcept
            {
                if ((bufferCell.y_ < 0) || (bufferCell.y_ >= kHeight))
                {
                    return;
                }

                int curX = bufferCell.x_;
                for (const Glyph ch : text)
                {
                    if (curX >= kWidth)
                    {
                        break;
                    }

                    if (curX >= 0)
                    {
                        Cell& cell = at(core::types::Vec2{curX, bufferCell.y_});
                        cell.glyph_ = ch;
                    }
                    ++curX;
                }
            }

        private:
            std::array<Cell, static_cast<std::size_t>(kWidth * kHeight)> cells_{};

            [[nodiscard]] static std::size_t toIndex(const int x, const int y) noexcept
            {
                return static_cast<std::size_t>(y * kWidth + x);
            }
        };
    } // namespace render
} // namespace core
