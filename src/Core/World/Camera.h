// Copyright (c) 2026 Bruce Yan. All Rights Reserved.

/**
 * @brief world 座標から viewport を切り出す camera を定義する
 */

#pragma once

#include "src/Core/Types/Types.h"

namespace core
{
    namespace world
    {
        class Camera final
        {
        public:
            static constexpr int kViewportWidth = core::types::kLogicalViewportWidth;
            static constexpr int kViewportHeight = core::types::kLogicalViewportHeight;

            [[nodiscard]] int getViewportOriginXAtWorld() const noexcept
            {
                return originX_;
            }

            [[nodiscard]] int getViewportOriginYAtWorld() const noexcept
            {
                return originY_;
            }

            /**
             * @brief 追従カメラ
             * ターゲットの位置より、Viewport の原点、左上の座標を計算する
             * Viewport のサイズは kViewportWidth * kViewportHeight とする
             * @param targetPos 追従ターゲットのワールド座標、Viewport の中心部にある
             * @param worldSize Game Scene の TileMap の全体のサイズ
             */
            void follow(types::Vec2 targetPos, types::Vec2 worldSize) noexcept;

            void setViewportOriginAtWorld(types::Vec2 originPos, types::Vec2 worldSize) noexcept;

        private:
            int originX_ = 0;
            int originY_ = 0;
        };
    } // namespace world
} // namespace core