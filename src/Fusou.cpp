// Copyright (c) 2026 Bruce Yan. All Rights Reserved.

/**
 * @brief Fusou アプリケーションの起動処理を実行する
 */

#include "Fusou.h"
#include "src/App/Composition/AppComposition.h"

#include <exception>
#include <iostream>

namespace fusou
{
    int runFusou()
    {
        try
        {
            return app::composition::run();
        }
        catch (const std::exception& e)
        {
            std::cerr << "Error from Fusou.cpp.\n";
            std::cerr << e.what();
            return 1;
        }
    }
} // namespace fusou
