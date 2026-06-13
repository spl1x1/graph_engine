#include "Widget.hpp"
#include "raylib.h"
#include <algorithm>
#include <iostream>
#include <ranges>
#include <vector>

std::unordered_map<std::string, std::unique_ptr<Widget>> Widget::widgetPool{};
std::unordered_map<std::string, std::vector<std::string>> Widget::widgetGroups{};
std::queue<std::string> Widget::inputBlockingWidgets{};

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

void Widget::AddToGroup(const std::string& groupKey, const std::string& widgetKey){
    if (!widgetPool.contains(widgetKey)) return;
    if (!widgetGroups.contains(groupKey)){
        widgetGroups.insert_or_assign(groupKey,std::vector<std::string>{widgetKey});
        return;
    };
    widgetGroups.at(groupKey).push_back(widgetKey);
}

void Widget::DrawGroup(const std::string& groupKey){
    if (!widgetGroups.contains(groupKey)) return;
    for (auto widget : widgetGroups.at(groupKey)) Draw(widget);
}

void Widget::DestroyGroup(const std::string& groupKey){
    if (!widgetGroups.contains(groupKey)) return;
    widgetGroups.erase(groupKey);
}

void Widget::RemoveFromGroup(const std::string& groupKey, const std::string& widgetKey){
    if (!widgetGroups.contains(groupKey)) return;
    auto &group = widgetGroups.at(groupKey);
    group.erase(std::remove(group.begin(), group.end(), widgetKey), group.end());
}

void Widget::AddToGroup(const std::string &groupKey,  const std::vector<std::string> widgets) {
    if (!widgetGroups.contains(groupKey)){
        widgetGroups.insert_or_assign(groupKey,widgets);
        return;
    };
    for (const auto& widgetKey: widgets) AddToGroup(groupKey, widgetKey);
}

void Widget::DrawBorder(Color color) const {
    if (Data.Border.Enabled)
        DrawRectangleLines(Data.PosX, Data.PosY, Data.Width, Data.Height, color);
}

void Widget::ConstructRelativePosition(const std::string& key, WidgetData rootData){
    const auto widget{Widget::GetWidget(key)};
    if (!widget) return;
    widget->Data.PosX += rootData.PosX;
    widget->Data.PosY += rootData.PosY;
}

void Widget::ProccessInputWidgets(){
    while (!inputBlockingWidgets.empty()) {
        auto widgetKey = inputBlockingWidgets.front();
        if (auto widget = Widget::GetWidget(widgetKey); widget != nullptr)
            widget->ProccesInput();
        inputBlockingWidgets.pop();
    }
}

void Widget::QueueInputWidget(const std::string& widgetKey){
    if (!widgetPool.contains(widgetKey)) return;
    inputBlockingWidgets.push(widgetKey);
}
