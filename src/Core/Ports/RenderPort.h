// Copyright (c) 2026 Bruce Yan. All Rights Reserved.

/**
 * @brief frame buffer を表示する抽象 interface を定義する
 */

#pragma once

#include "src/Core/Render/CellBuffer.h"
#include "src/Core/Types/Types.h"

namespace core
{
    namespace ports
    {
        /**
         * @brief 描画の抽象化ポート
         */
        class RenderPort
        {
        public:
            virtual ~RenderPort() noexcept = default;

            virtual void clear() = 0;

            /**
             * @brief カーソルの表示状態を切り替える
             * プログラムの終了時、カーソルの復元には ConsoleSession.cpp に実装する
             * @param bVisible true の場合は表示 false の場合は非表示
             */
            virtual void setCursorVisible(bool bVisible) = 0;

            virtual void present(const render::CellBuffer& frame) = 0;

            virtual void flush() = 0;
        };
    } // namespace ports
} // namespace core
