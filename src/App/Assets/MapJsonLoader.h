// Copyright (c) 2026 Bruce Yan. All Rights Reserved.

/**
 * @brief JSON map asset の読み込み API を公開する
 */

#pragma once

#include "src/Core/Game/RenderState.h"
#include "src/Core/World/TileMap.h"

#include <filesystem>

namespace app
{
    namespace assets
    {
        struct MapAsset final
        {
            core::world::TileMap         tileMap_{};
            core::game::TileColorPalette colorPalette_{};
        };

        [[nodiscard]] MapAsset loadMapAssetFromJson(const std::filesystem::path& filePath);
    } // namespace assets
} // namespace app
