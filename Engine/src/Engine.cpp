#include <cassert>
#include <cmath>
#include <iostream>
#include <ranges>
#include <string>
#include "Engine.hpp"
#include "EngineTypes.hpp"
#include "Node.hpp"
#include "Widget.hpp"
#include "raylib.h"
#include "Button.hpp"


std::unique_ptr<Engine> Engine::instance = nullptr;

Engine::Engine(Enviroment *env, SandboxData *sandboxData) : env(env), sandboxData(sandboxData) {
    // Initialize raylib
    InitWindow(env->Window.Width, env->Window.Height, env->Title.c_str());
    SetTargetFPS(env->Window.FrameRate);

}

void Engine::Init(Enviroment *env, SandboxData *sandboxData) {

    const auto WidgetDataTemplate = [] (const float PosX, const float PosY) -> WidgetData {
        return WidgetData{
            .PosX = PosX,
            .PosY = PosY,
            .Width = 150,
            .Height = 20,
            .Border = {0.0f, WHITE}
        };
    };

    const auto ButtonDataTemplate = [] (const std::string& Text, std::function<void()> OnClick) -> ButtonData {
        return ButtonData{
            .HoverColor = LIGHTGRAY,
            .Text = Text,
            .OnClick = OnClick
        };
    };

    [[unlikely]]
    if (instance != nullptr) {
        std::cerr << "Engine already initialized!" << "\n";
        return;
    }

    instance = std::unique_ptr<Engine>(new Engine(env, sandboxData));
    LoadBackground(env->Background);
    std::cout << "Engine initialized" << "\n";


    // Register UI widgets
    const auto ClearWidgetData {WidgetDataTemplate(10, 130)};
    const auto ClearButtonData {ButtonDataTemplate("Clear", [](){
        instance->nodes.ClearSelectedNodes();
        instance->sandboxData->Edit.SelectedMode.clear();
    })};

    const auto RemoveWidgetData = WidgetDataTemplate(10, 160);
    const auto RemoveButtonData = ButtonDataTemplate("Remove Node", [](){
        instance->sandboxData->Edit.SelectedMode = "REMOVE";
    });

    const auto AddEdgeWidgetData {WidgetDataTemplate(170, 160)};
    const auto AddEdgeButtonData {ButtonDataTemplate("Add Edge", [](){
        instance->sandboxData->Edit.SelectedMode = "ADD_EDGE";
    })};

    Widget::Register("ClearButton", std::make_unique<Button>(ClearWidgetData, ClearButtonData));
    Widget::Register("AddEdgeButton", std::make_unique<Button>(AddEdgeWidgetData, AddEdgeButtonData));
    Widget::Register("RemoveNodeButton", std::make_unique<Button>(RemoveWidgetData, RemoveButtonData));
}

void Engine::Loop() {
    assert(instance != nullptr && "Engine must be initialized before calling Loop");
    while (!WindowShouldClose()) {
        ProcessInput();
        Widget::EndDrawing();
        BeginDrawing();
        DrawSandbox();
        ClearBackground(RAYWHITE);
        EndDrawing();
        instance->env->DeltaTime = GetFrameTime();
    }
    std::cout << "Engine loop ended" << "\n";
}

void Engine::LoadBackground(Background background) {
    if (instance == nullptr) return;

    if (instance->backgroundTexture.id != 0) {
        UnloadTexture(instance->backgroundTexture);
        instance->backgroundTexture = {0, 0, 0, 0};
    }

    instance->backgroundTexture = LoadTexture(background.TexturePath.c_str());

    if (background.Height ==0 && background.Width ==0)
    {
        background.Width = instance->backgroundTexture.width;
        background.Height = instance->backgroundTexture.height;
    }

    instance->env->Background = background;

    std::cout << "Loading background texture... \n";
}

