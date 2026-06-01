// Copyright (c) 2026 Bruce Yan. All Rights Reserved.

/**
 * @brief tile 描画色と paint override の state を定義する
 */

#pragma once

#include "src/Core/Game/GameConstants.h"
#include "src/Core/Render/CellBuffer.h"

#include <array>
#include <cstdint>
#include <cstddef>

namespace core
{
    namespace game
    {
        struct TileColorPalette final
        {
            static constexpr std::size_t kSize = static_cast<std::size_t>(kMaxTileId + 1);

            /**
             * @note JSON palette の前景色と背景色
             * render::Color と同じ Win32 console 16 色の範囲になるので、std::uint8_t で保持する
             */
            std::array<std::uint8_t, kSize> fg_{};
            std::array<std::uint8_t, kSize> bg_{};

            [[nodiscard]] std::uint8_t getFg(world::TileId tileId) const noexcept
            {
                return fg_[static_cast<std::size_t>(tileId)];
            }

            [[nodiscard]] std::uint8_t getBg(world::TileId tileId) const noexcept
            {
                return bg_[static_cast<std::size_t>(tileId)];
            }

            void setOverride(world::TileId tileId, std::uint8_t fg, std::uint8_t bg) noexcept
            {
                const std::size_t index = static_cast<std::size_t>(tileId);
                fg_[index] = fg;
                bg_[index] = bg;
            }
        };

        struct PaintOverride final
        {
            bool bEnabled_{false};

            bool bHasFg_{false};
            bool bHasBg_{false};

            render::Color fg_{render::Color::White};
            render::Color bg_{render::Color::Black};
        };
    } // namespace game
} // namespace core
