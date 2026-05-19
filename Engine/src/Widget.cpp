#include "Widget.hpp"
#include <iostream>
#include <ranges>

std::unordered_map<std::string, std::unique_ptr<Widget>> Widget::widgetPool{};

void Widget::Register(const std::string& key, std::unique_ptr<Widget> widget){
    widgetPool.insert_or_assign(key, std::move(widget));
}

void Widget::Draw(const std::string& key){
    if (auto it = widgetPool.find(key); it != widgetPool.end())
    {
        it->second->isVisible = true;
        it->second->Draw();
    }
    else
        std::cout<<"[Draw] Couldn't find widget key: " << key << "\n";
}

void Widget::EndDrawing(){
    for (auto &widget : widgetPool | std::views::values)
        widget->isVisible = false;
}

void Widget::Destroy(const std::string& key){
    if (auto it = widgetPool.find(key); it != widgetPool.end())
        widgetPool.erase(it);
    else
        std::cout<<"[Destroy] Couldn't find widget key: " << key << "\n";
}

Widget* Widget::GetWidget(const std::string &key){
    if (auto it = widgetPool.find(key); it != widgetPool.end())
        return it->second.get();
    std::cout<<"[GetWidget] Couldn't find widget key: " << key << "\n";
    return nullptr;
}

bool Widget::ProccessInputs(){
    bool shouldBlockInput{false};
    for (auto &widget : widgetPool | std::views::values)
        if(!widget->isVisible) continue;
        else
        if(widget->ProccesInput()) shouldBlockInput = true;
    return shouldBlockInput;
}

bool Widget::CheckCollision(Vector2 point) const {
    return point.x >= Data.PosX && point.x <= Data.PosX + Data.Width &&
           point.y >= Data.PosY && point.y <= Data.PosY + Data.Height;
}
