#include "SandboxSave.hpp"
#include <cassert>
#include <cmath>
#include <cstddef>
#include <format>
#include <iostream>
#include <ranges>
#include <string>
#include <Engine.hpp>
#include <EngineTypes.hpp>
#include <Node.hpp>
#include <Widget.hpp>
#include <raylib.h>
#include <Button.hpp>


std::unique_ptr<Engine> Engine::instance = nullptr;

Engine::Engine(Enviroment *env, SandboxData *sandboxData) : env(env), sandboxData(sandboxData) {
    // Initialize raylib
    InitWindow(env->Window.Width, env->Window.Height, env->Title.c_str());
    SetTargetFPS(env->Window.FrameRate);
    SetExitKey(KEY_P);
}

void Engine::Init(Enviroment *env, SandboxData *sandboxData) {

    sandboxData->BaseCameraPosition = sandboxData->Camera;

    constexpr int TemplateWidth{150};
    constexpr int TemplateHeight{20};

    const auto ScreenCenterX {env->Window.Width/2};
    const auto ScreenCenterY{env->Window.Height/2};

    const auto WidgetDataTemplate = [] (const float PosX, const float PosY) -> WidgetData {
        return WidgetData{
            .PosX = PosX,
            .PosY = PosY,
            .Width = TemplateWidth,
            .Height = TemplateHeight,
            .Border = {0.0f, WHITE}
        };
    };

    const auto SmallWidgetDataTemplate = [] (const float PosX, const float PosY) -> WidgetData {
        return WidgetData{
            .PosX = PosX,
            .PosY = PosY,
            .Width = TemplateHeight,
            .Height = TemplateHeight,
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
    LoadBackground(env->Bg);
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

    const auto AreaOctet1PlusWidgetData = SmallWidgetDataTemplate(env->Window.Width - 220, 160);
    const auto AreaOctet1PlusButtonData = ButtonDataTemplate("+", [](){
        instance->sandboxData->Edit.SelectedNetworkArea.octets[0]++;
    });

    const auto AreaOctet1MinusWidgetData = SmallWidgetDataTemplate(env->Window.Width - 200, 160);
    const auto AreaOctet1MinusButtonData = ButtonDataTemplate("-", [](){
        instance->sandboxData->Edit.SelectedNetworkArea.octets[0]--;
    });

    const auto AreaOctet2PlusWidgetData = SmallWidgetDataTemplate(env->Window.Width - 180, 160);
    const auto AreaOctet2PlusButtonData = ButtonDataTemplate("+", [](){
        instance->sandboxData->Edit.SelectedNetworkArea.octets[1]++;
    });

    const auto AreaOctet2MinusWidgetData = SmallWidgetDataTemplate(env->Window.Width - 160,160);
    const auto AreaOctet2MinusButtonData = ButtonDataTemplate("-", [](){
        instance->sandboxData->Edit.SelectedNetworkArea.octets[1]--;
    });

    const auto LinkSpeedWidgetData = WidgetDataTemplate(env->Window.Width - 220, 220);
    const auto LinkSpeedButtonData = ButtonDataTemplate("Link Speed", [](){
        auto newSpeed{(static_cast<int>(instance->sandboxData->Edit.SelectedSpeed) + 1) % LinkSpeedsCount};
        instance->sandboxData->Edit.SelectedSpeed = static_cast<LinkSpeed>(newSpeed);
        instance->nodes.SelectedLinkSpeed = instance->sandboxData->Edit.SelectedSpeed;
    });

    const auto MenuPositionX{ScreenCenterX-(TemplateWidth/2)};
    const auto MenuPostionY{ScreenCenterY-(TemplateHeight/2)};


    Widget::Register("ClearButton", std::make_unique<Button>(ClearWidgetData, ClearButtonData));
    Widget::Register("AddEdgeButton", std::make_unique<Button>(AddEdgeWidgetData, AddEdgeButtonData));
    Widget::Register("RemoveNodeButton", std::make_unique<Button>(RemoveWidgetData, RemoveButtonData));
    Widget::Register("AreaOctet1PlusButton", std::make_unique<Button>(AreaOctet1PlusWidgetData, AreaOctet1PlusButtonData));
    Widget::Register("AreaOctet1MinusButton", std::make_unique<Button>(AreaOctet1MinusWidgetData, AreaOctet1MinusButtonData));
    Widget::Register("AreaOctet2PlusButton", std::make_unique<Button>(AreaOctet2PlusWidgetData, AreaOctet2PlusButtonData));
    Widget::Register("AreaOctet2MinusButton", std::make_unique<Button>(AreaOctet2MinusWidgetData, AreaOctet2MinusButtonData));
    Widget::Register("LinkSpeedButton", std::make_unique<Button>(LinkSpeedWidgetData, LinkSpeedButtonData));
    Widget::AddToGroup(
        "EditButtons", {
        "AddEdgeButton",
        "RemoveNodeButton",
        "AreaOctet1PlusButton",
        "AreaOctet1MinusButton",
        "AreaOctet2PlusButton",
        "AreaOctet2MinusButton",
        "LinkSpeedButton"
        });
    Widget::AddToGroup("BaseButtons","ClearButton");
}

void Engine::InitSave(const std::string& saveFile) {
    if (instance == nullptr) {
        std::cerr << "Error: Engine must be initialized before calling InitSave" << "\n";
        return;
    }

    SandboxVariablePointers pointers{
        .Network = &instance->nodes,
        .Sandbox = instance->sandboxData,
        .NodeFactory = &instance->NodeFactory
    };

    SandboxSave::Init(saveFile, pointers);
}

void Engine::Loop() {
    auto CallUpdateFunctions = []() {
        for (const auto& func: instance->UpdateFunctions) {
             func();
        }
    };

    assert(instance != nullptr && "Engine must be initialized before calling Loop");

    while (!WindowShouldClose()) {
        ProcessInput();
        Widget::EndDrawing();
        CallUpdateFunctions();
        BeginDrawing();

        DrawSandbox();

        ClearBackground(RAYWHITE);
        EndDrawing();
        instance->env->DeltaTime = GetFrameTime();    }
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

    instance->env->Bg = background;

    std::cout << "Loading background texture... \n";
}

void Engine::DrawBackground() {
    if (instance->backgroundTexture.id == 0) {
        DrawRectangle(0, 0, instance->env->Window.Width, instance->env->Window.Height, BLUE);
        return;
    }

    const float zoom = instance->sandboxData->Zoom;
    const float tileW = static_cast<float>(instance->env->Bg.Width) * zoom;
    const float tileH = static_cast<float>(instance->env->Bg.Height) * zoom;

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


void Engine::ProcessCameraMovement(){
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
}

void Engine::ProcessButtons() {
    instance->inputBlock = {
        .Blocked = Button::ProccessInputs(),
        .Type = InputBlock::BlockType::Button
    };
}

void Engine::ProcessEditInputs() {
    if (!instance->sandboxData->Edit.Enabled) return;
    const auto AddNode = [&](){
        Vec2 mousePos{GetMousePosition().x, GetMousePosition().y};
        const auto zoom {instance->sandboxData->Zoom};
        Vec2 worldPos{mousePos[0] / zoom + instance->sandboxData->Camera[0], mousePos[1] / zoom + instance->sandboxData->Camera[1]};

        auto node = instance->NodeFactory.at(instance->sandboxData->Edit.SelectedMode)(worldPos);
        instance->nodes.AddNode(std::move(node), instance->sandboxData->Edit.SelectedNetworkArea);
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

    instance->inputBlock = {
        .Blocked = instance-> nodes.ProcessNodeClick(worldPos,Event::StringToClick(mode)),
        .Type = InputBlock::BlockType::Node
    };
}

void Engine::ProcessKeyboard(){
    //Keyboard inputs

    // Toggle editing mode with E key
    if (IsKeyPressed(KEY_E)) {
        instance->sandboxData->Edit.Enabled = !instance->sandboxData->Edit.Enabled;
        instance->sandboxData->Edit.SelectedMode.clear();
        instance->nodes.ClearSelectedNodes();
    };
    if (IsKeyPressed(KEY_R)) {
        instance->sandboxData->Edit.SelectedMode.clear();
        instance->nodes.ClearSelectedNodes();
        sandboxData->Edit.MessageBuffer.clear();
    }
    if (IsKeyPressed(KEY_ENTER)) {
        instance->sandboxData->TextInputActive = true;
        instance->sandboxData->Edit.MessageBuffer.clear();
    }
    if(IsKeyPressed(KEY_C)){
        instance->sandboxData->Camera[0] = instance->sandboxData->BaseCameraPosition[0];
        instance->sandboxData->Camera[1] = instance->sandboxData->BaseCameraPosition[1];
        instance->sandboxData->Zoom = 1.0f;
    }
    if (IsKeyPressed(KEY_H)) {
        sandboxData->ShowDebug = !sandboxData->ShowDebug;
    }
    if (IsKeyPressed(KEY_ESCAPE)){
        SandboxSave::Save();
        std::cout<< "Saving sandbox! \n";
    }
}

void Engine::ResetInputBlock() {
    instance->inputBlock = {
        .Blocked = false,
        .Type = InputBlock::BlockType::None
    };
}

void Engine::ProcessTextInput(){
    if (!sandboxData->TextInputActive) return;
    if (IsKeyPressed(KEY_ENTER)) {sandboxData->TextInputActive = false;};
    if (IsKeyPressed(KEY_BACKSPACE) && !sandboxData->Edit.MessageBuffer.empty()) {
        sandboxData->Edit.MessageBuffer.pop_back();
    }
    instance->inputBlock = {
      .Blocked = true,
      .BlockLoop = 1, //Block input for one loop to prevent processing other inputs while typing
      .Type = InputBlock::BlockType::TextInput,
    };

    int key = GetCharPressed();
    while (key > 0) {
        sandboxData->Edit.MessageBuffer += static_cast<char>(key);
        key = GetCharPressed();
    }
};


void Engine::ProcessInput() {
    //Input processing
    #define ENTRY(type, func) if (!instance->inputBlock.Blocked || instance->inputBlock.Type == InputBlock::BlockType::type) func;
     INPUT_TABLE
    #undef ENTRY

    if (instance->inputBlock.BlockLoop <= 0 ) instance->ResetInputBlock();
    else instance->inputBlock.BlockLoop--;

    if (GetMouseWheelMove() != 0) instance->ZoomCamera();
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
    const auto MessageBufferString = [](SandboxData &data)-> std::string {
        const auto bufferData{data.Edit.MessageBuffer.empty() ? "<empty>" : data.Edit.MessageBuffer};
        return ("Message Buffer: "
            + bufferData);
    };
    auto messageBufferPosY{160.0f};

    auto DrawSelectedMode = [&] {
        const auto mode{instance->sandboxData->Edit.SelectedMode};
        const auto editText = mode.empty() ? "No mode selected" : "Selected mode: " + mode + " | Press R to clear selection";

        DrawText(editText.c_str(), 10, 100, 20, GREEN);
    };
    auto DrawEditData = [&] {
        DrawSelectedMode();
        DrawText((instance->sandboxData->Edit.SelectedNetworkArea.ToString()).c_str(), env->Window.Width - 220, 140, 20, GREEN);
        DrawText((std::format("{:.1f} Mbps", GetSpeedMbps(instance->sandboxData->Edit.SelectedSpeed))).c_str(), env->Window.Width - 220, 190, 20, GREEN);

        Widget::DrawGroup("EditButtons");
        for (const auto nodetype: instance->NodeFactory | std::views::keys) {
            Widget::Draw(nodetype);
        }
        messageBufferPosY = 160 + 30 * static_cast<float>(instance->NodeFactory.size() + 1);
    };

    if (instance->sandboxData->ShowDebug) {
        DrawText(CameraPosString(*instance->sandboxData).c_str(), 10, 10, 20, WHITE);
        DrawText(MousePosString(*instance->sandboxData).c_str(), 10, 40, 20, WHITE);
        DrawText(EnvInfoString().c_str(), 10, 70, 20, WHITE);
    }
    else{
        DrawText("Press H to toggle debug info", 10, 10, 20, WHITE);
        DrawText("Press C to reset camera position and zoom", 10, 40, 20, WHITE);
        DrawText("Press ESC to save sandbox", 10, 70, 20, WHITE);
    }

    if (instance->sandboxData->Edit.Enabled) DrawEditData();
    else if (instance->sandboxData->Edit.SelectedMode == "SEND_MESSAGE") DrawSelectedMode();
    else DrawText("E - EDIT MODE | ENTER - EDIT MESSAGE", 10, 100, 20, GREEN);

    DrawText(MessageBufferString(*instance->sandboxData).c_str(), 10, messageBufferPosY, 20, WHITE);
    if (instance->sandboxData->TextInputActive) {
        const auto BufferInfo {"Press ENTER to finish typing | Backspace to delete"};
        DrawText(BufferInfo, 10, messageBufferPosY + 30, 20, GREEN);
    }
    Widget::DrawGroup("BaseButtons");
}

void Engine::DrawNode(INode& node) {
    const auto NodeScreenPos{node.GetScreenPosition(instance->sandboxData->Camera)};
    const auto zoom {instance->sandboxData->Zoom};
    auto& nodeData = node.GetData();
    const auto id {nodeData.Id};

    const auto isConnected = !nodeData.Edges.empty();
    const auto baseColor = isConnected ? GREEN : RED;
    Color color = nodes.IsNodeSelected(id) ? ORANGE : baseColor;
    if (nodeData.FlashTimer > 0.0f) {
        color = SKYBLUE;
        nodeData.FlashTimer = std::max(0.0f, nodeData.FlashTimer - instance->env->DeltaTime);
    }

    DrawCircle(NodeScreenPos[0] * zoom, NodeScreenPos[1] * zoom, node.GetRadius() * zoom, color);
    if (nodeData.DeliveredMessageTimer > 0.0f && !nodeData.LastDeliveredMessage.empty()) {
        DrawText(nodeData.LastDeliveredMessage.c_str(),
            NodeScreenPos[0] * zoom - 30,
            NodeScreenPos[1] * zoom + 25,
            16,
            YELLOW);
        nodeData.DeliveredMessageTimer = std::max(0.0f, nodeData.DeliveredMessageTimer - instance->env->DeltaTime);
    }
    if (sandboxData->ShowDebug){
        DrawText(std::format("ID: {}", id).c_str(), NodeScreenPos[0] * zoom - 30, NodeScreenPos[1] * zoom + 25, 16, WHITE);
        DrawText(std::format("Addr: {}", nodeData.Address.ToString()).c_str(), NodeScreenPos[0] * zoom - 30, NodeScreenPos[1] * zoom + 45, 16, WHITE);
    }
}

void Engine::DrawEdge(Edge& edge) {
    const auto fromNode{nodes.GetNode(edge.Key.NodeA)};
    const auto toNode{nodes.GetNode(edge.Key.NodeB)};

    if (fromNode == nullptr || toNode == nullptr) return;

    const auto fromPos{fromNode->GetScreenPosition(instance->sandboxData->Camera)};
    const auto toPos{toNode->GetScreenPosition(instance->sandboxData->Camera)};
    const auto zoom = instance->sandboxData->Zoom;

    const auto areaColor = fromNode->GetData().Address.compareArea(toNode->GetData().Address) ? GREEN : BLUE;
    Color color = areaColor;
    if (edge.RouteFlashTimer > 0.0f) {
        color = MAGENTA;
        edge.RouteFlashTimer = std::max(0.0f, edge.RouteFlashTimer - instance->env->DeltaTime);
    } else if (edge.Active == true) {
        color = YELLOW;
    }
    edge.Active = false;

    const Vector2 fromScreen{fromPos[0] * zoom, fromPos[1] * zoom};
    const Vector2 toScreen{toPos[0] * zoom, toPos[1] * zoom};

    auto linkSpeedMbps = GetSpeedMbps(edge.Speed);
    constexpr float ThicknessScaleMultiplier = 1.2f;
    const auto thickness = static_cast<float>(std::log2(linkSpeedMbps)) * zoom * ThicknessScaleMultiplier;
    for (float i = -thickness / 2; i <= thickness / 2; i += 1.0f) {
        DrawLineEx(fromScreen, toScreen, std::abs(i), color);
    }

    if (sandboxData->ShowDebug){
    const Vector2 midPoint{(fromScreen.x + toScreen.x) / 2, (fromScreen.y + toScreen.y) / 2};
    DrawText(std::format("{:.1f} Mbps", linkSpeedMbps).c_str(), midPoint.x + 5, midPoint.y + 5, 20, WHITE);
    }
}

void Engine::RegisterNodeType(const std::string &typeName, std::function<std::unique_ptr<INode>(Vec2 position)> factoryFunction){
    if (typeName=="NONE" || Event::StringToClick(typeName) != Event::Click::NONE){
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

void Engine::RegisterUpdateFunction(std::function<void()> func) {
    instance->UpdateFunctions.push_back(func);
}

INode* Engine::GetSelectedNode(){
    if (!instance) return nullptr;
    return instance->nodes.GetSelectedNode();
}

std::vector<INode*> Engine::GetAllNodes() {
    std::vector<INode*> result;
    if (!instance) return result;
    for (const auto& [id, node] : instance->nodes.GetNodeMap()) {
        if (node) result.push_back(node.get());
    }
    return result;
}

std::optional<SelectedNodes> Engine::ConsumeMessageSelection() {
    if (!instance) return std::nullopt;
    return instance->nodes.ConsumeMessageSelection();
}

void Engine::FlashEdgeBetween(uint16_t nodeA, uint16_t nodeB, float durationSeconds) {
    if (!instance) return;
    instance->nodes.FlashEdgeBetween(nodeA, nodeB, durationSeconds);
}

void Engine::ClearSelectedNode(){
    if (!instance) return;
    instance->nodes.ClearSelectedNodes();
}
