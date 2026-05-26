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
            [[nodiscard]] core::ports::StepDirection toCoreStepDir(const StepDirection dir) noexcept
            {
                switch (dir)
                {
                    case StepDirection::Left:
                        return core::ports::StepDirection::Left;

                    case StepDirection::Right:
                        return core::ports::StepDirection::Right;

                    case StepDirection::Up:
                        return core::ports::StepDirection::Up;

                    case StepDirection::Down:
                        return core::ports::StepDirection::Down;

                    default:
                        return core::ports::StepDirection::None;
                }
            }

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

            // 前回の状態を保存する
            {
                stepPrimaryDir_ = StepDirection::None;

                if (allowedAction.bPlayerAxis)
                {
                    const bool bCurrWPressed = keyboardState_.isKeyPressed(kWCode);
                    const bool bCurrAPressed = keyboardState_.isKeyPressed(kACode);
                    const bool bCurrSPressed = keyboardState_.isKeyPressed(kSCode);
                    const bool bCurrDPressed = keyboardState_.isKeyPressed(kDCode);

                    bStepUpPressed_ = bCurrWPressed && !bPrevWPressed_;
                    bStepLeftPressed_ = bCurrAPressed && !bPrevAPressed_;
                    bStepDownPressed_ = bCurrSPressed && !bPrevSPressed_;
                    bStepRightPressed_ = bCurrDPressed && !bPrevDPressed_;

                    if (bStepLeftPressed_)
                    {
                        stepPrimaryDir_ = StepDirection::Left;
                    }
                    else if (bStepRightPressed_)
                    {
                        stepPrimaryDir_ = StepDirection::Right;
                    }
                    else if (bStepUpPressed_)
                    {
                        stepPrimaryDir_ = StepDirection::Up;
                    }
                    else if (bStepDownPressed_)
                    {
                        stepPrimaryDir_ = StepDirection::Down;
                    }

                    bPrevWPressed_ = bCurrWPressed;
                    bPrevAPressed_ = bCurrAPressed;
                    bPrevSPressed_ = bCurrSPressed;
                    bPrevDPressed_ = bCurrDPressed;
                }
                else
                {
                    bStepUpPressed_ = false;
                    bStepLeftPressed_ = false;
                    bStepDownPressed_ = false;
                    bStepRightPressed_ = false;

                    // 非 Game 中は prev をクリアしておく（Game 入りで edge を正しく拾う）
                    bPrevWPressed_ = false;
                    bPrevAPressed_ = false;
                    bPrevSPressed_ = false;
                    bPrevDPressed_ = false;
                }

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

                inputResult.bStepLeftPressed_ = bStepLeftPressed_;
                inputResult.bStepRightPressed_ = bStepRightPressed_;
                inputResult.bStepUpPressed_ = bStepUpPressed_;
                inputResult.bStepDownPressed_ = bStepDownPressed_;

                inputResult.stepPrimaryDir_ = toCoreStepDir(stepPrimaryDir_);

                return inputResult;
            }
        }
    } // namespace win32console
} // namespace platform
