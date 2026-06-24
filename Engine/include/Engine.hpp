#ifndef ENGINE_HPP
#define ENGINE_HPP

#include <EngineTypes.hpp>
#include <Node.hpp>

#include <functional>
#include <memory>
#include <optional>
#include <raylib.h>
#include <unordered_map>


class Engine{
    // Singleton instance
    static std::unique_ptr<Engine> instance;

    Texture2D backgroundTexture;

    //Pointer to enviroment struct, passed at initialization
    Enviroment *env;
    SandboxData *sandboxData;
    NodeNetwork nodes;

    std::unordered_map<std::string, std::function<std::unique_ptr<INode>(Vec2 position)>> NodeFactory;
    std::vector<std::function<void()>> UpdateFunctions;

    InputBlock inputBlock;

    void DrawBackground();
    void DrawEdge(Edge& edge);
    void DrawNode(INode& node);
    void DrawUI();

    void DrawNodes();
    void DrawEdges();

    void ResetInputBlock();

    void ProcessCameraMovement();
    void ProcessButtons();
    void ProcessEditInputs();
    void ProcessNodeClick();
    void ProcessKeyboard();
    void ProcessTextInput();

    void ZoomCamera();
    void MoveCamera(const Vec2 LastMousePosition);

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
    static void InitSave(const std::string& saveFile = "");
    // Static method to start the main loop
    static void Loop();
    static void LoadBackground(const Background background);
    static void RegisterNodeType(const std::string& typeName, std::function<std::unique_ptr<INode>(Vec2 position)> factoryFunction);
    static void RegisterUpdateFunction(std::function<void()> updateFunction);
    static INode* GetSelectedNode();
    static void ClearSelectedNode();
    static std::vector<INode*> GetAllNodes();
    static std::optional<SelectedNodes> ConsumeMessageSelection();
    static void FlashEdgeBetween(uint16_t nodeA, uint16_t nodeB, float durationSeconds = 0.3f);
};

#endif // ENGINE_HPP
