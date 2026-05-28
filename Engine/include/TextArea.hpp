#ifndef TEXTAREA_HPP
#define TEXTAREA_HPP

#include <Widget.hpp>
#include <string>


class TextArea: public Widget{
    void Draw() override;
    bool ProccesInput() override{return false;};

    public:

    std::string Text;

    TextArea(WidgetData data, std::string text): Widget(data), Text(text){};
};


#endif // TEXTAREA_HPP
