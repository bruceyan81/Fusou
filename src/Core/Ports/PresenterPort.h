// Copyright (c) 2026 Bruce Yan. All Rights Reserved.

/**
 * @brief GameState を frame buffer へ変換する抽象 interface を定義する
 */

#pragma once

#include "src/Core/Game/GameModel.h"
#include "src/Core/Render/CellBuffer.h"

namespace core
{
    namespace ports
    {
        /**
         * @brief Core からの描画内容を Render へ提出する抽象化ポート
         */
        class PresenterPort
        {
        public:
            virtual ~PresenterPort() noexcept = default;

            virtual void composeFrame(const game::GameState& gameState, render::CellBuffer& outFrame) = 0;
        };
    } // namespace ports
} // namespace core
