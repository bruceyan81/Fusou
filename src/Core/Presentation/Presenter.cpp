// Copyright (c) 2026 Bruce Yan. All Rights Reserved.

/**
 * @brief Cell buffer へ描画する
 */

#include "src/Core/Game/GameModel.h"
#include "src/Core/Game/GameScene.h"
#include "src/Core/Presentation/BackgroundSweepEffect.h"
#include "src/Core/Presentation/Presenter.h"
#include "src/Core/Presentation/TilePalette.h"
#include "src/Core/Render/CellBuffer.h"
#include "src/Core/Types/Types.h"
#include "src/Core/UI/UI.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <string_view>

namespace core
{
    namespace presentation
    {
        namespace
        {
            struct CellColorOverride
            {
                bool bEnabled_{ false };
                bool bHasFg_{ false };
                bool bHasBg_{ false };

                render::Color targetFg_{ render::Color::White };
                render::Color targetBg_{ render::Color::Black };
            };

            constexpr std::array<core::world::TileId, 2> kNearGoalHighlightTileIds = {
                core::world::tileid::kGoal, core::world::tileid::kSun };
            constexpr core::render::Color kNearGoalHighlightFg = core::render::Color::Yellow;
            constexpr core::render::Color kNearGoalHighlightBg = core::render::Color::LightRed;

            constexpr core::render::Glyph kEnemyGlyph = U'Ｘ';
            constexpr core::render::Color kEnemyFg = core::render::Color::LightRed;
            constexpr core::render::Color kEnemyBg = core::render::Color::Black;

            std::size_t getOverrideIndex(const types::Vec2 viewportCell) noexcept
            {
                return static_cast<std::size_t>(viewportCell.y_ * render::CellBuffer::kWidth + viewportCell.x_);
            }

            constexpr std::array<core::world::TileId, 1> kIntroSweepTileIds = { static_cast<core::world::TileId>(26) };

            constexpr core::presentation::SweepBandConfig kIntroSweepConfig{ 2, 20.0f, 5.0f };
            constexpr core::presentation::SweepBandConfig kFinSweepConfig = kIntroSweepConfig;
            constexpr float                               kDiedBlinkPeriodSeconds = 1.0f;
            constexpr core::world::TileId                 kDiedBlinkTargetTileId = core::world::tileid::kColorRed;

            void setOverride(CellColorOverride& cellColorOverride, bool bHasFg, render::Color targetFg, bool bHasBg,
                render::Color targetBg) noexcept
            {
                cellColorOverride.bEnabled_ = true;
                if (bHasFg)
                {
                    cellColorOverride.bHasFg_ = true;
                    cellColorOverride.targetFg_ = targetFg;
                }

                if (bHasBg)
                {
                    cellColorOverride.bHasBg_ = true;
                    cellColorOverride.targetBg_ = targetBg;
                }
            }

            [[nodiscard]] float toSeconds(core::types::Duration d) noexcept
            {
                return std::chrono::duration_cast<std::chrono::duration<float>>(d).count();
            }

            [[nodiscard]] bool isWhitelistedTileId(
                const core::world::TileId tileId, const std::array<core::world::TileId, 1>& whitelist) noexcept
            {
                return tileId == whitelist[0];
            }

            [[nodiscard]] bool isInsideDiagonalBand(const int worldX, const int worldY, const float phaseSeconds,
                const core::presentation::SweepBandConfig& cfg) noexcept
            {
                const float period = cfg.periodSeconds_;
                if (period <= 0.0f)
                {
                    return false;
                }

                const float t = std::fmod(phaseSeconds, period);

                const float center = cfg.speedCellsPerSecond_ * t;

                const float s = static_cast<float>(worldX + worldY);

                const float halfWidth = static_cast<float>(cfg.bandWidthCells_) * 0.5f;

                return std::fabs(s - center) < halfWidth;
            }

