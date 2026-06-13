#include <Widget.hpp>
#include <Dialog.hpp>

Dialog::Dialog(WidgetData data, DialogData dialogInfo): Widget(data), DialogInfo(dialogInfo){
    for (const auto& element : DialogInfo.WidgetKeys) ConstructRelativePosition(element,data);
}

void Dialog::Draw() {
    const auto color {Data.Border.BorderColor};
    DrawBorder(color);
    DrawText(DialogInfo.Title.c_str(), Data.PosX + 10, Data.PosY + 10, 20, color);

    for (const auto& element : DialogInfo.WidgetKeys) Widget::Draw(element);
}
