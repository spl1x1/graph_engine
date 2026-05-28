#include <TextInput.hpp>
#include <Button.hpp>


TextInput::TextInput(const WidgetData& data, std::string placeHolderText) : Widget(data), InputData{
    .PlaceHolderText = placeHolderText
} {
    //Vytvořit 2 tlačítka pro submit a close, které budou součástí widgetu
    // Placeholder text implementace
}