            template <typename Pred>
            void applySweepBand(const core::world::TileMap& tileMap, Pred pred, const float phaseSeconds,
                const core::presentation::SweepBandConfig& cfg, const bool bHasFg, const core::render::Color fg,
                const bool bHasBg, const core::render::Color bg, core::render::CellBuffer& ioFrame) noexcept
            {
                for (int y = 0; y < core::render::CellBuffer::kHeight; ++y)
                {
                    for (int x = 0; x < core::render::CellBuffer::kWidth; ++x)
                    {
                        if (!isInsideDiagonalBand(x, y, phaseSeconds, cfg))
                        {
                            continue;
                        }

                        const core::world::TileId tileId = tileMap.getTileIdByCell({ x, y });
                        if (!pred(tileId))
                        {
                            continue;
                        }

                        auto& cell = ioFrame.at({ x, y });

                        if (bHasFg)
                        {
                            cell.style_.fg_ = fg;
                        }

                        if (bHasBg)
                        {
                            cell.style_.bg_ = bg;
                        }
                    }
                }
            }

            template <typename ColorFn>
            void applyBgOverrideByTileId(const core::world::TileMap& tileMap, const core::world::TileId targetTileId,
                ColorFn colorFn, core::render::CellBuffer& ioFrame) noexcept
            {
                for (int y = 0; y < core::render::CellBuffer::kHeight; ++y)
                {
                    for (int x = 0; x < core::render::CellBuffer::kWidth; ++x)
                    {
                        const core::world::TileId tileId = tileMap.getTileIdByCell({ x, y });
                        if (tileId != targetTileId)
                        {
                            continue;
                        }

                        auto& cell = ioFrame.at({ x, y });
                        cell.style_.bg_ = colorFn();
                    }
                }
            }

            void applyIntroSweepBand(const core::game::GameState& gameState,
                const std::array<core::world::TileId, 1>& whitelist, const core::presentation::SweepBandConfig& cfg,
                core::render::CellBuffer& ioFrame) noexcept
            {
                const float seconds = toSeconds(gameState.effect_.bgTime_);

                const auto pred = [&whitelist](const core::world::TileId tileId) noexcept {
                    return isWhitelistedTileId(tileId, whitelist);
                    };

                applySweepBand(gameState.assets_.introScene_.tileMap_, pred, seconds, cfg, true,
                    core::render::Color::LightRed, true, core::render::Color::Yellow, ioFrame);
            }

            void applyFinSweepHighlight(const core::game::GameState& gameState,
                const core::presentation::SweepBandConfig& cfg, core::render::CellBuffer& ioFrame) noexcept
            {
                const float seconds = toSeconds(gameState.effect_.bgTime_);

                const auto pred = [](const core::world::TileId tileId) noexcept {
                    return tileId != core::world::tileid::kSpace;
                    };

                applySweepBand(gameState.assets_.finScene_.tileMap_, pred, seconds, cfg, false,
                    core::render::Color::White, true, core::render::Color::White, ioFrame);
            }

            void applyDiedBlink(const core::game::GameState& gameState, core::render::CellBuffer& ioFrame) noexcept
            {
                const float seconds = toSeconds(gameState.effect_.bgTime_);

                const int phase = static_cast<int>(std::floor(seconds / kDiedBlinkPeriodSeconds));

                const core::render::Color targetBg =
                    ((phase % 2) == 0) ? core::render::Color::Red : core::render::Color::Magenta;

                const auto colorFn = [targetBg]() noexcept { return targetBg; };

                applyBgOverrideByTileId(
                    gameState.assets_.diedScene_.tileMap_, kDiedBlinkTargetTileId, colorFn, ioFrame);
            }

            void presentIntroScene(
                const core::game::GameState& gameState, const auto& draw, core::render::CellBuffer& outFrame) noexcept
            {
                // ベースを描画
                draw(gameState.assets_.introScene_.tileMap_, gameState.assets_.introScene_.colorPalette_);

                applyIntroSweepBand(gameState, kIntroSweepTileIds, kIntroSweepConfig, outFrame);

                // intro UI を描画
                if (gameState.scene_.bIntroUiVisible_)
                {
                    ui::drawUI(outFrame, { 0, 23 }, ui::str::kIntroHint1, ui::TextAlign::Center);
                }
            }

            void presentCutsceneScene(
                const core::game::GameState& gameState, core::render::CellBuffer& outFrame) noexcept
            {
                constexpr int kCutsceneStartY = 2;
                constexpr int kCutsceneStartX = 8;
                constexpr int kCutsceneHintY = render::CellBuffer::kHeight - 2;

                // cutscene text を描画
                ui::drawCutsceneText(outFrame, kCutsceneStartY, gameState.scene_.cutscene_.shownCharCount_,
                    ui::TextAlign::Customization, kCutsceneStartX);

                // input hint UI を描画
                if (gameState.scene_.cutscene_.bIsInputAllowed)
                {
                    ui::drawUI(outFrame, { 0, kCutsceneHintY }, ui::str::kCutSceneHint1, ui::TextAlign::Center);
                }
            }

