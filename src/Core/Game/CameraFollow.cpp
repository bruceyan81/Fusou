// Copyright (c) 2026 Bruce Yan. All Rights Reserved.

/**
 * @brief camera の viewport origin をゲーム状態に合わせて更新する
 */

#include "src/Core/Game/CameraFollow.h"

namespace core
{
    namespace game
    {
        void runCameraFollow(world::Camera& camera, const PlayerState& playerState, LightState& lightState,
            const types::Vec2& worldSize) noexcept
        {
            camera.follow({static_cast<int>(playerState.feetWorldPos_.x_),
                              static_cast<int>(playerState.feetWorldPos_.y_)},
                worldSize);

            if (!lightState.bEnabled_)
            {
                return;
            }

            const types::Vec2 cameraOriginWorld{
                camera.getViewportOriginXAtWorld(), camera.getViewportOriginYAtWorld()};

            const types::Vec2 viewportCell{lightState.lightWorldCell_.x_ - cameraOriginWorld.x_,
                lightState.lightWorldCell_.y_ - cameraOriginWorld.y_};

            const bool bOutOfViewport = (viewportCell.x_ < 0) || (viewportCell.y_ < 0)
                || (viewportCell.x_ + kLightSize > world::Camera::kViewportWidth)
                || (viewportCell.y_ + kLightSize > world::Camera::kViewportHeight);

            if (bOutOfViewport)
            {
                lightState.bEnabled_ = false;
                return;
            }

            lightState.lightViewportCell_ = viewportCell;
        }
    } // namespace game
} // namespace core
