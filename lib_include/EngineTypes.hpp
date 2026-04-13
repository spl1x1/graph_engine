#ifndef ENGINE_TYPES_HPP
#define ENGINE_TYPES_HPP

#include <string>
struct EnvWindow{
    int Width;
    int Height;
    int FrameRate;
};
struct EnvSandbox{
    int Width;
    int Height;
};


struct Enviroment{
    EnvWindow Window;
    EnvSandbox Sandbox;
    std::string Title;
};

#endif // ENGINE_TYPES_HPP
