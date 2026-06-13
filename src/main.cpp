#include <Engine.hpp>
#include <EngineTypes.hpp>
#include <functional>
#include "AutoSyncManager.hpp"
#include "Node.hpp"
#include "Router.hpp"

int main() {
    constexpr int windowWidth = 1200;
    constexpr int windowHeight = 960;

    constexpr int sandboxWidth = 2000;
    constexpr int sandboxHeight = 2000;

    constexpr float sandboxCenterX = sandboxWidth / 2.f - windowWidth / 2.f;
    constexpr float sandboxCenterY = sandboxHeight / 2.f - windowHeight / 2.f;

    Enviroment env = {
        .Background = {"assets/nebula_background.png"},
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
