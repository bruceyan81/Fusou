// Copyright (c) 2026 Bruce Yan. All Rights Reserved.

/**
 * @brief tile ID と文字 glyph の対応を定義する
 */

#pragma once

#include "src/Core/Render/CellBuffer.h"
#include "src/Core/Presentation/TilePaletteGenerated.h"
#include "src/Core/World/TileMap.h"

#include <cstddef>

namespace core
{
    namespace presentation
    {
        class TilePalette final
        {
        public:
            [[nodiscard]] static const core::render::Cell& findCellById(const core::world::TileId tileId) noexcept
            {
                if (tileId > kMaxTileId)
                {
                    return kPalette[0];
                }

                return kPalette[static_cast<std::size_t>(tileId)];
            }

        private:
            static constexpr core::world::TileId kMaxTileId = core::world::kGeneratedMaxTileId;

            // 早めなアクセスのため、map ではなく、連続メモリの array を使う
            using CellPaletteArray = GeneratedCellPaletteArray;

            inline static const CellPaletteArray kPalette = makeGeneratedTilePalette();
        };
    } // namespace presentation
} // namespace core
