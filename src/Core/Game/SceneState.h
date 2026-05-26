// Copyright (c) 2026 Bruce Yan. All Rights Reserved.

/**
 * @brief scene ごとの runtime state を定義する
 */

#pragma once

#include "src/Core/Game/GameConstants.h"
#include "src/Core/Game/GameScene.h"

namespace core
{
    namespace game
    {
        struct CutsceneState final
        {
            float cutsceneSecAccumulator_{0.0f};

            bool bIsInputAllowed{false};

            float holdSecAccumulator_{0.0f};

            int shownCharCount_{0};

            bool bFinished_{false};

            float inputBlockSecAccumulator_{0.0f};
        };

        enum class FinalCutscenePhase
        {
            PanningUp,
            Hold,
        };

        struct FinalCutsceneState final
        {
            FinalCutscenePhase phase_{FinalCutscenePhase::PanningUp};

            int ticksUntilNextCell_{kFinalCutsceneTicksPerCell};

            float holdRemainingSeconds_{kFinalCutsceneHoldSeconds};

            int targetCameraOriginX_{};
            int startCameraOriginY_{};
            int endCameraOriginY_{};

            int cameraOriginY_{};
        };

        struct SceneRuntimeState final
        {
            GameScene current_{GameScene::Intro};

            float introUiShowSeconds_{1.0f};
            float introUiHideSeconds_{0.5f};

            float introUiPhaseSeconds_{0.0f};

            bool bIntroUiVisible_{true};

            CutsceneState cutscene_{};

            FinalCutsceneState finalCutscene_{};
        };
    } // namespace game
} // namespace core
