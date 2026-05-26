// Copyright (c) 2026 Bruce Yan. All Rights Reserved.

/**
 * @brief Game scene の各 system を固定 Tick 順に実行する
 */

#include "src/Core/Game/CameraFollow.h"
#include "src/Core/Game/EnemyAi.h"
#include "src/Core/Game/EnemySim.h"
#include "src/Core/Game/GameTick.h"
#include "src/Core/Game/GoalWaveEffect.h"
#include "src/Core/Game/LadderSim.h"
#include "src/Core/Game/GoalActivation.h"
#include "src/Core/Game/LightSystem.h"
#include "src/Core/Game/PlayerContact.h"
#include "src/Core/Game/PlayerMoveRules.h"
#include "src/Core/Game/TileRules.h"

#include <algorithm>
#include <chrono>
#include <cmath>

namespace core
{
    namespace game
    {
        namespace
        {
            [[nodiscard]] float calculateDeltaSeconds(const types::Duration fixedDeltaTime) noexcept
            {
                return std::chrono::duration<float, std::ratio<1>>(fixedDeltaTime).count();
            }

            [[nodiscard]] int calculateEnemyAlertCooldownDurationTicks(const types::Duration fixedDeltaTime) noexcept
            {
                constexpr float kAlertCooldownSeconds = 0.5f;

                const float dtSeconds =
                    std::chrono::duration_cast<std::chrono::duration<float>>(fixedDeltaTime).count();
                if (dtSeconds <= 0.0f)
                {
                    return 0;
                }

                const int ticks = static_cast<int>(std::ceil(kAlertCooldownSeconds / dtSeconds));
                return (ticks > 0) ? ticks : 1;
            }

            [[nodiscard]] PlayerMovement createPlayerMovementByInputResult(
                const ports::InputResult& inputResult) noexcept
            {
                PlayerMovement movement{};

                const auto axis = inputResult.playerMoveAxis_;

                movement.moveIntentX_ = (axis.x_ > 0) ? 1 : (axis.x_ < 0) ? -1 : 0;
                movement.climbIntentY_ = (axis.y_ > 0) ? 1 : (axis.y_ < 0) ? -1 : 0;
                movement.bHasMoveInput_ = (movement.moveIntentX_ != 0);

                return movement;
            }

            void integratePlayerVelocityNormal(
                PlayerState& playerState, const PlayerMovement& movement, const float dt) noexcept
            {
                if (!movement.bHasMoveInput_)
                {
                    playerState.velocity_.x_ = 0.0f;
                }
                else
                {
                    const float ax = kHorizontalAcceleration * static_cast<float>(movement.moveIntentX_);
                    const float nextVx = playerState.velocity_.x_ + (ax * dt);

                    playerState.velocity_.x_ = std::clamp(nextVx, -kMaxHorizontalSpeed, kMaxHorizontalSpeed);
                }

                const float nextVy = playerState.velocity_.y_ + (kGravity * dt);
                playerState.velocity_.y_ = std::clamp(nextVy, -kMaxFallSpeed, kMaxFallSpeed);
            }

            void integratePlayerVelocityOnLadder(PlayerState& playerState, const PlayerMovement& movement) noexcept
            {
                playerState.velocity_.x_ = 0.0f;
                playerState.velocity_.y_ = kLadderClimbSpeed * static_cast<float>(movement.climbIntentY_);
            }

            [[nodiscard]] int calculateSubstepCountByVelocity(const types::Vec2f& velocity, const float dt) noexcept
            {
                constexpr float kSubstepMaxDelta = 0.5f;

                const float totalDx = velocity.x_ * dt;
                const float totalDy = velocity.y_ * dt;
                const float maxAbsDelta = std::max(std::abs(totalDx), std::abs(totalDy));

                if (maxAbsDelta == 0.0f)
                {
                    return 1;
                }

                return static_cast<int>(std::ceil(maxAbsDelta / kSubstepMaxDelta));
            }

