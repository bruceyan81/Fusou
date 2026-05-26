// Copyright (c) 2026 Bruce Yan. All Rights Reserved.

/**
 * @brief tile の通行可否と可視性の rule を定義する
 */

#pragma once

#include "src/Core/Game/LightState.h"
#include "src/Core/Types/Types.h"
#include "src/Core/World/Camera.h"
#include "src/Core/World/TileMap.h"

namespace core
{
    namespace game
    {
        namespace detail
        {
            /**
             * @brief ビューポート cell = ワールド cell - カメラの左上のワールド cell
             * ビューポートの座標系はスクリーンのサイズ
             * x は 0 以上 40 未満 y は 0 以上 25 未満 左上の座標は {0, 0}
             * ワールド座標系はマップ全体の絶対座標系
             * カメラの左上の座標は「ビューポートの座標」の {0, 0} が「ワールド座標系」に対する座標点
             * 例
             * ワールド座標系は x が 0 以上 100 未満 y が 0 以上 50 未満
             * カメラの左上の座標は {10, 10}、スクリーンはワールド座標系に
             * x が 10 以上 50 未満 y が 10 以上 35 未満で表示されている
             * こういう場合、ワールド座標 {15, 15} はビューポート座標系に {15 - 10, 15 - 10} になる
             * 逆に、ビューポートの座標 {1, 1} は、カメラの左上の座標を加算して、ワールド座標 {1 + 10, 1 + 10} になる
             */
            [[nodiscard]] inline types::Vec2 worldCellToViewportCell(
                const types::Vec2& worldCell, const world::Camera& camera) noexcept
            {
                return {
                    worldCell.x_ - camera.getViewportOriginXAtWorld(), worldCell.y_ - camera.getViewportOriginYAtWorld() };
            }

            [[nodiscard]] inline bool isSpace(const world::TileId tileId) noexcept
            {
                return (tileId == world::tileid::kSpace) || (tileId == world::tileid::kGoal);
            }

            [[nodiscard]] inline bool isVisibleLadder(const world::TileId tileId) noexcept
            {
                return tileId == world::tileid::kVisibleLadder;
            }

            [[nodiscard]] inline bool isHiddenLadder(const world::TileId tileId) noexcept
            {
                return tileId == world::tileid::kInvisibleLadder;
            }

            [[nodiscard]] inline bool isHiddenBlock(const world::TileId tileId) noexcept
            {
                return tileId == world::tileid::kInvisibleBlackBlock;
            }
        } // namespace detail

        enum class BlockType
        {
            NotBlocking,
            Blocking,
            BlockingByHidden,
        };

        /**
         * @brief Player が Ladder と重ねる場合、登るかどうかを判定する
         * Hidden Ladder の場合は、ライトに照らされるのみ、登る可能になる
         */
        [[nodiscard]] inline bool isLadderClimbable(const world::TileMap& map, const types::Vec2& worldCell,
            const world::Camera& camera, const LightState& lightState) noexcept
        {
            const auto tileId = map.getTileIdByCell(worldCell);

            // 見える ladder はそのまま登る可能
            if (detail::isVisibleLadder(tileId))
            {
                return true;
            }

            // Hidden ladder はライトに照らされる場合、登る可能
            if (detail::isHiddenLadder(tileId))
            {
                return lightState.isLit(detail::worldCellToViewportCell(worldCell, camera));
            }

            return false;
        }

