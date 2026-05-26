// Copyright (c) 2026 Bruce Yan. All Rights Reserved.

/**
 * @brief light の ON OFF と移動を入力に応じて更新する
 */

#include "src/Core/Game/LightSystem.h"

#include "src/Core/World/Camera.h"

#include <algorithm>

namespace core
{
    namespace game
    {
        namespace
        {
            [[nodiscard]] bool isLightMoveAxisZero(const types::Vec2f& lightMoveAxis) noexcept
            {
                return (lightMoveAxis.x_ == 0.0f) && (lightMoveAxis.y_ == 0.0f);
            }

            [[nodiscard]] types::Vec2 getLightMoveDir(const types::Vec2f& lightMoveAxis) noexcept
            {
                const int moveX = (lightMoveAxis.x_ > 0.0f) ? 1 : ((lightMoveAxis.x_ < 0.0f) ? -1 : 0);
                const int moveY = (lightMoveAxis.y_ > 0.0f) ? 1 : ((lightMoveAxis.y_ < 0.0f) ? -1 : 0);

                return types::Vec2{moveX, moveY};
            }

            [[nodiscard]] types::Vec2 addVec2(const types::Vec2& lhs, const types::Vec2& rhs) noexcept
            {
                return types::Vec2{lhs.x_ + rhs.x_, lhs.y_ + rhs.y_};
            }

            [[nodiscard]] int clampToViewport(const int value, const int maxValue) noexcept
            {
                return std::clamp(value, 0, maxValue);
            }

            [[nodiscard]] types::Vec2 getInitialLightViewportCell() noexcept
            {
                return types::Vec2{(world::Camera::kViewportWidth / 2) - kLightHalfExtent,
                    (world::Camera::kViewportHeight / 2) - kLightSpawnCenterYOffset};
            }

            void tickLightToggle(LightState& lightState, const ports::InputResult& inputResult,
                const types::Vec2& cameraOriginWorld) noexcept
            {
                if (!inputResult.bLightToggled_)
                {
                    return;
                }

                if (lightState.bEnabled_)
                {
                    lightState.bEnabled_ = false;
                    lightState.resetRepeatMoveState();
                    return;
                }

                lightState.bEnabled_ = true;
                lightState.resetRepeatMoveState();

                const types::Vec2 lightViewportCell = getInitialLightViewportCell();

                lightState.lightViewportCell_ = lightViewportCell;
                lightState.lightWorldCell_ = addVec2(cameraOriginWorld, lightViewportCell);
            }

            void tickLightMove(LightState& lightState, const ports::InputResult& inputResult,
                const types::Vec2& cameraOriginWorld) noexcept
            {
                if (!lightState.bEnabled_)
                {
                    return;
                }

                const types::Vec2f lightMoveAxis = inputResult.lightMoveAxis_;
                if (isLightMoveAxisZero(lightMoveAxis))
                {
                    lightState.resetRepeatMoveState();
                    return;
                }

                const types::Vec2 dir = getLightMoveDir(lightMoveAxis);

                bool bShouldMoveOneStep = false;

                // 方向キーは押下直後に 1 cell 動かし 長押し中は初回遅延後に一定間隔で繰り返す
                if (!(dir == lightState.dir_))
                {
                    lightState.dir_ = dir;
                    lightState.bIsMoveHeldForRepeat_ = true;
                    lightState.bIsRepeatingMove_ = false;
                    lightState.repeatMoveTickAccumulator_ = 0;
                    bShouldMoveOneStep = true;
                }
                else if (!lightState.bIsMoveHeldForRepeat_)
                {
                    lightState.bIsMoveHeldForRepeat_ = true;
                    lightState.bIsRepeatingMove_ = false;
                    lightState.repeatMoveTickAccumulator_ = 0;
                    bShouldMoveOneStep = true;
                }
                else
                {
                    ++lightState.repeatMoveTickAccumulator_;

                    const int thresholdTicks =
                        lightState.bIsRepeatingMove_ ? kLightRepeatIntervalTicks : kLightRepeatDelayTicks;

                    if (lightState.repeatMoveTickAccumulator_ >= thresholdTicks)
                    {
                        lightState.repeatMoveTickAccumulator_ -= thresholdTicks;
                        lightState.bIsRepeatingMove_ = true;
                        bShouldMoveOneStep = true;
                    }
                }

                if (!bShouldMoveOneStep)
                {
                    return;
                }

                types::Vec2 nextViewportCell = addVec2(lightState.lightViewportCell_, dir);

                nextViewportCell.x_ = clampToViewport(nextViewportCell.x_, world::Camera::kViewportWidth - 1);
                nextViewportCell.y_ = clampToViewport(nextViewportCell.y_, world::Camera::kViewportHeight - 1);

                lightState.lightViewportCell_ = nextViewportCell;
                lightState.lightWorldCell_ = addVec2(cameraOriginWorld, nextViewportCell);
            }
        } // namespace

        void runLightToggle(
            LightState& lightState, const ports::InputResult& inputResult, const types::Vec2& cameraOriginWorld) noexcept
        {
            tickLightToggle(lightState, inputResult, cameraOriginWorld);
        }

        void runLightMove(
            LightState& lightState, const ports::InputResult& inputResult, const types::Vec2& cameraOriginWorld) noexcept
        {
            tickLightMove(lightState, inputResult, cameraOriginWorld);
        }
    } // namespace game
} // namespace core
