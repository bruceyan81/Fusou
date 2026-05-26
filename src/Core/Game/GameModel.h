// Copyright (c) 2026 Bruce Yan. All Rights Reserved.

/**
 * @brief ゲーム全体の runtime state を集約する
 */

#pragma once

#include "src/Core/Game/Enemy.h"
#include "src/Core/Game/EnemyRuntimeState.h"
#include "src/Core/Game/EffectState.h"
#include "src/Core/Game/GameAssets.h"
#include "src/Core/Game/GameConstants.h"
#include "src/Core/Game/GameScene.h"
#include "src/Core/Game/GoalState.h"
#include "src/Core/Game/LightState.h"
#include "src/Core/Game/PlayerState.h"
#include "src/Core/Game/RenderState.h"
#include "src/Core/Game/SceneState.h"
#include "src/Core/Types/Types.h"
#include "src/Core/World/Camera.h"
#include "src/Core/World/TileMap.h"

#include <cstddef>

namespace core
{
    namespace game
    {
        struct GameState final
        {
            SceneRuntimeState scene_{};

            LightState light_{};

            std::size_t tickIndex_{0};

            GameAssets assets_{};

            GoalRuntimeState goal_{};

            EffectRuntimeState effect_{};

            PlayerState player_{};

            world::Camera mainCamera_{};

            EnemyRuntimeState enemyRuntime_{};
        };
    } // namespace game
} // namespace core
