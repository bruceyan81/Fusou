// Copyright (c) 2026 Bruce Yan. All Rights Reserved.

/**
 * @brief tile map の格納と参照操作を定義する
 */

#pragma once

#include "src/Core/Types/Types.h"
#include "src/Core/World/TileIdsGenerated.h"

#include <vector>

namespace core
{
    namespace world
    {
        class TileMap final
        {
        public:
            TileMap() = default;

            TileMap(int width, int height, TileId fill = 0);

            [[nodiscard]] bool isCellInBounds(types::Vec2 cell) const noexcept;

            [[nodiscard]] TileId getTileIdByCell(types::Vec2 cell) const noexcept;

            [[nodiscard]] int getWidth() const noexcept
            {
                return width_;
            };

            [[nodiscard]] int getHeight() const noexcept
            {
                return height_;
            };

            void setTileIdAtCell(types::Vec2 cell, TileId tileId) noexcept;

            void setTiles(std::vector<TileId> tiles);

        private:
            int width_ = 0;
            int height_ = 0;

            std::vector<TileId> tiles_;
        };
    } // namespace world
} // namespace core