        [[nodiscard]] inline BlockType getBlockType(const world::TileMap& map, const types::Vec2& worldCell,
            const world::Camera& camera, const LightState& lightState,
            const bool bTreatClimbableLadderAsBlocking = false) noexcept
        {
            const auto tileId = map.getTileIdByCell(worldCell);

            // Space / Goal は通過可能
            if (detail::isSpace(tileId))
            {
                return BlockType::NotBlocking;
            }

            // Ladder は通常通過可能、さらに状況によって「地面」として扱う
            if (detail::isVisibleLadder(tileId) || detail::isHiddenLadder(tileId))
            {
                // ライトが OFF になると、Hidden Ladder が「登れない」のようになる
                if (bTreatClimbableLadderAsBlocking && isLadderClimbable(map, worldCell, camera, lightState))
                {
                    return BlockType::Blocking;
                }

                return BlockType::NotBlocking;
            }

            // Hidden の壁はライトで照らされた時、当たり可能のようになる
            if (detail::isHiddenBlock(tileId))
            {
                // ライトはビューポートに有効になる、ビューポート以外の Hidden の壁がそのまま NotBlocking
                if (!lightState.isLit(detail::worldCellToViewportCell(worldCell, camera)))
                {
                    return BlockType::NotBlocking;
                }

                return BlockType::BlockingByHidden;
            }
            return BlockType::Blocking;
        }

        /**
         * @brief 指定セルが通過可能のセルかどうかを判断する
         */
        [[nodiscard]] inline bool isBlockingForPlayer(const world::TileMap& map, const types::Vec2& worldCell,
            const world::Camera& camera, const LightState& lightState, const types::Vec2& playerHeadWorldCell,
            const types::Vec2& playerFeetWorldCell, const bool bTreatClimbableLadderAsBlocking = false) noexcept
        {
            const auto blockType = getBlockType(map, worldCell, camera, lightState, bTreatClimbableLadderAsBlocking);
            // 通過可能
            if (blockType == BlockType::NotBlocking)
            {
                return false;
            }

            // ブロックする
            if (blockType == BlockType::Blocking)
            {
                return true;
            }

            // Player は Hidden の壁に立っている場合、一時的にこのまま許す
            if ((worldCell.x_ == playerHeadWorldCell.x_ && worldCell.y_ == playerHeadWorldCell.y_)
                || (worldCell.x_ == playerFeetWorldCell.x_ && worldCell.y_ == playerFeetWorldCell.y_))
            {
                return false;
            }

            return true;
        }

        /**
         * @brief Enemy が指定セルに当たり判定を行う
         * Enemy は通過できるため、 Ladder は壁として扱わない
         */
        [[nodiscard]] inline bool isBlockingForEnemyOccupy(
            const world::TileMap& map,
            const types::Vec2& worldCell,
            const world::Camera& camera,
            const LightState& lightState
        ) noexcept
        {
            const world::TileId tileId = map.getTileIdByCell(worldCell);

            if (detail::isVisibleLadder(tileId) || detail::isHiddenLadder(tileId))
            {
                return false;
            }

            const BlockType blockType = getBlockType(map, worldCell, camera, lightState, false);
            return blockType != BlockType::NotBlocking;
        }

        /**
         * @brief Enemy が指定セルを床として扱うかどうかを判断する
         * Visible Ladder が常に床として扱う
         * Hidden Ladder がライトに照らされた場合、床として扱う
         */
        [[nodiscard]] inline bool isSupportingForEnemy(
            const world::TileMap& map,
            const types::Vec2& worldCell,
            const world::Camera& camera,
            const LightState& lightState
        ) noexcept
        {
            const world::TileId tileId = map.getTileIdByCell(worldCell);

            if (detail::isVisibleLadder(tileId) || detail::isHiddenLadder(tileId))
            {
                return isLadderClimbable(map, worldCell, camera, lightState);
            }

            const BlockType blockType = getBlockType(map, worldCell, camera, lightState, false);
            return blockType != BlockType::NotBlocking;
        }

        [[nodiscard]] inline bool isBlockingForEnemy(
            const world::TileMap& map,
            const types::Vec2& worldCell,
            const world::Camera& camera,
            const LightState& lightState,
            const bool bTreatClimbableLadderAsBlocking = false
        ) noexcept
        {
            const world::TileId tileId = map.getTileIdByCell(worldCell);

            if (bTreatClimbableLadderAsBlocking && (detail::isVisibleLadder(tileId) || detail::isHiddenLadder(tileId))
                && isLadderClimbable(map, worldCell, camera, lightState))
            {
                return true;
            }

            return isBlockingForEnemyOccupy(map, worldCell, camera, lightState);
        }
    } // namespace game
} // namespace core