void Engine::DrawBackground() {
    if (instance->backgroundTexture.id == 0) {
        DrawRectangle(0, 0, instance->env->Window.Width, instance->env->Window.Height, BLUE);
        return;
    }

    const float zoom = instance->sandboxData->Zoom;
    const float tileW = static_cast<float>(instance->env->Background.Width) * zoom;
    const float tileH = static_cast<float>(instance->env->Background.Height) * zoom;

    float offsetX = std::fmod(instance->sandboxData->Camera[0] * zoom, tileW);
    float offsetY = std::fmod(instance->sandboxData->Camera[1] * zoom, tileH);

    if (offsetX < 0) offsetX += tileW;
    if (offsetY < 0) offsetY += tileH;

    Vec2 FrameCorner{
        -offsetX,
        -offsetY,
    };

    const Rectangle sourceRect{0.0f, 0.0f, static_cast<float>(instance->backgroundTexture.width), static_cast<float>(instance->backgroundTexture.height)};
    const Vector2 origin{0.0f, 0.0f};

    for (float y = FrameCorner[1]; y < instance->env->Window.Height; y += tileH) {
        for (float x = FrameCorner[0]; x < instance->env->Window.Width; x += tileW) {
            const Rectangle destRect{x, y, tileW, tileH};
            DrawTexturePro(instance->backgroundTexture, sourceRect, destRect, origin, 0.0f, WHITE);
        }
    }
}

void Engine::DrawNodes() {
    for (const auto& node: nodes.GetNodeMap() | std::views::values) {
        if (node) instance->DrawNode(*node);
    }
}

void Engine::DrawEdges() {
    for (auto& edge: nodes.GetEdgeMap() | std::views::values) {
        instance->DrawEdge(edge);
    }
}

void Engine::DrawSandbox() {
    auto engine{Engine::instance.get()};
    engine->DrawBackground();
    engine->DrawNodes();
    engine->DrawEdges();
    engine->DrawUI();
}

bool Engine::ProcessCameraMovement(){
    //Mouse dragging for camera movement
    constexpr float MouseHoldThreshold = 0.2f; // Time in seconds to trigger camera movement
    auto engine{Engine::instance.get()};
    auto mouseDown {IsMouseButtonDown(MOUSE_RIGHT_BUTTON)};
    static float MouseDownTime{0.0f};
    static Vec2 LastMousePosition{GetMouseX(), GetMouseY()};

    if (mouseDown) MouseDownTime += engine->env->DeltaTime;
    else MouseDownTime = 0.0f;
    if (MouseDownTime >= MouseHoldThreshold) instance->MoveCamera(LastMousePosition);

    LastMousePosition[0] = GetMouseX();
    LastMousePosition[1] = GetMouseY();
    return (MouseDownTime >= MouseHoldThreshold);
}

void Engine::ProcessButtons() {
    instance->inputBlock = {
        .Blocked = Button::ProccessInputs(),
        .Type = InputBlock::BlockType::Button
    };
}

void Engine::ProcessEditInputs() {
    const auto AddNode = [&](){
        Vec2 mousePos{GetMousePosition().x, GetMousePosition().y};
        const auto zoom {instance->sandboxData->Zoom};
        Vec2 worldPos{mousePos[0] / zoom + instance->sandboxData->Camera[0], mousePos[1] / zoom + instance->sandboxData->Camera[1]};

        auto node = instance->NodeFactory.at(instance->sandboxData->Edit.SelectedMode)(worldPos);
        instance->nodes.AddNode(std::move(node));
     };

    if (!IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) return;

    const auto mode = instance->sandboxData->Edit.SelectedMode;
    if (mode == "ADD_EDGE")
        std::cout << "Processing add edge input... \n";
    else if (instance->NodeFactory.contains(instance->sandboxData->Edit.SelectedMode)) AddNode();
}

void Engine::ZoomCamera() {
    constexpr float ZoomSensitivity{0.05f}; // Adjust zoom sensitivity here
    constexpr float MinZoom{0.5f}; // Minimum zoom level
    constexpr float MaxZoom{2.0f}; // Maximum zoom level

    const auto zoomAmount{GetMouseWheelMove() * ZoomSensitivity};
    const auto oldZoom {instance->sandboxData->Zoom};
    const auto newZoom {oldZoom + zoomAmount};

    if (newZoom > MinZoom && newZoom < MaxZoom) {
        const auto mouse {GetMousePosition()};
        const auto worldX {instance->sandboxData->Camera[0] + mouse.x / oldZoom};
        const auto worldY {instance->sandboxData->Camera[1] + mouse.y / oldZoom};

        instance->sandboxData->Zoom = newZoom;
        instance->sandboxData->Camera[0] = worldX - mouse.x / newZoom;
        instance->sandboxData->Camera[1] = worldY - mouse.y / newZoom;

        const auto viewW{static_cast<float>(instance->env->Window.Width) / newZoom};
        const auto viewH {static_cast<float>(instance->env->Window.Height) / newZoom};
        const auto BoundX {std::max(0.0f, instance->env->Sandbox.Width - viewW)};
        const auto BoundY {std::max(0.0f, instance->env->Sandbox.Height - viewH)};

        if (instance->sandboxData->Camera[0] < 0) instance->sandboxData->Camera[0] = 0;
        if (instance->sandboxData->Camera[1] < 0) instance->sandboxData->Camera[1] = 0;
        if (instance->sandboxData->Camera[0] > BoundX) instance->sandboxData->Camera[0] = BoundX;
        if (instance->sandboxData->Camera[1] > BoundY) instance->sandboxData->Camera[1] = BoundY;
    }
}