            void resolveMovePlayerAxisX(PlayerState& playerState, const float dt, const world::TileMap& gameMap,
                const world::TileMap& playerMap, const LightState& lightState, const world::Camera& camera) noexcept
            {
                const float dx = playerState.velocity_.x_ * dt;

                if (dx == 0.0f)
                {
                    return;
                }

                const types::Vec2f currFeetPos = playerState.feetWorldPos_;
                const types::Vec2f candidateFeetPos{currFeetPos.x_ + dx, currFeetPos.y_};

                if (canPlayerMoveToByFeet(
                        gameMap, playerMap, currFeetPos, candidateFeetPos, lightState, camera, false))
                {
                    playerState.feetWorldPos_ = candidateFeetPos;
                    return;
                }

                playerState.velocity_.x_ = 0.0f;
            }

            void resolveMovePlayerAxisY(PlayerState& playerState, const float dt, const world::TileMap& gameMap,
                const world::TileMap& playerMap, const LightState& lightState, const world::Camera& camera) noexcept
            {
                const float dy = playerState.velocity_.y_ * dt;

                if (dy == 0.0f)
                {
                    return;
                }

                const types::Vec2f currFeetPos = playerState.feetWorldPos_;
                const types::Vec2f candidateFeetPos{currFeetPos.x_, currFeetPos.y_ + dy};

                const bool bTreatClimbableLadderAsBlocking =
                    (playerState.movementState_ == PlayerMovementState::Normal) && (dy > 0.0f);

                if (canPlayerMoveToByFeet(gameMap, playerMap, currFeetPos, candidateFeetPos, lightState, camera,
                        bTreatClimbableLadderAsBlocking))
                {
                    playerState.feetWorldPos_ = candidateFeetPos;
                    return;
                }

                if (dy > 0.0f)
                {
                    playerState.bGrounded_ = true;
                }

                playerState.velocity_.y_ = 0.0f;
            }

            void solvePlayerMovementOneStep(PlayerState& playerState, const float dtStep,
                const world::TileMap& gameMap, const world::TileMap& playerMap, const LightState& lightState,
                const world::Camera& camera) noexcept
            {
                resolveMovePlayerAxisX(playerState, dtStep, gameMap, playerMap, lightState, camera);
                resolveMovePlayerAxisY(playerState, dtStep, gameMap, playerMap, lightState, camera);
            }

            /**
             * @brief 入力意図、Ladder 処理、速度積分、当たり判定をまとめて player に反映する
             */
            void tickPlayerSim(GameState& gameState, const float dt, const ports::InputResult& inputResult) noexcept
            {
                const types::Vec2 playerFeetCellBeforeStep = getPlayerFeetCell(gameState.player_.feetWorldPos_);
                PlayerMovement    simMovement = createPlayerMovementByInputResult(inputResult);
                PlayerMovement    stateMovement = simMovement;

                // Ladder step は simMovement を消費して、後続の速度積分に反映される
                const LadderMoveResult ladderMoveResult =
                    runLadderMoveMode(gameState, simMovement, inputResult, playerFeetCellBeforeStep);

                gameState.player_.bGrounded_ = false;

                if (gameState.player_.movementState_ == PlayerMovementState::Normal)
                {
                    integratePlayerVelocityNormal(gameState.player_, simMovement, dt);
                }
                else
                {
                    integratePlayerVelocityOnLadder(gameState.player_, simMovement);
                }

                const int   rawSteps = calculateSubstepCountByVelocity(gameState.player_.velocity_, dt);
                const int   steps = std::min(rawSteps, 128);
                const float dtStep = dt / static_cast<float>(steps);

                for (int i = 0; i < steps; ++i)
                {
                    solvePlayerMovementOneStep(gameState.player_, dtStep, gameState.assets_.gameScene_.tileMap_,
                        gameState.assets_.player_.tileMap_, gameState.light_, gameState.mainCamera_);
                }

                if (ladderMoveResult.bConsumedY_ && stateMovement.climbIntentY_ == 0)
                {
                    stateMovement.climbIntentY_ = (ladderMoveResult.stepDir_ == ports::StepDirection::Up) ? -1 : 1;
                }

                // stateMovement は Ladder 状態を収束させるため、消費前の入力意図を残す
                runLadderStateFinalize(gameState, stateMovement);
            }

