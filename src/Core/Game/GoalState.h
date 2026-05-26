// Copyright (c) 2026 Bruce Yan. All Rights Reserved.

/**
 * @brief goal 点灯と goal wave 演出の runtime state を定義する
 */

#pragma once

#include "src/Core/Game/RenderState.h"
#include "src/Core/Types/Types.h"
#include "src/Core/World/TileMap.h"

#include <vector>

namespace core
{
    namespace game
    {
        /**
         * @brief 1 つの goal が反応する縦方向の範囲と操作位置を持つ
         */
        struct GoalActivationRegion final
        {
            int beginY_{};
            int endY_{};
            types::Vec2 goalWorldCell_{};
        };

        struct GoalWaveEffectState final
        {
            bool bActive_{false};

            types::Vec2 centerWorldCell_{};

            int beginY_{0};
            int endY_{0};

            float elapsedSeconds_{0.0f};

            float speedCellsPerSec_{0.0f};

            float durationSeconds_{1.5f};

            int ringThicknessCells_{0};
        };

        struct GoalWaveRule final
        {
            world::TileId tileId_{0};

            bool bHasFg_{false};
            bool bHasBg_{false};

            render::Color fg_{render::Color::White};
            render::Color bg_{render::Color::Black};
        };

        struct GoalRuntimeState final
        {
            std::vector<bool> goalLitFlags_{};

            std::vector<GoalActivationRegion> goalRegions_{};

            int currGoalRegionIndex_{};

            bool bIsNear_{};

            bool bIsClear_{false};

            bool bPendingFinalCutscene_{false};

            std::vector<PaintOverride> paintOverrides_{};

            GoalWaveEffectState wave_{};

            // 一時的に色を変更する
            std::vector<PaintOverride> wavePaintOverrides_{};

            std::vector<GoalWaveRule> waveRules_{};

            int lastLitIndex_{};
        };
    } // namespace game
} // namespace core
