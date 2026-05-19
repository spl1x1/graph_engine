#ifndef DIALOG_HPP
#define DIALOG_HPP

#include "Widget.hpp"

struct DialogData{
    std::string Title;
    std::string Content;
};


class Dialog: public Widget{

    void Draw() override;
    bool ProccesInput() override{return false;};

    public:
    
    DialogData DialogInfo;
    
    Dialog(WidgetData data, DialogData dialogInfo): Widget(data), DialogInfo(dialogInfo){};
};

#endif // DIALOG_HPP
