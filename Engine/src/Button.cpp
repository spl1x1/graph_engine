#include "Button.hpp"
#include "raylib.h"

std::unordered_map<std::string, Button> Button::buttonPool;

Button::Button(const std::string& key, const ButtonData data): data(data) {
    buttonPool.insert_or_assign(key, *this);
}

void Button::Destroy(const std::string& key) {
    buttonPool.erase(key);
}

void Button::Draw() {
    const auto color {IsActive ? data.ActiveColor : IsHovered ? data.HoverColor : data.IdleColor};
    const auto textWidth {MeasureText(data.Text.c_str(), 20)};
    const auto textX {data.PosX + (data.Width - textWidth) / 2};
    const auto textY {data.PosY + (data.Height - 20) / 2};

    if (data.InvertedColors) {
        DrawRectangle(data.PosX, data.PosY, data.Width, data.Height, color);
        DrawText(data.Text.c_str(), textX, textY, 20, BLACK);
        return;
    }
    DrawRectangleLines(data.PosX, data.PosY, data.Width, data.Height, color);
    DrawText(data.Text.c_str(), textX, textY, 20, color);
}

Button Button::GetButton(const std::string& key) {
    return buttonPool.at(key);
}


bool Button::ProccessInputs() {
    auto CheckCollision = [](Vector2 point, Rectangle rect) {
        return point.x >= rect.x && point.x <= rect.x + rect.width &&
               point.y >= rect.y && point.y <= rect.y + rect.height;
    };

    auto shouldBlockInput{false};
    const auto mousePos{GetMousePosition()};
    for (auto& [key, button] : buttonPool) {
        button.IsHovered = CheckCollision(mousePos,
            Rectangle{button.data.PosX, button.data.PosY, button.data.Width, button.data.Height});

        if (button.IsHovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            button.IsActive = true;
            button.data.OnClick();
        } else if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
            button.IsActive = false;
        }
        if (button.IsHovered || button.IsActive) shouldBlockInput = true;
    }
    return shouldBlockInput;
}

void Button::Draw(const std::string& key) {
    const auto it = buttonPool.find(key);
    if (it != buttonPool.end()) {
        it->second.Draw();
    }
}
