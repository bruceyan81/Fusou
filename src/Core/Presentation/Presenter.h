// Copyright (c) 2026 Bruce Yan. All Rights Reserved.

/**
 * @brief GameState presentation の core 実装 class を定義する
 */

#pragma once

#include "src/Core/Ports/PresenterPort.h"

namespace core
{
    namespace presentation
    {
        class CellBufferPresenter final : public ports::PresenterPort
        {
        public:
            void composeFrame(const game::GameState& gameState, render::CellBuffer& outFrame) override;
        };
    } // namespace presentation
} // namespace core
