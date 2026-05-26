// Copyright (c) 2026 Bruce Yan. All Rights Reserved.

/**
 * @brief ゲームロジックで共有する定数を定義する
 */

#pragma once

#include "src/Core/UI/UI.h"
#include "src/Core/World/TileMap.h"

namespace core
{
    namespace game
    {
        inline constexpr world::TileId kMaxTileId = world::kGeneratedMaxTileId;

        inline constexpr int kLightSize = 7;
        inline constexpr int kLightHalfExtent = kLightSize / 2;
        inline constexpr int kLightSpawnCenterYOffset = 4;

        inline constexpr float kGravity = 9.8f;
        inline constexpr float kHorizontalAcceleration = 30.0f;

        inline constexpr float kMaxHorizontalSpeed = 6.0f;
        inline constexpr float kMaxFallSpeed = 20.0f;

        inline constexpr float kLadderClimbSpeed = 5.0f;

        inline constexpr int kLightRepeatDelayTicks = 5;
        inline constexpr int kLightRepeatIntervalTicks = 4;

        inline constexpr int kEnemyPatrolHalfRange = 3;
        inline constexpr int kCharsPerSecond = 14;
        inline constexpr float kInputBlockSeconds = 2.0f;
        inline constexpr float kHoldSeconds = 1.0f;

        inline constexpr int kFinalCutsceneTicksPerCell = 4;
        inline constexpr float kFinalCutsceneHoldSeconds = 3.0f;

        inline consteval int getCutsceneTextTotalChars() noexcept
        {
            int totalChars = 0;
            for (const auto line : ui::str::kCutsceneText)
            {
                totalChars += static_cast<int>(line.size());
            }
            return totalChars;
        }
    } // namespace game
} // namespace core
