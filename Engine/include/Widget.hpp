#ifndef WIDGET_HPP
#define WIDGET_HPP

#include <queue>
#include <string>
#include <unordered_map>
#include <vector>
#include <raylib.h>

struct BorderData{
    float Thickness;
    Color Color;
    bool Enabled{true};
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
    static std::queue<std::string> inputBlockingWidgets;

protected:
    bool isVisible{false};
    bool CheckCollision(Vector2 point) const;

    void DrawBorder(Color color) const;
    static void ConstructRelativePosition(const std::string& key, WidgetData rootData);

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

    // Queue a widget to block input for multiple frames, only used for widget that need to block input for multiple frames, like text input
    static void QueueInputWidget(const std::string& widgetKey);

    static bool ProccessInputs();
    // Proccess input for all widgets in the queue, only used for widget that need to block input for multiple frames, like text input
    static void ProccessInputWidgets();
    static void EndDrawing();
};


#endif // WIDGET_HPP
