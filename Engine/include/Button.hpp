#ifndef BUTTON_HPP
#define BUTTON_HPP

#include <Widget.hpp>
#include <string>
#include <functional>

struct ButtonData{
    Color IdleColor{WHITE};
    Color HoverColor{YELLOW};
    Color ActiveColor{GREEN};

    std::string Text;
    std::function<void()> OnClick = [](){};
};


class Button : public Widget{

    bool IsActive{false};
    bool IsHovered{false};

    void Draw() override;
    bool ProccesInput() override;

 public:
    ButtonData ButtonInfo;

    Button(WidgetData data, ButtonData buttonInfo): Widget(data), ButtonInfo(buttonInfo){};
};
#endif // BUTTON_HPP
