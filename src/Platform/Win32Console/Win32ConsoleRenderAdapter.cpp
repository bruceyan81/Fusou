// Copyright (c) 2026 Bruce Yan. All Rights Reserved.

/**
 * @brief cell buffer を Win32 console screen buffer へ反映する
 */

#include "src/Platform/Win32Console/Win32ConsoleRenderAdapter.h"
#include "src/Platform/Win32Console/Win32ConsoleApi.h"

#include <string>

namespace platform
{
    namespace win32console
    {
        void Win32ConsoleRenderAdapter::clear()
        {
            clearScreen();

            preFrame_.fill(core::render::Cell{});

            cacheAttr_ = 0x0;

            bHasPrevious_ = true;
        }

        void Win32ConsoleRenderAdapter::flush()
        {
            flushStdout();
        }

        void Win32ConsoleRenderAdapter::setCursorVisible(bool bVisible)
        {
            setCursor(bVisible ? CursorMode::Normal : CursorMode::Hidden);
        }

        void Win32ConsoleRenderAdapter::present(const core::render::CellBuffer& frame)
        {
            // 初回のみ、前フレームが存在していないため
            if (!bHasPrevious_)
            {
                clear();
            }

            std::u32string text;

            text.reserve(static_cast<std::size_t>(core::render::CellBuffer::kWidth));

            for (int y = 0; y < core::render::CellBuffer::kHeight; ++y)
            {
                // 最初から、変更しないセルをスキップするため、while を使う
                int x = 0;
                while (x < core::render::CellBuffer::kWidth)
                {
                    const core::types::Vec2 bufferCell{x, y};

                    core::render::TextStyle style{};

                    {
                        const auto& nextCell = frame.at(bufferCell);
                        const auto& preCell = preFrame_.at(bufferCell);

                        // セルが完全一致の場合、スキップ
                        if (nextCell == preCell)
                        {
                            ++x;
                            continue;
                        }
                        style = nextCell.style_;
                    }

                    // これから、変更する必要なセル
                    {
                        const int startX = x;
                        text.clear();

                        while (x < core::render::CellBuffer::kWidth)
                        {
                            const core::types::Vec2 currentBufferCell{x, y};

                            const auto& nextCell = frame.at(currentBufferCell);
                            const auto& preCell = preFrame_.at(currentBufferCell);

                            if (nextCell == preCell)
                            {
                                break;
                            }

                            if ((nextCell.style_.fg_ != style.fg_) || (nextCell.style_.bg_ != style.bg_))
                            {
                                break;
                            }

                            text.push_back(nextCell.glyph_);

                            ++x;
                        }

                        moveCursorTo(core::types::Vec2{startX, y});

                        const auto attr = createTextAttr(style.fg_, style.bg_);

                        if (attr != cacheAttr_)
                        {
                            setTextAttr(attr);
                            cacheAttr_ = attr;
                        }

                        write(std::u32string_view(text));
                    }
                }
            }
            preFrame_ = frame;
            bHasPrevious_ = true;
        }
    } // namespace win32console
} // namespace platform
