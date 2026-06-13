#include <Engine.hpp>
#include <EngineTypes.hpp>
#include <functional>
#include <iostream>
#include "AutoSyncManager.hpp"
#include "Node.hpp"
#include "Router.hpp"
#include <Button.hpp>
#include <Widget.hpp>

int main() {
    constexpr int windowWidth = 1200;
    constexpr int windowHeight = 960;

    constexpr int sandboxWidth = 2000;
    constexpr int sandboxHeight = 2000;

    constexpr float sandboxCenterX = sandboxWidth / 2.f - windowWidth / 2.f;
    constexpr float sandboxCenterY = sandboxHeight / 2.f - windowHeight / 2.f;

    Enviroment env = {
        .Bg = {"assets/nebula_background.png"},
        .Window = {windowWidth, windowHeight, 60},
        .Sandbox = {sandboxWidth, sandboxHeight},
        .Title = "Graph Engine",
    };

    SandboxData sandboxData = {
        .Camera = Vec2{sandboxCenterX, sandboxCenterY},
        .Zoom = 1.0
    };

    Engine::Init(&env, &sandboxData);
    Engine::RegisterNodeType("Router", [](Vec2 position) {
        return std::make_unique<Router>(position);
    });

    // Register Sync LSDB button – floods LSAs to all neighbors of the selected router
    WidgetData syncWidgetData{
        .PosX = 170.0f,
        .PosY = 130.0f,
        .Width = 120.0f,
        .Height = 20.0f,
        .Border = {0.0f, WHITE}
    };
    ButtonData syncButtonData{
        .HoverColor = LIGHTGRAY,
        .Text = "Sync LSDB",
        .OnClick = [](){
            INode* selected = Engine::GetSelectedNode();
            if (selected) {
                if (auto* router = dynamic_cast<Router*>(selected)) {
                    router->GetLSDB().SyncWithNeighbors();
                } else {
                    std::cout << "Selected node is not a Router\n";
                }
            } else {
                std::cout << "No router selected. Click a router first.\n";
            }
        }
    };
    Widget::Register("SyncButton", std::make_unique<Button>(syncWidgetData, syncButtonData));

    /*Engine::RegisterUpdateFunction({
        [&]() {
            auto selectedNode = Engine::GetSelectedNode();
            if (selectedNode && dynamic_cast<Router*>(selectedNode)) {
                AutoSyncManager::SetTarget(dynamic_cast<Router*>(selectedNode));
            }
            AutoSyncManager::Update(env.DeltaTime);
        }
    });*/
    Engine::Loop();
    return 0;
}
