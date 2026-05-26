// Copyright (c) 2026 Bruce Yan. All Rights Reserved.

/**
 * @brief tile map の cell 操作と領域検証を実装する
 */

#include "src/Core/World/TileMap.h"

#include <limits>
#include <stdexcept>

namespace core
{
    namespace world
    {
        TileMap::TileMap(int width, int height, TileId fill) : width_(width), height_(height)
        {
            if (width_ <= 0 || height_ <= 0)
            {
                throw std::runtime_error("The size of TileMap must be positive.");
            }

            constexpr auto kMaxSize = std::numeric_limits<std::size_t>::max();

            const auto     w = static_cast<std::size_t>(width_);
            const auto     h = static_cast<std::size_t>(height_);

            if (w > (kMaxSize / h))
            {
                throw std::runtime_error("Tile count overflow.");
            }

            tiles_.assign(w * h, fill);
        }

        bool TileMap::isCellInBounds(types::Vec2 cell) const noexcept
        {
            return cell.x_ >= 0 && cell.x_ < width_ && cell.y_ >= 0 && cell.y_ < height_;
        }

        TileId TileMap::getTileIdByCell(types::Vec2 cell) const noexcept
        {
            if (!isCellInBounds(cell))
            {
                return 0;
            }

            const auto index = static_cast<std::size_t>(cell.y_ * width_ + cell.x_);
            return tiles_[index];
        }

        void TileMap::setTileIdAtCell(types::Vec2 cell, TileId tileId) noexcept
        {
            if (!isCellInBounds(cell))
            {
                return;
            }

            const auto index = static_cast<std::size_t>(cell.y_ * width_ + cell.x_);
            tiles_[index] = tileId;
        }

        void TileMap::setTiles(std::vector<TileId> tiles)
        {
            constexpr auto kMaxSize = std::numeric_limits<std::size_t>::max();

            if (width_ <= 0 || height_ <= 0)
            {
                throw std::runtime_error("The size of TileMap must be positive.");
            }

            const auto width  = static_cast<std::size_t>(width_);
            const auto height = static_cast<std::size_t>(height_);

            if (width > (kMaxSize / height))
            {
                throw std::runtime_error("Tile count overflow.");
            }

            tiles_ = std::move(tiles);
        }
    } // namespace world
} // namespace core
