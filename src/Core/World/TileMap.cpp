// Copyright (c) 2026 Bruce Yan. All Rights Reserved.

/**
 * @brief tile map の cell 操作と領域検証を実装する
 */

#include "src/Core/World/TileMap.h"

#include <limits>
#include <stdexcept>
#include <utility>

namespace core
{
    namespace world
    {
        std::size_t TileMap::calculateTileCount(int width, int height)
        {
            if (width <= 0 || height <= 0)
            {
                throw std::runtime_error("The size of TileMap must be positive.");
            }

            constexpr auto kMaxSize = std::numeric_limits<std::size_t>::max();

            const auto widthSize = static_cast<std::size_t>(width);
            const auto heightSize = static_cast<std::size_t>(height);

            if (widthSize > (kMaxSize / heightSize))
            {
                throw std::runtime_error("Tile count overflow.");
            }

            return widthSize * heightSize;
        }

        TileMap::TileMap(TileMap&& other) noexcept
            : width_(other.width_), height_(other.height_), tiles_(std::move(other.tiles_))
        {
            other.width_ = 0;
            other.height_ = 0;
            other.tiles_.clear();
        }

        TileMap& TileMap::operator=(const TileMap& other)
        {
            if (this == &other)
            {
                return *this;
            }

            TileMap copiedMap{other};
            *this = std::move(copiedMap);
            return *this;
        }

        TileMap& TileMap::operator=(TileMap&& other) noexcept
        {
            if (this == &other)
            {
                return *this;
            }

            width_ = other.width_;
            height_ = other.height_;
            tiles_ = std::move(other.tiles_);

            other.width_ = 0;
            other.height_ = 0;
            other.tiles_.clear();

            return *this;
        }

        TileMap::TileMap(int width, int height, TileId fill)
        {
            const std::size_t tileCount = calculateTileCount(width, height);

            width_ = width;
            height_ = height;
            tiles_.assign(tileCount, fill);
        }

        TileMap::TileMap(int width, int height, std::vector<TileId> tiles)
        {
            const std::size_t tileCount = calculateTileCount(width, height);

            if (tiles.size() != tileCount)
            {
                throw std::runtime_error("Tiles size mismatch.");
            }

            width_ = width;
            height_ = height;
            tiles_ = std::move(tiles);
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

    } // namespace world
} // namespace core
