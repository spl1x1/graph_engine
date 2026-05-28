#include <TextArea.hpp>

void TextArea::Draw() {
    const auto color{Data.Border.Color};
    DrawBorder(color);
    DrawText(Text.c_str(), Data.PosX + 10, Data.PosY + 10, 20, WHITE);
}
