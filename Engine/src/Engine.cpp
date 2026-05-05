#include <cassert>
#include <cmath>
#include <iostream>
#include <ranges>
#include <string>
#include "Engine.hpp"
#include "EngineTypes.hpp"
#include "raylib.h"
#include "Button.hpp"


std::unique_ptr<Engine> Engine::instance = nullptr;

Engine::Engine(Enviroment *env, SandboxData *sandboxData) {
    // Initialize raylib
    InitWindow(env->Window.Width, env->Window.Height, env->Title.c_str());
    SetTargetFPS(env->Window.FrameRate);
    this->env = env;
    this->sandboxData = sandboxData;

}

void Engine::Init(Enviroment *env, SandboxData *sandboxData) {
    [[unlikely]]
    if (instance != nullptr) {
        std::cerr << "Engine already initialized!" << "\n";
        return;
    }

    instance = std::unique_ptr<Engine>(new Engine(env, sandboxData));
    LoadBackground(env->Background);
    std::cout << "Engine initialized" << "\n";
}

void Engine::Loop() {
    assert(instance != nullptr && "Engine must be initialized before calling Loop");
    while (!WindowShouldClose()) {
        ProcessInput();
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

    const float tileW = static_cast<float>(instance->env->Background.Width);
    const float tileH = static_cast<float>(instance->env->Background.Height);

    float offsetX = std::fmod(instance->sandboxData->Camera[0], tileW);
    float offsetY = std::fmod(instance->sandboxData->Camera[1], tileH);

    if (offsetX < 0) offsetX += tileW;
    if (offsetY < 0) offsetY += tileH;

    Vec2 FrameCorner{
        -offsetX,
        -offsetY,
    };

    for (float y = FrameCorner[1]; y < instance->env->Window.Height; y += tileH) {
        for (float x = FrameCorner[0]; x < instance->env->Window.Width; x += tileW) {
            DrawTexture(instance->backgroundTexture, static_cast<int>(x), static_cast<int>(y), WHITE);
        }
    }
}

void Engine::DrawSandbox() {
    auto engine{Engine::instance.get()};
    engine->DrawBackground();
    engine->DrawUI();
}

void Engine::ProccesCameraMovement(){
    //Mouse dragging for camera movement
    constexpr float MouseHoldThreshold = 0.2f; // Time in seconds to trigger camera movement
    auto engine{Engine::instance.get()};
    auto mouseDown {IsMouseButtonDown(MOUSE_LEFT_BUTTON)};
    static float MouseDownTime{0.0f};
    static Vec2 LastMousePosition{GetMouseX(), GetMouseY()};

    if (mouseDown) MouseDownTime += engine->env->DeltaTime;
    else MouseDownTime = 0.0f;
    if (MouseDownTime >= MouseHoldThreshold) instance->MoveCamera(LastMousePosition);

    LastMousePosition[0] = GetMouseX();
    LastMousePosition[1] = GetMouseY();
}

void Engine::ProcessInput() {
    //Mouse inputs
    if (!Button::ProccessInputs()) instance->ProccesCameraMovement();

    //Keyboard inputs

    // Toggle editing mode with E key
    if (IsKeyPressed(KEY_E)) {
        instance->sandboxData->Edit.Enabled = !instance->sandboxData->Edit.Enabled;
        instance->sandboxData->Edit.SelcectedNode.clear();
    };

    if (instance->sandboxData->Edit.Enabled) {
        // Add node with left mouse button
    }
}

void Engine::MoveCamera(const Vec2 LastMousePosition) {
    constexpr float MinimumMovementThreshold = 5.0f; // Minimum movement in pixels to trigger camera movement
    auto mousePos = GetMousePosition();

    const auto deltaX {LastMousePosition[0] - mousePos.x};
    const auto deltaY {LastMousePosition[1] - mousePos.y};

    // If the mouse movement is too small, ignore it to prevent jittery camera movement
    if (std::abs(deltaX) < MinimumMovementThreshold && std::abs(deltaY) < MinimumMovementThreshold)
        return;

    const auto newCameraX{instance->sandboxData->Camera[0] + deltaX};
    const auto newCameraY{instance->sandboxData->Camera[1] + deltaY};

    const auto BoundX{instance->env->Sandbox.Width - instance->env->Window.Width};
    const auto BoundY{instance->env->Sandbox.Height - instance->env->Window.Height};

    instance->sandboxData->Camera[0] = newCameraX;
    instance->sandboxData->Camera[1] = newCameraY;


    // Clamp camera position to sandbox bounds
    if (instance->sandboxData->Camera[0] < 0) instance->sandboxData->Camera[0] = 0;
    if (instance->sandboxData->Camera[1] < 0) instance->sandboxData->Camera[1] = 0;

    if (instance->sandboxData->Camera[0] > BoundX) instance->sandboxData->Camera[0] = BoundX;
    if (instance->sandboxData->Camera[1] > BoundY) instance->sandboxData->Camera[1] = BoundY;
}

void Engine::DrawUI() {
    const auto CameraPosition{"Camera Position: "
        + std::to_string(instance->sandboxData->Camera[0])
        + " "
        + std::to_string(instance->sandboxData->Camera[1])};

    const auto MousePosition{"Mouse Position: "
        + std::to_string(GetMousePosition().x)
        + " "
        + std::to_string(GetMousePosition().y)};

    const auto DeltaTime{"FPS: " + std::to_string(GetFPS())};

    DrawText(CameraPosition.c_str(), 10, 10, 20, WHITE);
    DrawText(MousePosition.c_str(), 10, 40, 20, WHITE);
    DrawText(DeltaTime.c_str(), 10, 70, 20, WHITE);

    auto DrawEditData = [] {
        const auto node{instance->sandboxData->Edit.SelcectedNode};
        const auto editText = node.empty() ? "No node selected" : "Selected node: " + node;

        DrawText(editText.c_str(), 10, 100, 20, GREEN);
        for (const auto nodetype: instance->NodeFactory | std::views::keys) {
            Button::Draw(nodetype);
        }
    };

    if (instance->sandboxData->Edit.Enabled) DrawEditData();
}

void Engine::DrawNode(const NodeAbstract& node) {
    const auto NodeScreenPos{node.GetScreenPosition(instance->sandboxData->Camera)};
    DrawCircle(NodeScreenPos[0], NodeScreenPos[1], 10, RED);
}

void Engine::RegisterNodeType(const std::string &typeName, std::function<std::unique_ptr<NodeAbstract>(Vec2 position)> factoryFunction){
    instance->NodeFactory.insert_or_assign(typeName, factoryFunction);

    Button::ButtonData buttonData{
        .PosX = 10,
        .PosY = 130 + 30 * static_cast<float>(instance->NodeFactory.size() - 1),
        .Width = 150,
        .Height = 20,
        .HoverColor = LIGHTGRAY,
        .Text = typeName,
        .OnClick = [typeName](){
            instance->sandboxData->Edit.SelcectedNode = typeName;
            std::cout << "Selected node type: " << typeName << "\n";
        }
    };
    Button button(typeName, buttonData);
}
