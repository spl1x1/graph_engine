#include "Button.hpp"
#include "raylib.h"


void Button::Draw() {
    const auto color {IsActive ? ButtonInfo.ActiveColor : IsHovered ? ButtonInfo.HoverColor : ButtonInfo.IdleColor};
    const auto textWidth {MeasureText(ButtonInfo.Text.c_str(), 20)};
    const auto textX {Data.PosX + (Data.Width - textWidth) / 2};
    const auto textY {Data.PosY + (Data.Height - 20) / 2};

    if (ButtonInfo.InvertedColors) {
        DrawRectangle(Data.PosX, Data.PosY, Data.Width, Data.Height, color);
        DrawText(ButtonInfo.Text.c_str(), textX, textY, 20, BLACK);
        return;
    }
    DrawRectangleLines(Data.PosX, Data.PosY, Data.Width, Data.Height, color);
    DrawText(ButtonInfo.Text.c_str(), textX, textY, 20, color);
}

bool Button::ProccesInput() {
    IsHovered = CheckCollision(GetMousePosition());
    if (!IsHovered) return false;

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        IsActive = true;
        ButtonInfo.OnClick();
    } else if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
        IsActive = false;
    return true;
}
