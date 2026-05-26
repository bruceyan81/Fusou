// Copyright (c) 2026 Bruce Yan. All Rights Reserved.

/**
 * @brief core 全体で使う vector と時間型を定義する
 */

#pragma once

#include <chrono> // steady_clock, time_point, duration
#include <cmath>  // hypot

namespace core
{
    namespace types
    {
        using Clock = std::chrono::steady_clock;
        using TimePoint = Clock::time_point;
        using Duration = Clock::duration;

        // 全角 Only ため、40 x 25 のサイズを扱う
        inline constexpr int kLogicalViewportWidth = 40;
        inline constexpr int kLogicalViewportHeight = 25;

        /**
         * @brief 座標
         */
        struct Vec2f final
        {
            float x_{};
            float y_{};

            [[nodiscard]] float getHypotLength() const
            {
                return std::hypot(x_, y_);
            }

            [[nodiscard]] Vec2f normalize() const
            {
                const float len = getHypotLength();

                // 0 除算を避ける
                if (len <= 0.0f)
                {
                    return Vec2f{0.0f, 0.0f};
                }

                const float inv = 1.0f / len;
                return Vec2f{x_ * inv, y_ * inv};
            }

            constexpr Vec2f& operator+=(const Vec2f& rhs) noexcept
            {
                x_ += rhs.x_;
                y_ += rhs.y_;
                return *this;
            }

            constexpr Vec2f& operator*=(const float scalar) noexcept
            {
                x_ *= scalar;
                y_ *= scalar;
                return *this;
            }

            constexpr bool operator==(const Vec2f& other) const noexcept
            {
                return (x_ == other.x_) && (y_ == other.y_);
            }
        };

        /**
         * @brief cell の添え字ため、int を使う
         */
        struct Vec2 final
        {
            int x_{};
            int y_{};

            [[nodiscard]] constexpr Vec2f toVec2f() const noexcept
            {
                return Vec2f{static_cast<float>(x_), static_cast<float>(y_)};
            };

            constexpr bool operator==(const Vec2& rhs) const noexcept
            {
                return x_ == rhs.x_ && y_ == rhs.y_;
            }
        };

        [[nodiscard]] inline Vec2 vec2fFloorToVec2(const Vec2f& worldPos) noexcept
        {
            return Vec2{
                static_cast<int>(std::floor(worldPos.x_)),
                static_cast<int>(std::floor(worldPos.y_)),
            };
        }
    } // namespace types
} // namespace core