            void presentGameScene(
                const core::game::GameState& gameState,
                std::array<
                CellColorOverride,
                core::render::CellBuffer::kWidth* core::render::CellBuffer::kHeight
                >& overrides,
                const auto& applyPaletteColor,
                const auto& changeBackgroundColor,
                const auto& applyOverrides,
                core::render::CellBuffer& outFrame
            ) noexcept
            {
                // 描画対象の参照を準備
                const auto& gameTileMap = gameState.assets_.gameScene_.tileMap_;
                const auto& backgroundTileMap = gameState.assets_.gameSceneBackground_.tileMap_;
                const auto& camera = gameState.mainCamera_;
                const auto& player = gameState.assets_.player_.tileMap_;
                const auto& light = gameState.light_;

                const types::Vec2 playerFeetWorldCell{
                    static_cast<int>(std::floor(gameState.player_.feetWorldPos_.x_)),
                    static_cast<int>(std::floor(gameState.player_.feetWorldPos_.y_))
                };

                const int originX = camera.getViewportOriginXAtWorld();
                const int originY = camera.getViewportOriginYAtWorld();

                const int mapWidth = backgroundTileMap.getWidth();
                const int mapHeight = backgroundTileMap.getHeight();

                world::TileId backgroundTileId = world::tileid::kSpace;
                world::TileId mapTileId = world::tileid::kSpace;

                const render::Cell enemyCellTemplate{
                    kEnemyGlyph,
                    render::TextStyle{kEnemyFg, kEnemyBg},
                };

                auto drawEnemyAtWorldCell =
                    [&outFrame, &enemyCellTemplate, originX, originY]
                    (const types::Vec2 worldCell)
                    {
                        const int enemyViewportX = worldCell.x_ - originX;
                        const int enemyViewportY = worldCell.y_ - originY;

                        if (!render::CellBuffer::isInBounds({ enemyViewportX, enemyViewportY }))
                        {
                            return;
                        }

                        outFrame.at({ enemyViewportX, enemyViewportY }) = enemyCellTemplate;
                    };

                for (int viewportY = 0; viewportY < render::CellBuffer::kHeight; ++viewportY)
                {
                    for (int viewportX = 0; viewportX < render::CellBuffer::kWidth; ++viewportX)
                    {
                        const int worldX = originX + viewportX;
                        const int worldY = originY + viewportY;

                        // 背景を描画
                        {
                            backgroundTileId = backgroundTileMap.getTileIdByCell({ worldX, worldY });

                            auto cell = TilePalette::findCellById(backgroundTileId);

                            applyPaletteColor(cell, backgroundTileId, gameState.assets_.gameSceneBackground_.colorPalette_);

                            outFrame.at({ viewportX, viewportY }) = cell;
                        }

                        // 前景を描画
                        {
                            mapTileId = gameTileMap.getTileIdByCell({ worldX, worldY });

                            if (light.isHiddenTile(mapTileId))
                            {
                                if (!light.isLit({ viewportX, viewportY }))
                                {
                                    mapTileId = world::tileid::kSpace;
                                }
                            }

                            if (mapTileId != world::tileid::kSpace)
                            {
                                auto cell = TilePalette::findCellById(mapTileId);
                                applyPaletteColor(cell, mapTileId, gameState.assets_.gameScene_.colorPalette_);
                                outFrame.at({ viewportX, viewportY }) = cell;
                            }
                        }

                        // near goal の色を上書き
                        {
                            if (gameState.goal_.bIsNear_ && gameState.goal_.currGoalRegionIndex_ >= 0)
                            {
                                const auto& currGoalRegion = gameState.goal_.goalRegions_[gameState.goal_.currGoalRegionIndex_];

                                const world::TileId finalTileId =
                                    (mapTileId != world::tileid::kSpace) ? mapTileId : backgroundTileId;

                                if (worldY >= currGoalRegion.beginY_ && worldY <= currGoalRegion.endY_)
                                {
                                    for (auto highlightTileId : kNearGoalHighlightTileIds)
                                    {
                                        if (finalTileId == highlightTileId)
                                        {
                                            auto& cell = outFrame.at({ viewportX, viewportY });
                                            cell.style_.fg_ = kNearGoalHighlightFg;
                                            cell.style_.bg_ = kNearGoalHighlightBg;
                                        }
                                    }
                                }
                            }
                        }

                        // 永続の色を上書き
                        {
                            const bool bInBounds =
                                (0 <= worldX && worldX < mapWidth) && (0 <= worldY && worldY < mapHeight);

                            if (bInBounds)
                            {
                                const auto& paintOverride = gameState.goal_.paintOverrides_[worldY * mapWidth + worldX];

                                if (paintOverride.bEnabled_)
                                {
                                    auto& cell = outFrame.at({ viewportX, viewportY });

                                    if (paintOverride.bHasFg_)
                                    {
                                        cell.style_.fg_ = paintOverride.fg_;
                                    }

                                    if (paintOverride.bHasBg_)
                                    {
                                        cell.style_.bg_ = paintOverride.bg_;
                                    }
                                }
                            }
                        }

                        // wave の色を上書き
                        {
                            const bool bInBounds =
                                (0 <= worldX && worldX < mapWidth) && (0 <= worldY && worldY < mapHeight);

                            if (bInBounds)
                            {
                                const auto& paintOverride = gameState.goal_.wavePaintOverrides_[worldY * mapWidth + worldX];

                                if (paintOverride.bEnabled_)
                                {
                                    auto& cell = outFrame.at({ viewportX, viewportY });

                                    if (paintOverride.bHasFg_)
                                    {
                                        cell.style_.fg_ = paintOverride.fg_;
                                    }

                                    if (paintOverride.bHasBg_)
                                    {
                                        cell.style_.bg_ = paintOverride.bg_;
                                    }
                                }
                            }
                        }

                        // player を描画
                        {
                            const int playerFeetOriginX = playerFeetWorldCell.x_;
                            const int playerFeetY = playerFeetWorldCell.y_;
                            const int playerFeetOriginY = playerFeetY - 1;

                            const types::Vec2 playerLocalCell{ worldX - playerFeetOriginX, worldY - playerFeetOriginY };

                            if (!player.isCellInBounds(playerLocalCell))
                            {
                                continue;
                            }

                            const auto playerTileId = player.getTileIdByCell(playerLocalCell);

                            {
                                const auto coveredBg = outFrame.at({ viewportX, viewportY }).style_.bg_;
                                auto& cellColorOverride = overrides.at(getOverrideIndex({ viewportX, viewportY }));

                                setOverride(cellColorOverride, false, render::Color::White, true, coveredBg);
                            }

                            auto cell = TilePalette::findCellById(playerTileId);
                            applyPaletteColor(cell, playerTileId, gameState.assets_.player_.colorPalette_);
                            outFrame.at({ viewportX, viewportY }) = cell;
                        }
                    }
                }

                // enemy を描画
                for (const auto& enemy : gameState.enemyRuntime_.enemies_)
                {
                    const types::Vec2 enemyWorldCell{ static_cast<int>(std::floor(enemy.worldPos_.x_)),
                        static_cast<int>(std::floor(enemy.worldPos_.y_)) };

                    drawEnemyAtWorldCell(enemyWorldCell);
                }

                // player の背景色を復元
                applyOverrides();

                // light の背景色を強調
                if (light.bEnabled_)
                {
                    const int left = light.lightViewportCell_.x_;
                    const int top = light.lightViewportCell_.y_;

                    const int beginX = std::max(0, left);
                    const int beginY = std::max(0, top);
                    const int endX = std::min(render::CellBuffer::kWidth, left + game::kLightSize);
                    const int endY = std::min(render::CellBuffer::kHeight, top + game::kLightSize);

                    for (int y = beginY; y < endY; ++y)
                    {
                        for (int x = beginX; x < endX; ++x)
                        {
                            auto& cell = outFrame.at({ x, y });
                            cell.style_.bg_ = changeBackgroundColor(cell.style_.bg_);
                        }
                    }
                }

                // lives UI を描画
                {
                    std::u32string livesText = U"ＬＩＶＥＳ：　";

                    ui::appendFullWidthInt(livesText, gameState.player_.currLives_);

                    livesText.push_back(U'／');

                    ui::appendFullWidthInt(livesText, gameState.player_.maxLives_);

                    ui::drawUIStyled(outFrame, { 0, 0 }, std::u32string_view(livesText), render::Color::Black,
                        render::Color::White, ui::TextAlign::Customization, 0);
                }

                // enemy alert UI を描画
                {
                    auto clampInt = [](int v, int lo, int hi) noexcept {
                        if (v < lo)
                            return lo;
                        if (v > hi)
                            return hi;
                        return v;
                        };

                    auto drawAlertStyled = [&outFrame, &clampInt](
                        int baseX, int baseY, std::u32string_view text) noexcept {
                            const int len = static_cast<int>(text.size());
                            if (len <= 0)
                            {
                                return;
                            }

                            const int y = clampInt(baseY, 0, render::CellBuffer::kHeight - 1);

                            const int maxX = render::CellBuffer::kWidth - len;
                            if (maxX < 0)
                            {
                                return;
                            }

                            const int x = clampInt(baseX, 0, maxX);

                            ui::drawUIStyled(outFrame, { 0, y }, text, render::Color::LightRed, render::Color::White,
                                ui::TextAlign::Customization, x);
                        };

                    for (const auto& enemy : gameState.enemyRuntime_.enemies_)
                    {
                        if (enemy.alert_ == game::EnemyAlert::None || enemy.alertRemainingTicks_ <= 0)
                        {
                            continue;
                        }

                        const types::Vec2 enemyWorldCell{ static_cast<int>(std::floor(enemy.worldPos_.x_)),
                            static_cast<int>(std::floor(enemy.worldPos_.y_)) };

                        const int enemyViewportX = enemyWorldCell.x_ - originX;
                        const int enemyViewportY = enemyWorldCell.y_ - originY;

                        if (!render::CellBuffer::isInBounds({ enemyViewportX, enemyViewportY }))
                        {
                            continue;
                        }

                        const int alertBaseX = enemyViewportX;
                        const int alertBaseY = enemyViewportY - 1;

                        if (enemy.alert_ == game::EnemyAlert::Spotted)
                        {
                            drawAlertStyled(alertBaseX, alertBaseY, U"！");
                        }
                        else if (enemy.alert_ == game::EnemyAlert::Lost)
                        {
                            drawAlertStyled(alertBaseX, alertBaseY, U"。。。");
                        }
                    }
                }
            }

