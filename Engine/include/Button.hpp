#ifndef BUTTON_HPP
#define BUTTON_HPP

#include <unordered_map>
#include <string>
#include <functional>
#include <raylib.h>

class Button{
    static std::unordered_map<std::string, Button> buttonPool;

    bool IsActive{false};
    bool IsHovered{false};

 public:
    struct ButtonData{
        float PosX;
        float PosY;
        float Width;
        float Height;

        bool InvertedColors{false};
        Color IdleColor{WHITE};
        Color HoverColor{YELLOW};
        Color ActiveColor{GREEN};

        std::string Text;
        std::function<void()> OnClick = [](){};
    } data;

    // Delete default constructor to prevent creating buttons without data
    Button() = delete;

    Button(const Button&) = default;
    Button& operator=(const Button&) = default;

    Button(const std::string& key,const ButtonData data);
    ~Button() = default;

    void Draw();

    static bool ProccessInputs();
    static void Draw(const std::string& key);
    static void Destroy(const std::string& key);

    static Button GetButton(const std::string& key);
};
#endif // BUTTON_HPP
