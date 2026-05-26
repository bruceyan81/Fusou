// Copyright (c) 2026 Bruce Yan. All Rights Reserved.

/**
 * @brief Win32 keyboard の現在押下と押下瞬間を取得する
 */

#include "src/Platform/Win32Console/Win32KeyboardState.h"

#include <vector>
#include <windows.h>

namespace platform
{
    namespace win32console
    {
        bool Win32KeyboardState::isKeyPressed(int virtualKeyCode) noexcept
        {
            pumpInputEvents();

            if (virtualKeyCode < 0 || virtualKeyCode > kMaxVirtualKeyCode)
            {
                return false;
            }

            const auto keyCode = static_cast<std::uint32_t>(virtualKeyCode);
            const auto bucketIndex = static_cast<std::size_t>(keyCode >> kVirtualKeyBucketShift);
            const auto bitMask = static_cast<std::uint32_t>(1U << (keyCode & kVirtualKeyBucketMask));

            return (keyMap_[bucketIndex] & bitMask) != 0;
        }

        void Win32KeyboardState::reset() noexcept
        {
            keyMap_.fill(0);
        }

        void Win32KeyboardState::pumpInputEvents() noexcept
        {
            const HANDLE inputHandle = GetStdHandle(STD_INPUT_HANDLE);
            if (inputHandle == INVALID_HANDLE_VALUE || inputHandle == nullptr)
            {
                return;
            }

            DWORD eventCount = 0;
            if (GetNumberOfConsoleInputEvents(inputHandle, &eventCount) == FALSE || eventCount == 0)
            {
                return;
            }

            std::vector<INPUT_RECORD> inputRecords(static_cast<std::size_t>(eventCount));

            DWORD readCount = 0;
            if (ReadConsoleInputW(inputHandle, inputRecords.data(), eventCount, &readCount) == FALSE)
            {
                return;
            }

            for (DWORD i = 0; i < readCount; ++i)
            {
                const INPUT_RECORD& inputRecord = inputRecords[static_cast<std::size_t>(i)];
                if (inputRecord.EventType != KEY_EVENT)
                {
                    continue;
                }

                const KEY_EVENT_RECORD& keyEvent = inputRecord.Event.KeyEvent;
                setKeyPressed(keyEvent.wVirtualKeyCode, keyEvent.bKeyDown != FALSE);

                const DWORD controlKeyState = keyEvent.dwControlKeyState;
                setKeyPressed(VK_MENU, (controlKeyState & (LEFT_ALT_PRESSED | RIGHT_ALT_PRESSED)) != 0);
                setKeyPressed(VK_CONTROL, (controlKeyState & (LEFT_CTRL_PRESSED | RIGHT_CTRL_PRESSED)) != 0);
                setKeyPressed(VK_SHIFT, (controlKeyState & SHIFT_PRESSED) != 0);
            }
        }

        void Win32KeyboardState::setKeyPressed(std::uint16_t virtualKeyCode, bool bPressed) noexcept
        {
            if (virtualKeyCode > kMaxVirtualKeyCode)
            {
                return;
            }

            const auto keyCode = static_cast<std::uint32_t>(virtualKeyCode);
            const auto bucketIndex = static_cast<std::size_t>(keyCode >> kVirtualKeyBucketShift);
            const auto bitMask = static_cast<std::uint32_t>(1U << (keyCode & kVirtualKeyBucketMask));

            if (bPressed)
            {
                keyMap_[bucketIndex] |= bitMask;
                return;
            }

            keyMap_[bucketIndex] &= ~bitMask;
        }
    } // namespace win32console
} // namespace platform
