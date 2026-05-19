#ifndef WIDGET_HPP
#define WIDGET_HPP

#include <string>
#include <unordered_map>
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
    static bool ProccessInputs();
    static void Draw(const std::string& key);
    static void EndDrawing();
    static void Destroy(const std::string& key);

    static Widget* GetWidget(const std::string& key);

};


#endif // WIDGET_HPP
