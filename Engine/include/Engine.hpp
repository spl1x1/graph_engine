#ifndef ENGINE_HPP
#define ENGINE_HPP

#include "EngineTypes.hpp"

#include <functional>
#include <memory>
#include <raylib.h>
#include <unordered_map>


class Engine{
    // Singleton instance
    static std::unique_ptr<Engine> instance;

    Texture2D backgroundTexture;

    //Pointer to enviroment struct, passed at initialization
    Enviroment *env;
    SandboxData *sandboxData;
    Nodes nodes;

    std::unordered_map<std::string, std::function<std::unique_ptr<NodeAbstract>(Vec2 position)>> NodeFactory;

    void DrawBackground();
    void DrawEdge();
    void DrawGrid();
    void DrawUI();
    void MoveCamera(const Vec2 LastMousePosition);
    void DrawNode(const NodeAbstract& node);
    void ProccesCameraMovement();

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
    static void LoadBackground(const Background background);
    static void RegisterNodeType(const std::string& typeName, std::function<std::unique_ptr<NodeAbstract>(Vec2 position)> factoryFunction);
};

#endif // ENGINE_HPP
