#ifndef TEXT_INPUT_HPP
#define TEXT_INPUT_HPP

#include <Widget.hpp>

struct TextInputData : public WidgetData{
    bool Active{false};

    std::string PlaceHolderText;
    std::string Buffer;

    std::string closeButtonKey;
    std::string submitButtonKey;
};

class TextInput : public Widget{

    TextInputData InputData;

    void Draw() override;
    bool ProccesInput() override;

public:
    TextInput(const WidgetData& data, std::string placeHolderText);
    ~TextInput();



};


#endif // TEXT_INPUT_HPP
