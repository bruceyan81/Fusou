// Copyright (c) 2026 Bruce Yan. All Rights Reserved.

/**
 * @brief platform input の抽象 interface と入力結果を定義する
 */

#pragma once

#include "src/Core/Types/Types.h"

namespace core
{
    namespace ports
    {
        enum class InputContext
        {
            Intro,
            Cutscene,
            Game,
            Fin,
            Died,
        };

        enum class StepDirection
        {
            None,
            Left,
            Right,
            Up,
            Down,
        };

        /**
         * @brief 1 フレームに入力した結果
         */
        struct InputResult final
        {
            // 任意キー Cutscene のみ
            bool bAnyKeyPressed_{false};

            // ESC
            bool bQuitRequested_{false};

            // Enter
            bool bConfirmed_{false};

            // E
            bool bInteracted_{false};

            // F
            bool bLightToggled_{false};

            // R
            bool bRPressed_{false};

            // I
            bool bIPressed_{false};

            // P
            bool bPPressed_{false};

            core::types::Vec2f playerMoveAxis_{};
            core::types::Vec2f lightMoveAxis_{};

            // TODO StepDirection を瞬間的な一 cell 移動入力だと分かる名前へ変える
            // W A S D を押す瞬間に発生する一 cell 移動入力
            bool bStepLeftPressed_{false};
            bool bStepRightPressed_{false};
            bool bStepUpPressed_{false};
            bool bStepDownPressed_{false};

            StepDirection stepPrimaryDir_{StepDirection::None};
        };

        /**
         * @brief 入力機能の抽象化ポート
         */
        class InputPort
        {
        public:
            virtual ~InputPort() noexcept = default;

            /**
             * @brief 入力をサンプルする
             * @return 入力の結果
             */
            [[nodiscard]] virtual InputResult sample(InputContext inputContext) = 0;
        };
    } // namespace ports
} // namespace core
