#include "Engine.hpp"
#include <raylib.h>
#include <iostream>
#include <cassert>

std::unique_ptr<Engine> Engine::instance = nullptr;

Engine::Engine(Enviroment env) {
    // Initialize raylib
    InitWindow(env.Window.Width, env.Window.Height, env.Title.c_str());
    SetTargetFPS(env.Window.FrameRate);
    this->env = env;
}

void Engine::Init(Enviroment env){
    if (instance != nullptr) {
        std::cerr << "Engine already initialized!" << std::endl;
        return;
    }
    instance = std::unique_ptr<Engine>(new Engine(env));
    std::cout << "Engine initialized" << std::endl;
}

void Engine::Loop() {
    assert(instance != nullptr && "Engine must be initialized before calling Loop");
    while (!WindowShouldClose()) {
        // Update logic here

        // Draw
        BeginDrawing();
        ClearBackground(RAYWHITE);

        // Draw your game objects here

        EndDrawing();
    }
    std::cout << "Engine loop ended" << std::endl;
}