            /**
             * @brief 固定 Tick 開始時点の値を各処理に同じ基準として渡す
             */
            struct FixedStepContext
            {
                float       dtSeconds{};
                types::Vec2 worldSize{};
                types::Vec2 viewportOriginBeforeCameraFollow{};
                types::Vec2 playerFeetCellBeforeMove{};
            };

            void runEnemyAiSim(GameState& gameState, const FixedStepContext& fixedStepContext)
            {
                for (auto& enemy : gameState.enemyRuntime_.enemies_)
                {
                    const int moveIntentX = tickEnemyAi(enemy, gameState.player_.feetWorldPos_,
                        gameState.enemyRuntime_.chaseRange_, gameState.enemyRuntime_.alertCooldownDurationTicks_,
                        gameState.tickIndex_, gameState.assets_.gameScene_.tileMap_, gameState.mainCamera_,
                        gameState.light_);
                    tickEnemySim(enemy, moveIntentX, fixedStepContext.dtSeconds, gameState.enemyRuntime_.moveSpeed_,
                        gameState.assets_.gameScene_.tileMap_, gameState.mainCamera_, gameState.light_);
                }
            }
        } // namespace

        void tickGameStateByFixedStep(
            GameState& gameState, types::Duration fixedDeltaTime, const ports::InputResult& inputResult)
        {
            FixedStepContext fixedStepContext{};

            fixedStepContext.dtSeconds = calculateDeltaSeconds(fixedDeltaTime);
            fixedStepContext.worldSize = types::Vec2{
                gameState.assets_.gameScene_.tileMap_.getWidth(),
                gameState.assets_.gameScene_.tileMap_.getHeight()
            };
            fixedStepContext.viewportOriginBeforeCameraFollow = types::Vec2{
                gameState.mainCamera_.getViewportOriginXAtWorld(),
                gameState.mainCamera_.getViewportOriginYAtWorld()
            };
            fixedStepContext.playerFeetCellBeforeMove = getPlayerFeetCell(gameState.player_.feetWorldPos_);

            if (gameState.enemyRuntime_.alertCooldownDurationTicks_ <= 0)
            {
                gameState.enemyRuntime_.alertCooldownDurationTicks_ =
                    calculateEnemyAlertCooldownDurationTicks(fixedDeltaTime);
            }

            // Light の更新は一番優先、以降の判定が最新の Light 状態により
            runLightToggle(gameState.light_, inputResult, fixedStepContext.viewportOriginBeforeCameraFollow);
            runLightMove(gameState.light_, inputResult, fixedStepContext.viewportOriginBeforeCameraFollow);

            // 敵は現時点の player と light により動く
            runEnemyAiSim(gameState, fixedStepContext);

            // Ladder の判定は player 移動前の足元 cell を基準にする
            runLadderPrecheck(gameState, fixedStepContext.playerFeetCellBeforeMove);
            tickPlayerSim(gameState, fixedStepContext.dtSeconds, inputResult);

            // Goal の接近と点灯は player 移動後に確定する
            runGoalActivation(gameState, inputResult);
            runGoalWaveEffect(gameState, fixedDeltaTime);

            // Goal wave などで Game シーンを出た後は接触や camera を進めない
            if (gameState.scene_.current_ != GameScene::Game)
            {
                return;
            }

            // 接触処理は player の damage respawn Died 遷移を起こす可能性がある
            if (runPlayerContact(gameState))
            {
                return;
            }

            // Camera は Fixed Tick の最後に player の確定位置へ追従する
            runCameraFollow(gameState.mainCamera_, gameState.player_, gameState.light_, fixedStepContext.worldSize);

            ++gameState.tickIndex_;
        }

    } // namespace game
} // namespace core
