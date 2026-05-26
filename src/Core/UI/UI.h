// Copyright (c) 2026 Bruce Yan. All Rights Reserved.

/**
 * @brief console UI text 描画 API を公開する
 */

#pragma once

#include "src/Core/Render/CellBuffer.h"
#include "src/Core/Types/Types.h"

#include <array>
#include <string>
#include <string_view>

namespace core
{
    namespace ui
    {
        namespace str
        {
            // Intro
            inline constexpr std::u32string_view kIntroHint1 = U"Ｐｒｅｓｓ　Ｅｎｔｅｒ　Ｋｅｙ";

            // Cutscene
            inline constexpr std::u32string_view kCutSceneHint1 =
                U"Ｐｒｅｓｓ　ａｎｙ　ｋｅｙ　ｔｏ　ｓｋｉｐ　ｃｕｔｓｃｅｎｅ";
            inline constexpr std::array<std::u32string_view, 19> kCutsceneText{
                U"東の海に、湯谷（とうこく）と呼ばれる水域がある", U"　", U"そのほとりに立つ神木——扶桑（ふそう）",
                U"　", U"古い物語では、十の太陽がその枝に留まり", U"　", U"九つは下枝に、ひとつは上枝に", U"　",
                U"順に目覚め、天を渡ったという", U"　", U"かつて中国では、日本を「扶桑」と呼んだとも伝えられる", U"　",
                U"それは、東の果てに立つその樹の名を借りた——詩の呼び名", U"　",
                U"同じ二文字が、神話となり、国の名となり、歌の余韻となって残った", U"　",
                U"湯谷の波は今日も静かに揺れている", U"　", U"忘れられた光を映すように、ただ——遠い記憶をたたえながら"};
        } // namespace str

        enum class TextAlign
        {
            Left,
            Center,
            Right,
            Customization,
        };

        void drawUI(render::CellBuffer& outFrame, types::Vec2 pos, std::u32string_view text,
            TextAlign align = TextAlign::Left, int customizationX = 0);

        void drawUIStyled(render::CellBuffer& outFrame, types::Vec2 pos, std::u32string_view text, render::Color fg,
            render::Color bg, TextAlign align = TextAlign::Left, int customizationX = 0);

        void appendFullWidthInt(std::u32string& out, int value);

        void drawCutsceneText(render::CellBuffer& outFrame, int startY, int shownCharCount,
            TextAlign align = TextAlign::Center, int customizationX = 0);
    } // namespace ui
} // namespace core
