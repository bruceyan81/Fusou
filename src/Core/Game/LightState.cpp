// Copyright (c) 2026 Bruce Yan. All Rights Reserved.

/**
 * @brief light state の初期化と移動補助処理を実装する
 */

#include "src/Core/Game/LightState.h"

namespace core
{
    namespace game
    {
        bool LightState::isLit(types::Vec2 viewportCell) const noexcept
        {
            if (!bEnabled_)
            {
                return false;
            }

            if (viewportCell.x_ < 0 || viewportCell.x_ >= world::Camera::kViewportWidth || viewportCell.y_ < 0
                || viewportCell.y_ >= world::Camera::kViewportHeight)
            {
                return false;
            }

            const int left = lightViewportCell_.x_;
            const int top = lightViewportCell_.y_;

            const int right = left + kLightSize;
            const int bottom = top + kLightSize;

            return (viewportCell.x_ >= left) && (viewportCell.x_ < right) && (viewportCell.y_ >= top)
                && (viewportCell.y_ < bottom);
        }

        void LightState::resetRepeatMoveState(types::Vec2 newDir) noexcept
        {
            dir_ = newDir;
            repeatMoveTickAccumulator_ = 0;
            bIsRepeatingMove_ = false;
            bIsMoveHeldForRepeat_ = false;
        }
    } // namespace game
} // namespace core
