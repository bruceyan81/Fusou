// Copyright (c) 2026 Bruce Yan. All Rights Reserved.

/**
 * @brief Win32 keyboard の key state sampling を定義する
 */

#pragma once

#include <array>
#include <cstdint>

namespace platform
{
    namespace win32console
    {
        class Win32KeyboardState final
        {
        public:
            [[nodiscard]] bool isKeyPressed(int virtualKeyCode) noexcept;

            void reset() noexcept;

        private:
            static constexpr int kMaxVirtualKeyCode = 0xFF;
            static constexpr int kVirtualKeyBucketShift = 5;
            static constexpr int kVirtualKeyBucketMask = 31;
            static constexpr std::size_t kKeyMapBucketCount = 8;

            std::array<std::uint32_t, kKeyMapBucketCount> keyMap_{};

            void pumpInputEvents() noexcept;

            void setKeyPressed(std::uint16_t virtualKeyCode, bool bPressed) noexcept;
        };
    } // namespace win32console
} // namespace platform
