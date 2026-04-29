#ifndef ENGINE_HPP
#define ENGINE_HPP

#include "EngineTypes.hpp"

#include <memory>
#include <raylib.h>


class Engine{
    // Singleton instance
    static std::unique_ptr<Engine> instance;

    Texture2D backgroundTexture;

    //Pointer to enviroment struct, passed at initialization
    Enviroment *env;
    SandboxData *sandboxData;

    void DrawBackground();
    void DrawNode();
    void DrawEdge();
    void DrawGrid();
    void DrawUI();
    void MoveCamera(Vec2& LastMousePosition);

    static void DrawSandbox();
    static void ProcessInput();

    // Private constructor to prevent direct instantiation
    Engine(Enviroment *env, SandboxData *sandboxData);

    // Delete copy and move constructors and assignment operators
    Engine(const Engine&) = delete;
    Engine& operator=(const Engine&) = delete;
    Engine(Engine&&) = delete;
    Engine& operator=(Engine&&) = delete;

public:
    // Static method to initialize the engine
    static void Init(Enviroment *env, SandboxData *sandboxData);
    // Static method to start the main loop
    static void Loop();
    static void LoadBackground(Background background);
};

#endif // ENGINE_HPP
