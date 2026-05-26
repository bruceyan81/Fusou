// Copyright (c) 2026 Bruce Yan. All Rights Reserved.

/**
 * @brief ゲーム実行に必要な asset 一式を保持する構造を定義する
 */

#pragma once

#include "src/Core/Game/RenderState.h"
#include "src/Core/World/TileMap.h"

namespace core
{
    namespace game
    {
        struct TileMapAsset final
        {
            world::TileMap tileMap_{};
            TileColorPalette colorPalette_{};
        };

        struct GameAssets final
        {
            TileMapAsset introScene_{};
            TileMapAsset gameScene_{};
            TileMapAsset gameSceneBackground_{};
            TileMapAsset diedScene_{};
            TileMapAsset finScene_{};
            TileMapAsset player_{};
        };
    } // namespace game
} // namespace core
