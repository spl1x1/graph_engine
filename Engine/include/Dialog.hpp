#ifndef DIALOG_HPP
#define DIALOG_HPP

#include <Widget.hpp>
#include <string>

struct DialogData{
    std::string Title;
    std::vector<std::string>  WidgetKeys;
    bool RelativePositions{true};
    //Positions are relative to root element
};


class Dialog: public Widget{

    void Draw() override;
    bool ProccesInput() override{return false;};

    public:

    DialogData DialogInfo;

    Dialog(WidgetData data, DialogData dialogInfo);
    //By default element positions are relative to dialog position
};

#endif // DIALOG_HPP
