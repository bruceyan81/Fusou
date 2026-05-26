// Copyright (c) 2026 Bruce Yan. All Rights Reserved.

/**
 * @brief Win32 keyboard 入力を core input port へ適配する class を定義する
 */

#pragma once

#include "src/Core/Ports/InputPort.h"
#include "src/Platform/Win32Console/Win32KeyboardState.h"

#include <cstdint>

namespace platform
{
    namespace win32console
    {
        enum class StepDirection
        {
            None,
            Left,
            Right,
            Up,
            Down,
        };

        class Win32ConsoleInputAdapter final : public core::ports::InputPort
        {
        public:
            [[nodiscard]] core::ports::InputResult sample(core::ports::InputContext inputContext) override;

        private:
            static constexpr int kEscCode = 0x1B;

            static constexpr int kEnterCode = 0x0D;

            static constexpr int kLeftCode = 0x25;
            static constexpr int kUpCode = 0x26;
            static constexpr int kRightCode = 0x27;
            static constexpr int kDownCode = 0x28;

            static constexpr int kACode = 0x41;
            static constexpr int kDCode = 0x44;
            static constexpr int kECode = 0x45;
            static constexpr int kFCode = 0x46;
            static constexpr int kSCode = 0x53;
            static constexpr int kWCode = 0x57;

            static constexpr int kICode = 0x49;
            static constexpr int kPCode = 0x50;
            static constexpr int kRCode = 0x52;

            // デバウンス のため、前回の押した結果を記録する
            bool bPrevEnterPressed_ = false;
            bool bPrevEPressed_ = false;
            bool bPrevFPressed_ = false;

            bool bPrevIPressed_ = false;
            bool bPrevRPressed_ = false;
            bool bPrevPPressed_ = false;

            bool bPrevWPressed_ = false;
            bool bPrevAPressed_ = false;
            bool bPrevSPressed_ = false;
            bool bPrevDPressed_ = false;

            bool bStepLeftPressed_ = false;
            bool bStepRightPressed_ = false;
            bool bStepUpPressed_ = false;
            bool bStepDownPressed_ = false;

            StepDirection stepPrimaryDir_ = StepDirection::None;

            Win32KeyboardState keyboardState_{};

            std::uint64_t stepPressSequenceCounter_ = 0;
        };
    } // namespace win32console
} // namespace platform
