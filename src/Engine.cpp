#include "../include/Engine.hpp"
#include <raylib.h>
#include <iostream>
#include <cassert>

std::unique_ptr<Engine> Engine::instance = nullptr;

Engine::Engine() {
    // Initialize raylib
    InitWindow(800, 600, "Graph Engine");
    SetTargetFPS(60);
}

void Engine::Init(){
    if (instance != nullptr) {
        std::cerr << "Engine already initialized!" << std::endl;
        return;
    }
    instance = std::unique_ptr<Engine>(new Engine());
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
}
