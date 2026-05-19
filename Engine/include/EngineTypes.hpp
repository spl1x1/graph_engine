#ifndef ENGINE_TYPES_HPP
#define ENGINE_TYPES_HPP

#include <string>
#include <sys/types.h>
#include "Vector.hpp"
#include "raylib.h"

Vec2 operator+(Vec2 vec, Vector2 RaylibVec2);
Vec2 operator-(Vec2 vec, Vector2 RaylibVec2);
bool operator==(Vec2 vec, Vector2 RaylibVec2);
Vec2 ConvertRayVec2(Vector2 RaylibVec2);
float GetDistance(Vec2 a, Vec2 b);


struct EnvWindow{
    uint Width;
    uint Height;
    uint FrameRate;
};
struct EnvSandbox{
    const uint Width;
    const uint Height;
};

struct Background{
    std::string TexturePath;
    // Optional width and height for the background texture, if not provided, it will be loaded with its original size
    uint Width{0};
    // Optional width and height for the background texture, if not provided, it will be loaded with its original size
    uint Height{0};

    bool operator==(const Background& other);
};

struct Enviroment{
    Background Background;
    EnvWindow Window;
    EnvSandbox Sandbox;

    std::string Title;
    float DeltaTime;
};

struct EditMode{
    bool Enabled;
    std::string SelectedMode;
};

struct InputBlock{
    bool Blocked;
    int BlockLoop{0};
    enum class BlockType{
        None,
        Button,
        Camera,
        Node
    } Type;
};


struct SandboxData{
    Vec2 Camera;
    float Zoom;
    EditMode Edit;
};
#endif // ENGINE_TYPES_HPP
