#ifndef WIDGET_HPP
#define WIDGET_HPP

#include <string>
#include <unordered_map>
#include <vector>
#include "raylib.h"

struct BorderData{
    float Thickness;
    Color Color;
};

struct WidgetData{
    float PosX;
    float PosY;
    float Width;
    float Height;

    BorderData Border;
};

class Widget{
    static std::unordered_map<std::string, std::unique_ptr<Widget>> widgetPool;
    static std::unordered_map<std::string, std::vector<std::string>> widgetGroups;
protected:
    bool isVisible{false};
    bool CheckCollision(Vector2 point) const;

    virtual void Draw() = 0;
    virtual bool ProccesInput() = 0;
public:
    WidgetData Data;

    virtual ~Widget() = default;
    Widget(WidgetData data): Data(data){};

    static void Register(const std::string& key, std::unique_ptr<Widget> widget);
    static void Destroy(const std::string& key);
    static void Draw(const std::string& key);
    static Widget* GetWidget(const std::string& key);

    static void AddToGroup(const std::string& groupKey, const std::string& widgetKey);
    static void AddToGroup(const std::string& groupKey, std::vector<std::string> widgets);
    static void DrawGroup(const std::string& groupKey);
    static void DestroyGroup(const std::string& groupKey);
    static void RemoveFromGroup(const std::string& groupKey, const std::string& widgetKey);

    static bool ProccessInputs();
    static void EndDrawing();
};


#endif // WIDGET_HPP
