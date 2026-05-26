// Copyright (c) 2026 Bruce Yan. All Rights Reserved.

/**
 * @brief camera viewport origin の clamp と変換を実装する
 */

#include "src/Core/World/Camera.h"

#include <algorithm>

namespace core
{
    namespace world
    {
        void Camera::follow(types::Vec2 targetPos, types::Vec2 worldSize) noexcept
        {
            /**
             * @note Viewport 左上は Origin 座標とする、この座標が取り得る最大値を計算する
             * Viewport の右側端は originX + kViewportWidth である
             * よって、originX <= worldSize.x - kViewportWidth とする必要がある
             * ワールド座標は Viewport のサイズより小さい場合
             * {originX_, originY_} は {0, 0} のまま維持する
             */
            const int originMaxX = std::max<int>(0, worldSize.x_ - kViewportWidth);
            const int originMaxY = std::max<int>(0, worldSize.y_ - kViewportHeight);

            /**
             * @note Viewport の左上の座標を計算する
             * targetPos は中心点として、X は kViewportWidth の半分を引いた結果、Viewport の左辺境界である
             * Y は kViewportHeight の半分を引いた結果、Viewport の↑上境界である
             * {originX_, originY_} とは Viewport の左上のワールド座標である
             * kViewportWidth と kViewportHeight は奇数の場合、int の割り算における切り捨てが発生するため
             * 若干左側に偏ることがある
             */
            originX_ = std::clamp(targetPos.x_ - (kViewportWidth / 2), 0, originMaxX);
            originY_ = std::clamp(targetPos.y_ - (kViewportHeight / 2), 0, originMaxY);
        }

        void Camera::setViewportOriginAtWorld(types::Vec2 originPos, types::Vec2 worldSize) noexcept
        {
            const int originMaxX = std::max<int>(0, worldSize.x_ - kViewportWidth);
            const int originMaxY = std::max<int>(0, worldSize.y_ - kViewportHeight);

            originX_ = std::clamp(originPos.x_, 0, originMaxX);
            originY_ = std::clamp(originPos.y_, 0, originMaxY);
        }
    } // namespace world
} // namespace core