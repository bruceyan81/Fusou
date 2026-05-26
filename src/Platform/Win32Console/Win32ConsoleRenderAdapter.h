// Copyright (c) 2026 Bruce Yan. All Rights Reserved.

/**
 * @brief Win32 console renderer を core render port へ適配する class を定義する
 */

#pragma once

#include "src/Core/Ports/RenderPort.h"
#include "src/Core/Render/CellBuffer.h"

#include <cstdint>

namespace platform
{
    namespace win32console
    {
        class Win32ConsoleRenderAdapter final : public core::ports::RenderPort
        {
        public:
            void clear() override;

            void flush() override;

            void setCursorVisible(bool bVisible) override;

            /**
             * @brief 差分更新
             * @param frame 1 フレームのバッファ
             */
            void present(const core::render::CellBuffer& frame) override;

        private:
            core::render::CellBuffer preFrame_{};

            bool bHasPrevious_{false};

            std::uint8_t cacheAttr_{};
        };
    } // namespace win32console
} // namespace platform