void Engine::ProcessNodeClick() {
    if (!IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) return;

    Vec2 mousePos{GetMousePosition().x, GetMousePosition().y};
    const auto zoom {instance->sandboxData->Zoom};
    Vec2 worldPos{mousePos[0] / zoom + instance->sandboxData->Camera[0], mousePos[1] / zoom + instance->sandboxData->Camera[1]};

    const auto mode{instance->sandboxData->Edit.SelectedMode};
    auto it {instance->ModeMap.find(mode)};
    auto event {it== ModeMap.end() ? ClickEvent::NONE : it->second};

    instance->inputBlock = {
        .Blocked = instance-> nodes.ProcessNodeClick(worldPos,event),
        .Type = InputBlock::BlockType::Node
    };
}

void Engine::ProcessInput() {
    //Mouse inputs
    if (!instance->inputBlock.Blocked || instance->inputBlock.Type == InputBlock::BlockType::Button) instance->ProcessButtons();
    if (!instance->inputBlock.Blocked || instance->inputBlock.Type == InputBlock::BlockType::Node) instance->ProcessNodeClick();
    if (!instance->inputBlock.Blocked || instance->inputBlock.Type == InputBlock::BlockType::Camera) instance->ProcessCameraMovement();
    if (instance->sandboxData->Edit.Enabled && !instance->inputBlock.Blocked) instance->ProcessEditInputs();

    if (instance->inputBlock.BlockLoop <= 0 ) {instance->inputBlock = {
        .Blocked = false,
        .Type = InputBlock::BlockType::None};
    }
    else instance->inputBlock.BlockLoop--;

    if (GetMouseWheelMove() != 0) instance->ZoomCamera();

    //Keyboard inputs

    // Toggle editing mode with E key
    if (IsKeyPressed(KEY_E)) {
        instance->sandboxData->Edit.Enabled = !instance->sandboxData->Edit.Enabled;
        instance->sandboxData->Edit.SelectedMode.clear();
        instance->nodes.ClearSelectedNodes();
    };
}

void Engine::MoveCamera(const Vec2 LastMousePosition) {
    constexpr float MinimumMovementThreshold = 2.0f; // Minimum movement in pixels to trigger camera movement
    auto mousePos = GetMousePosition();

    const auto deltaX {LastMousePosition[0] - mousePos.x};
    const auto deltaY {LastMousePosition[1] - mousePos.y};

    // If the mouse movement is too small, ignore it to prevent jittery camera movement
    if (std::abs(deltaX) < MinimumMovementThreshold && std::abs(deltaY) < MinimumMovementThreshold)
        return;

    const auto newCameraX{instance->sandboxData->Camera[0] + deltaX};
    const auto newCameraY{instance->sandboxData->Camera[1] + deltaY};

    const auto zoom = instance->sandboxData->Zoom;
    const auto viewW = static_cast<float>(instance->env->Window.Width) / zoom;
    const auto viewH = static_cast<float>(instance->env->Window.Height) / zoom;
    const auto BoundX = std::max(0.0f, instance->env->Sandbox.Width - viewW);
    const auto BoundY = std::max(0.0f, instance->env->Sandbox.Height - viewH);

    instance->sandboxData->Camera[0] = newCameraX;
    instance->sandboxData->Camera[1] = newCameraY;


    // Clamp camera position to sandbox bounds
    if (instance->sandboxData->Camera[0] < 0) instance->sandboxData->Camera[0] = 0;
    if (instance->sandboxData->Camera[1] < 0) instance->sandboxData->Camera[1] = 0;

    if (instance->sandboxData->Camera[0] > BoundX) instance->sandboxData->Camera[0] = BoundX;
    if (instance->sandboxData->Camera[1] > BoundY) instance->sandboxData->Camera[1] = BoundY;
}

