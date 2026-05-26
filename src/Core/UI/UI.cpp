// Copyright (c) 2026 Bruce Yan. All Rights Reserved.

/**
 * @brief console UI text を cell buffer に描画する
 */

#include "src/Core/UI/UI.h"
#include "src/Core/Game/GameModel.h"

#include <string>

namespace core
{
    namespace ui
    {
        void drawUI(render::CellBuffer& outFrame, types::Vec2 pos, std::u32string_view text, TextAlign align,
            int customizationX)
        {
            const int textLen = static_cast<int>(text.size());

            int startX = 0;

            switch (align)
            {
                case TextAlign::Left:
                    startX = 0;
                    break;
                case TextAlign::Center:
                    // 完璧な中央揃えは保証しない
                    startX = (render::CellBuffer::kWidth - textLen) / 2;
                    break;
                case TextAlign::Right:
                    startX = render::CellBuffer::kWidth - textLen;
                    break;
                case TextAlign::Customization:
                    startX = customizationX;
            }

            outFrame.drawText({startX, pos.y_}, text);
        }

        void drawUIStyled(render::CellBuffer& outFrame, types::Vec2 pos, std::u32string_view text, render::Color fg,
            render::Color bg, TextAlign align, int customizationX)
        {
            const int textLen = static_cast<int>(text.size());

            int startX = 0;

            switch (align)
            {
                case TextAlign::Left:
                    startX = 0;
                    break;
                case TextAlign::Center:
                    // 完璧な中央揃えは保証しない
                    startX = (render::CellBuffer::kWidth - textLen) / 2;
                    break;
                case TextAlign::Right:
                    startX = render::CellBuffer::kWidth - textLen;
                    break;
                case TextAlign::Customization:
                    startX = customizationX;
                    break;
            }

            // style を上書きする
            if ((pos.y_ < 0) || (pos.y_ >= render::CellBuffer::kHeight))
            {
                return;
            }

            // glyph を書く
            outFrame.drawText({startX, pos.y_}, text);

            int curX = startX;
            for (std::size_t i = 0; i < text.size(); ++i)
            {
                if (curX >= render::CellBuffer::kWidth)
                {
                    break;
                }

                if (curX >= 0)
                {
                    auto& cell = outFrame.at(core::types::Vec2{curX, pos.y_});
                    cell.style_.fg_ = fg;
                    cell.style_.bg_ = bg;
                }

                ++curX;
            }
        }

        void appendFullWidthInt(std::u32string& out, int value)
        {
            if (value == 0)
            {
                out.push_back(U'０');
                return;
            }

            if (value < 0)
            {
                out.push_back(U'－');
                value = -value;
            }

            std::u32string reversedDigits{};

            while (value > 0)
            {
                const int digit = value % 10;
                reversedDigits.push_back(static_cast<char32_t>(U'０' + digit));
                value /= 10;
            }

            for (auto it = reversedDigits.rbegin(); it != reversedDigits.rend(); ++it)
            {
                out.push_back(*it);
            }
        }

        /**
         * @brief Cutscene にプリンター効果で文字を表示する
         * @param shownCharCount 現時点表示されている文字数
         */
        void drawCutsceneText(
            render::CellBuffer& outFrame, int startY, int shownCharCount, TextAlign align, int customizationX)
        {
            // drawCutsceneText を呼び出す 1 回に、表示する文字数
            int remainingChars = shownCharCount;

            // 1 行目ずつ表示する
            for (std::size_t i = 0; i < str::kCutsceneText.size(); ++i)
            {
                const std::u32string_view fullLine = str::kCutsceneText[i];

                if (remainingChars <= 0)
                {
                    break;
                }

                // 次の表示する文字数はこの行の最大文字数以内
                const int nextShowCharCount = remainingChars > static_cast<int>(fullLine.size())
                    ? static_cast<int>(fullLine.size())
                    : remainingChars;

                const std::u32string_view showText = fullLine.substr(0, nextShowCharCount);

                drawUI(outFrame, core::types::Vec2{0, startY + static_cast<int>(i)}, showText, align, customizationX);

                remainingChars -= nextShowCharCount;
            }
        }
    } // namespace ui
} // namespace core