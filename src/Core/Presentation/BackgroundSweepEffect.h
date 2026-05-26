// Copyright (c) 2026 Bruce Yan. All Rights Reserved.

/**
 * @brief 背景 sweep 表現を cell buffer に適用する helper を定義する
 */

#pragma once

#include "src/Core/Types/Types.h"
#include "src/Core/World/TileMap.h"

namespace core
{
    namespace presentation
    {
        struct SweepBandConfig final
        {
            int   bandWidthCells_{2};
            float speedCellsPerSecond_{20.0f};
            float periodSeconds_{5.0f};
        };
    } // namespace presentation
} // namespace core