void Engine::DrawUI() {
    const auto CameraPosString = [](SandboxData &data)-> std::string {
        return ("Camera Position: "
            + std::to_string(data.Camera[0])
            + " "
            + std::to_string(data.Camera[1]));
    };
    const auto MousePosString = [](SandboxData &data)-> std::string {
        return ("Mouse Position: "
            + std::to_string(GetMousePosition().x)
            + " "
            + std::to_string(GetMousePosition().y));
    };
    const auto EnvInfoString = []()-> std::string {
        return ("FPS: "
            + std::to_string(GetFPS())
            + " Delta Time: "
            + std::to_string(GetFrameTime()));
    };

    auto DrawEditData = [] {
        const auto node{instance->sandboxData->Edit.SelectedMode};
        const auto editText = node.empty() ? "No node selected" : "Selected node: " + node;

        DrawText(editText.c_str(), 10, 100, 20, GREEN);
        Widget::Draw("RemoveNodeButton");
        Widget::Draw("AddEdgeButton");
        for (const auto nodetype: instance->NodeFactory | std::views::keys) {
            Widget::Draw(nodetype);
        }
    };

    DrawText(CameraPosString(*instance->sandboxData).c_str(), 10, 10, 20, WHITE);
    DrawText(MousePosString(*instance->sandboxData).c_str(), 10, 40, 20, WHITE);
    DrawText(EnvInfoString().c_str(), 10, 70, 20, WHITE);

    if (instance->sandboxData->Edit.Enabled) DrawEditData();
    else DrawText("Press E to enter edit mode", 10, 100, 20, GREEN);
    Widget::Draw("ClearButton");
}

void Engine::DrawNode(INode& node) {
    const auto NodeScreenPos{node.GetScreenPosition(instance->sandboxData->Camera)};
    const auto zoom {instance->sandboxData->Zoom};
    const auto id {node.GetData().Id};

    const auto isConnected = !node.GetData().Edges.empty();
    const auto baseColor = isConnected ? GREEN : RED;
    const auto color{nodes.IsNodeSelected(id) ? ORANGE : baseColor};

    DrawCircle(NodeScreenPos[0] * zoom, NodeScreenPos[1] * zoom, node.GetRadius() * zoom, color);
}

void Engine::DrawEdge(Edge& edge) {
    const auto fromNode{nodes.GetNode(edge.NodeA)};
    const auto toNode{nodes.GetNode(edge.NodeB)};

    if (fromNode == nullptr || toNode == nullptr) return;

    const auto fromPos{fromNode->GetScreenPosition(instance->sandboxData->Camera)};
    const auto toPos{toNode->GetScreenPosition(instance->sandboxData->Camera)};
    const auto zoom = instance->sandboxData->Zoom;

    const auto color = (edge.Active == true ? GREEN : BLUE);
    edge.Active = false;

    DrawLine(fromPos[0] * zoom, fromPos[1] * zoom, toPos[0] * zoom, toPos[1] * zoom, color);
}

void Engine::RegisterNodeType(const std::string &typeName, std::function<std::unique_ptr<INode>(Vec2 position)> factoryFunction){

    if (instance->ModeMap.contains(typeName)) {
        std::cerr << "Cannot register node type with reserved name: " << typeName << "\n";
        return;
    }

    instance->NodeFactory.insert_or_assign(typeName, factoryFunction);

    WidgetData data {
        .PosX = 10,
        .PosY = 190 + 30 * static_cast<float>(instance->NodeFactory.size() - 1),
        .Width = 150,
        .Height = 20,
        .Border = {0.0f, WHITE}
    };
    ButtonData buttonData{
        .HoverColor = LIGHTGRAY,
        .Text = typeName,
        .OnClick = [typeName](){
            instance->sandboxData->Edit.SelectedMode = typeName;
        }
    };
    Widget::Register(typeName, std::make_unique<Button>(data, buttonData));
}
