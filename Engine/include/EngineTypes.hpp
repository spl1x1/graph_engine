#ifndef ENGINE_TYPES_HPP
#define ENGINE_TYPES_HPP

#include <string>
#include <sys/types.h>
#include "Vector.hpp"

using Vec2 = Vec<float, 2>;

Vec2 operator+(Vec2 vec, Vector2 RaylibVec2);
Vec2 operator-(Vec2 vec, Vector2 RaylibVec2);
bool operator==(Vec2 vec, Vector2 RaylibVec2);
Vec2 ConvertRayVec2(Vector2 RaylibVec2);


struct EnvWindow{
    uint Width;
    uint Height;
    uint FrameRate;
};
struct EnvSandbox{
    const uint Width;
    const uint Height;
    const uint CellSize;
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


struct SandboxData{
    Vec2 Camera;
    double Zoom;
};

#endif // ENGINE_TYPES_HPP
