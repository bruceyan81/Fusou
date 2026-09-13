// Copyright (c) 2026 Bruce Yan. All Rights Reserved.

/**
 * @brief scene ごとの入力文脈と基本入力遷移を処理する
 */

#include "src/Core/Game/GameScene.h"
#include "src/Core/Game/GameModel.h"

namespace core
{
    namespace game
    {
        namespace
        {
            void cutsceneInitialization(GameState& gameState) noexcept
            {
                auto& state = gameState.scene_.cutscene_;

                state.cutsceneSecAccumulator_ = 0.0f;
                state.bIsInputAllowed = false;
                state.holdSecAccumulator_ = 0.0f;
                state.shownCharCount_ = 0;
                state.bFinished_ = false;
                state.inputBlockSecAccumulator_ = 0.0f;
            }
        } // namespace

        [[nodiscard]] ports::InputContext getInputContextByScene(const GameScene scene) noexcept
        {
            switch (scene)
            {
                case GameScene::Intro:
                    return ports::InputContext::Intro;
                case GameScene::Cutscene:
                    return ports::InputContext::Cutscene;
                case GameScene::Game:
                    return ports::InputContext::Game;
                case GameScene::FinalCutscene:
                    return ports::InputContext::Cutscene;
                case GameScene::Fin:
                    return ports::InputContext::Fin;
                case GameScene::Died:
                    return ports::InputContext::Died;
                default:
                    return ports::InputContext::Intro;
            }
        };

        void updateSceneByInput(GameState& gameState, const ports::InputResult& inputResult) noexcept
        {
            switch (gameState.scene_.current_)
            {
                case GameScene::Intro:
                    if (inputResult.bConfirmed_)
                    {
                        gameState.goal_.bIsClear_ = false;
                        gameState.goal_.goalLitFlags_.assign(gameState.goal_.goalLitFlags_.size(), false);
                        gameState.scene_.current_ = GameScene::Cutscene;
                        cutsceneInitialization(gameState);
                    }
                    break;

                case GameScene::Cutscene:
                case GameScene::FinalCutscene:
                case GameScene::Game:
                    break;

                case GameScene::Fin:
                    if (inputResult.bConfirmed_)
                    {
                        gameState.scene_.current_ = GameScene::Intro;
                    }
                    break;

                case GameScene::Died:
                    if (inputResult.bConfirmed_)
                    {
                        gameState.scene_.current_ = GameScene::Intro;
                    }
                    break;
            }
        };

    } // namespace game
} // namespace core
