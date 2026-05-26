// Copyright (c) 2026 Bruce Yan. All Rights Reserved.

/**
 * @brief ゲーム scene の種類と scene 入力更新 API を定義する
 */

#pragma once

#include "src/Core/Ports/InputPort.h"

namespace core
{
    namespace game
    {
        struct GameState;

        enum class GameScene
        {
            Intro,
            Cutscene,
            Game,
            FinalCutscene,
            Fin,
            Died,
        };
        
        [[nodiscard]] ports::InputContext getInputContextByScene(const GameScene scene) noexcept;

        void updateSceneByInput(GameState& gameState, const ports::InputResult& inputResult) noexcept;
    } // namespace game
} // namespace core
