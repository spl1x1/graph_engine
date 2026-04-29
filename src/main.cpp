#include <Engine.hpp>
#include <EngineTypes.hpp>

int main() {
    constexpr int windowWidth = 800;
    constexpr int windowHeight = 600;

    constexpr int sandboxWidth = 2000;
    constexpr int sandboxHeight = 2000;

    constexpr float sandboxCenterX = sandboxWidth / 2.f - windowWidth / 2.f;
    constexpr float sandboxCenterY = sandboxHeight / 2.f - windowHeight / 2.f;

    Enviroment env = {
        .Background = {"assets/nebula_background.png"},
        .Window = {windowWidth, windowHeight, 60},
        .Sandbox = {sandboxWidth, sandboxHeight, 20},
        .Title = "Graph Engine",
    };

    SandboxData sandboxData = {
        .Camera = Vec2{sandboxCenterX, sandboxCenterY},
        .Zoom = 1.0
    };

    Engine::Init(&env, &sandboxData);
    Engine::Loop();
    return 0;
}