            void presentFinalCutsceneScene(const core::game::GameState& gameState, const auto& applyPaletteColor,
                core::render::CellBuffer& outFrame) noexcept
            {
                // 描画対象の参照を準備
                const auto& backgroundTileMap = gameState.assets_.gameSceneBackground_.tileMap_;
                const auto& camera = gameState.mainCamera_;

                const int originX = camera.getViewportOriginXAtWorld();
                const int originY = camera.getViewportOriginYAtWorld();

                const int mapWidth = backgroundTileMap.getWidth();
                const int mapHeight = backgroundTileMap.getHeight();

                // viewport を走査
                for (int viewportY = 0; viewportY < render::CellBuffer::kHeight; ++viewportY)
                {
                    for (int viewportX = 0; viewportX < render::CellBuffer::kWidth; ++viewportX)
                    {
                        const int worldX = originX + viewportX;
                        const int worldY = originY + viewportY;

                        world::TileId tileId = world::tileid::kSpace;

                        const bool bInBounds =
                            (0 <= worldX && worldX < mapWidth) && (0 <= worldY && worldY < mapHeight);

                        // 背景を描画
                        if (bInBounds)
                        {
                            tileId = backgroundTileMap.getTileIdByCell({ worldX, worldY });
                        }

                        auto cell = TilePalette::findCellById(tileId);
                        applyPaletteColor(cell, tileId, gameState.assets_.gameSceneBackground_.colorPalette_);

                        // 永続の色を上書き
                        if (bInBounds)
                        {
                            const auto& paintOverride = gameState.goal_.paintOverrides_[worldY * mapWidth + worldX];

                            if (paintOverride.bEnabled_)
                            {
                                if (paintOverride.bHasFg_)
                                {
                                    cell.style_.fg_ = paintOverride.fg_;
                                }

                                if (paintOverride.bHasBg_)
                                {
                                    cell.style_.bg_ = paintOverride.bg_;
                                }
                            }
                        }

                        outFrame.at({ viewportX, viewportY }) = cell;
                    }
                }
            }

