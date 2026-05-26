// Copyright (c) 2026 Bruce Yan. All Rights Reserved.

/**
 * @brief JSON map asset を TileMap と palette へ変換する
 */

#include "src/App/Assets/MapJsonLoader.h"

// https://github.com/nlohmann/json/tree/develop/single_include/nlohmann
#include <json.hpp>

#include <fstream>
#include <limits> // std::numeric_limits
#include <stdexcept>
#include <string>
#include <vector>

namespace app
{
    namespace assets
    {
        constexpr int kVer = 2;

        constexpr char kVersion[] = "version";
        constexpr char kWidth[] = "width";
        constexpr char kHeight[] = "height";
        constexpr char kTiles[] = "tiles";

        constexpr char kPalette[] = "palette";
        constexpr char kTileId[] = "tileId";
        constexpr char kFg[] = "fg";
        constexpr char kBg[] = "bg";

        MapAsset loadMapAssetFromJson(const std::filesystem::path& filePath)
        {
            std::ifstream in(filePath, std::ios::binary);

            if (!in.is_open())
            {
                throw std::runtime_error("Failed to open file. Path: " + filePath.string());
            }

            using Json = nlohmann::json;

            Json json;

            try
            {
                // https://json.nlohmann.me/api/operator_gtgt/
                in >> json;
            }
            catch (const std::exception& e)
            {
                throw std::runtime_error(std::string("Error: ") + e.what());
            }

            auto requireInt = [](const Json& json, const auto key) {
                // https://json.nlohmann.me/api/basic_json/at/#exceptions
                const Json& val = json.at(key);
                // https://json.nlohmann.me/api/basic_json/is_number_integer/
                if (!val.is_number_integer())
                {
                    throw std::runtime_error(std::string(key) + " must be an integer.");
                }
                return val.get<int>();
            };

            const int version = requireInt(json, kVersion);
            const int width = requireInt(json, kWidth);
            const int height = requireInt(json, kHeight);

            if (version != kVer)
            {
                throw std::runtime_error("Unsupported version.");
            }

            if (width <= 0 || height <= 0)
            {
                throw std::runtime_error("Width or height must be positive.");
            }

            auto requireArray = [](const auto& json, const auto key) {
                const Json& val = json.at(key);
                if (!val.is_array())
                {
                    throw std::runtime_error(std::string(key) + " must be an array.");
                }
                return val;
            };

            const auto& tilesJson = requireArray(json, kTiles);

            auto computeTileCount = [](const int width, const int height) {
                /**
                 * @note static constexpr unsigned long long(max)() noexcept
                 * size_t の最大値を取得する
                 */
                constexpr auto kMaxSize = std::numeric_limits<std::size_t>::max();

                // 直接に w * h > kMaxSize のように書くことはダメ、w * h はオーバーが発生すると UB になってしまう
                if (static_cast<std::size_t>(width) > (kMaxSize / static_cast<std::size_t>(height)))
                {
                    throw std::runtime_error("Tile count overflow.");
                }

                return static_cast<std::size_t>(width) * static_cast<std::size_t>(height);
            };

            const auto tileCount = computeTileCount(width, height);

            // JSON array size を取得する
            if (tilesJson.size() != tileCount)
            {
                throw std::runtime_error("Tiles size mismatch.");
            }

            core::world::TileMap map{width, height, static_cast<core::world::TileId>(0)};

            std::vector<core::world::TileId> tiles;
            tiles.reserve(tileCount);

            auto checkTileId = [](auto tileId) {
                const int maxTileId = static_cast<int>(core::game::kMaxTileId);
                if (tileId < 0)
                {
                    throw std::runtime_error("Tile id must be >= 0.");
                }

                if (tileId > maxTileId)
                {
                    throw std::runtime_error("Tile id must be <= " + std::to_string(maxTileId) + ".");
                }

                return static_cast<core::world::TileId>(tileId);
            };

            for (const auto& tileId : tilesJson)
            {
                const auto rawId = tileId.get<int>();
                tiles.push_back(checkTileId(rawId));
            }

            map.setTiles(std::move(tiles));

            // palette のイテレータを取得する
            core::game::TileColorPalette colorPalette{};

            const auto paletteIter = json.find(kPalette);

            if (paletteIter != json.end())
            {
                if (!paletteIter->is_array())
                {
                    throw std::runtime_error(std::string(kPalette) + " must be an array.");
                }

                for (const auto& object : *paletteIter)
                {
                    if (!object.is_object())
                    {
                        throw std::runtime_error("Palette object must be an object.");
                    }

                    const int rawTileId = requireInt(object, kTileId);
                    const int rawFg = requireInt(object, kFg);
                    const int rawBg = requireInt(object, kBg);

                    const int maxTileId = static_cast<int>(core::game::kMaxTileId);

                    if (rawTileId < 0 || rawTileId > maxTileId)
                    {
                        throw std::runtime_error("TileId out of range.");
                    }

                    if (rawFg < 0 || rawFg > 15 || rawBg < 0 || rawBg > 15)
                    {
                        throw std::runtime_error("Palette fg/bg out of range.");
                    }

                    const auto tileId = static_cast<core::world::TileId>(rawTileId);

                    colorPalette.setOverride(
                        tileId, static_cast<std::uint8_t>(rawFg), static_cast<std::uint8_t>(rawBg));
                }
            }

            MapAsset asset{};
            asset.tileMap_ = std::move(map);
            asset.colorPalette_ = std::move(colorPalette);
            return asset;
        }
    } // namespace assets
} // namespace app
