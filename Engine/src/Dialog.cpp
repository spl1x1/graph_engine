#include "Dialog.hpp"


void Dialog::Draw() {
    DrawRectangle(Data.PosX, Data.PosY, Data.Width, Data.Height, GRAY);
    DrawText(DialogInfo.Title.c_str(), Data.PosX + 10, Data.PosY + 10, 20, WHITE);
    DrawText(DialogInfo.Content.c_str(), Data.PosX + 10, Data.PosY + 40, 20, LIGHTGRAY);
}