            void presentFinScene(
                const core::game::GameState& gameState, const auto& draw, core::render::CellBuffer& outFrame) noexcept
            {
                // ベースを描画
                draw(gameState.assets_.finScene_.tileMap_, gameState.assets_.finScene_.colorPalette_);

                applyFinSweepHighlight(gameState, kFinSweepConfig, outFrame);
            }

            void presentDiedScene(
                const core::game::GameState& gameState, const auto& draw, core::render::CellBuffer& outFrame) noexcept
            {
                // ベースを描画
                draw(gameState.assets_.diedScene_.tileMap_, gameState.assets_.diedScene_.colorPalette_);

                applyDiedBlink(gameState, outFrame);
            }

        } // namespace

        void CellBufferPresenter::composeFrame(const game::GameState& gameState, render::CellBuffer& outFrame)
        {
            outFrame.fill(render::Cell{});

            // 動的な色上書きを保持
            std::array<CellColorOverride, render::CellBuffer::kWidth* render::CellBuffer::kHeight> overrides{};

            // palette を適用
            auto applyPaletteColor = [](auto& cell, const auto tileId, const auto& palette) {
                const std::size_t index = static_cast<std::size_t>(tileId);

                cell.style_.fg_ = static_cast<render::Color>(palette.fg_[index]);
                cell.style_.bg_ = static_cast<render::Color>(palette.bg_[index]);
                };

            // 背景色を変換
            auto changeBackgroundColor = [](const render::Color bg) noexcept {
                // 背景色は White の場合、LightGrayにする
                if (bg == render::Color::White)
                {
                    return render::Color::LightGray;
                }

                // render::Color は std::uint8_t、変更ルールは次の color にする
                const auto currBg = static_cast<std::uint8_t>(bg);
                return static_cast<render::Color>(currBg + 1);
                };

            // ベースを描画
            auto draw = [&outFrame, &applyPaletteColor](const auto& tileMap, const auto& palette) {
                const int height = std::min(tileMap.getHeight(), render::CellBuffer::kHeight);
                const int width = std::min(tileMap.getWidth(), render::CellBuffer::kWidth);

                for (int y = 0; y < height; ++y)
                {
                    for (int x = 0; x < width; ++x)
                    {
                        const auto tileId = tileMap.getTileIdByCell({ x, y });

                        auto cell = TilePalette::findCellById(tileId);
                        applyPaletteColor(cell, tileId, palette);

                        outFrame.at({ x, y }) = cell;
                    }
                }
                };

            // 上書きを反映
            auto applyOverrides = [&overrides, &outFrame]() noexcept {
                for (int viewportY = 0; viewportY < render::CellBuffer::kHeight; ++viewportY)
                {
                    for (int viewportX = 0; viewportX < render::CellBuffer::kWidth; ++viewportX)
                    {
                        const auto& cellColorOverride = overrides.at(getOverrideIndex({ viewportX, viewportY }));

                        if (!cellColorOverride.bEnabled_)
                        {
                            continue;
                        }

                        auto& cell = outFrame.at({ viewportX, viewportY });

                        if (cellColorOverride.bHasFg_)
                        {
                            cell.style_.fg_ = cellColorOverride.targetFg_;
                        }

                        if (cellColorOverride.bHasBg_)
                        {
                            cell.style_.bg_ = cellColorOverride.targetBg_;
                        }
                    }
                }
                };

            // scene ごとに描画
            switch (gameState.scene_.current_)
            {
            case game::GameScene::Intro:
            {
                presentIntroScene(gameState, draw, outFrame);
            }
            break;

            case game::GameScene::Cutscene:
            {
                presentCutsceneScene(gameState, outFrame);
            }
            break;

            case game::GameScene::Game:
            {
                presentGameScene(
                    gameState, overrides, applyPaletteColor, changeBackgroundColor, applyOverrides, outFrame);
            }
            break;

            case game::GameScene::FinalCutscene:
            {
                presentFinalCutsceneScene(gameState, applyPaletteColor, outFrame);
            }
            break;

            case game::GameScene::Fin:
            {
                presentFinScene(gameState, draw, outFrame);
            }
            break;

            case game::GameScene::Died:
            {
                presentDiedScene(gameState, draw, outFrame);
            }
            break;
            }
        }
    } // namespace presentation
} // namespace core
