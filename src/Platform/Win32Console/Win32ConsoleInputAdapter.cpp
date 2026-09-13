// Copyright (c) 2026 Bruce Yan. All Rights Reserved.

/**
 * @brief Win32 keyboard state から scene 別の入力結果を作る
 */

#include "src/Platform/Win32Console/Win32ConsoleInputAdapter.h"

namespace platform
{
    namespace win32console
    {
        namespace
        {
            struct AllowedAction final
            {
                bool bQuit{false};
                bool bConfirm{false};
                bool bInteract{false};
                bool bToggleLight{false};
                bool bPlayerAxis{false};
                bool bLightAxis{false};

                // Died から Game へ
                bool bBackdoorR{false};

                // Game から Intro へ
                bool bBackdoorI{false};

                // 前回の checkpoint へ戻る
                bool bBackdoorP{false};

                // Cutscene のみ
                bool bAnyKey{false};
            };

            [[nodiscard]] AllowedAction getAllowedAction(const core::ports::InputContext inputContext) noexcept
            {
                switch (inputContext)
                {
                    case core::ports::InputContext::Game:
                        return AllowedAction{true, true, true, true, true, true, false, true, true, false};

                    case core::ports::InputContext::Intro:
                    case core::ports::InputContext::Fin:
                        return AllowedAction{true, true, false, false, false, false, false, false, false, false};

                    case core::ports::InputContext::Died:
                        return AllowedAction{true, true, false, false, false, false, true, false, false, false};

                    case core::ports::InputContext::Cutscene:
                        return AllowedAction{false, false, false, false, false, false, false, false, false, true};

                    default:
                        return AllowedAction{false, false, false, false, false, false, false, false, false, false};
                }
            }

            [[nodiscard]] bool sampleAnyKeyPressed(Win32KeyboardState& keyboardState) noexcept
            {
                /**
                 * @note 普通なキーボード、すべての場面にも対応することを保証しない
                 */
                for (int keyCode = 0x08; keyCode <= 0xA5; ++keyCode)
                {
                    if (keyboardState.isKeyPressed(keyCode))
                    {
                        return true;
                    }
                }
                return false;
            }
        } // namespace

        core::ports::InputResult Win32ConsoleInputAdapter::sample(core::ports::InputContext inputContext)
        {
            const auto allowedAction = getAllowedAction(inputContext);

            core::ports::InputResult inputResult{};

            {
                if (allowedAction.bAnyKey)
                {
                    inputResult.bAnyKeyPressed_ = sampleAnyKeyPressed(keyboardState_);
                    return inputResult;
                }

                /**
                 * @param positiveX D と →
                 * @param positiveY W と ↑
                 * @param negativeX A と ←
                 * @param negativeY S と ↓
                 */
                auto sampleAxis = [this](const int positiveX, const int positiveY, const int negativeX,
                                      const int negativeY) {
                    float x = 0.0f;
                    float y = 0.0f;

                    if (keyboardState_.isKeyPressed(positiveX))
                    {
                        x += 1.0f;
                    }

                    // 上に移動するのは引き算
                    if (keyboardState_.isKeyPressed(positiveY))
                    {
                        y -= 1.0f;
                    }

                    if (keyboardState_.isKeyPressed(negativeX))
                    {
                        x -= 1.0f;
                    }

                    if (keyboardState_.isKeyPressed(negativeY))
                    {
                        y += 1.0f;
                    }

                    return core::types::Vec2f{x, y}.normalize();
                };

                // ESC
                inputResult.bQuitRequested_ = allowedAction.bQuit ? keyboardState_.isKeyPressed(kEscCode) : false;

                {
                    inputResult.playerMoveAxis_ =
                        allowedAction.bPlayerAxis ? sampleAxis(kDCode, kWCode, kACode, kSCode) : core::types::Vec2f{};
                }

                {
                    inputResult.lightMoveAxis_ = allowedAction.bLightAxis
                        ? sampleAxis(kRightCode, kUpCode, kLeftCode, kDownCode)
                        : core::types::Vec2f{};
                }

                // E と Enter デバウンス
                {
                    const bool bCurrEnterPressed = keyboardState_.isKeyPressed(kEnterCode);
                    const bool bCurrEPressed = keyboardState_.isKeyPressed(kECode);
                    const bool bCurrFPressed = keyboardState_.isKeyPressed(kFCode);

                    const bool bCurrRPressed = keyboardState_.isKeyPressed(kRCode);
                    const bool bCurrIPressed = keyboardState_.isKeyPressed(kICode);
                    const bool bCurrPPressed = keyboardState_.isKeyPressed(kPCode);

                    if (bCurrEnterPressed && !bPrevEnterPressed_)
                    {
                        inputResult.bConfirmed_ = allowedAction.bConfirm;
                    }

                    if (bCurrEPressed && !bPrevEPressed_)
                    {
                        inputResult.bInteracted_ = allowedAction.bInteract;
                    }

                    if (bCurrFPressed && !bPrevFPressed_)
                    {
                        inputResult.bLightToggled_ = allowedAction.bToggleLight;
                    }

                    if (bCurrRPressed && !bPrevRPressed_)
                    {
                        inputResult.bRPressed_ = allowedAction.bBackdoorR;
                    }

                    if (bCurrIPressed && !bPrevIPressed_)
                    {
                        inputResult.bIPressed_ = allowedAction.bBackdoorI;
                    }

                    if (bCurrPPressed && !bPrevPPressed_)
                    {
                        inputResult.bPPressed_ = allowedAction.bBackdoorP;
                    }

                    bPrevEnterPressed_ = bCurrEnterPressed;
                    bPrevEPressed_ = bCurrEPressed;
                    bPrevFPressed_ = bCurrFPressed;

                    bPrevRPressed_ = bCurrRPressed;
                    bPrevIPressed_ = bCurrIPressed;
                    bPrevPPressed_ = bCurrPPressed;
                }

                return inputResult;
            }
        }
    } // namespace win32console
} // namespace platform
