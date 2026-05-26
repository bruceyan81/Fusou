// Copyright (c) 2026 Bruce Yan. All Rights Reserved.

/**
 * @brief アプリケーション実行に必要な依存関係を組み立てる API を公開する
 */

#pragma once

namespace app
{
    namespace composition
    {
        /**
         * @brief 構成ルート
         * GameState の初期化、アセットの読み込み、Adapter と Presenter の組み立てを行う
         * @return 0 は正常終了 1 は標準例外系の例外 2 はその他の例外
         */
        int run();
    } // namespace composition
} // namespace app
