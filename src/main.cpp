
#include "Engine.hpp"

int main() {
    Enviroment env = {
        .Window = {800, 600, 60},
        .Sandbox = {4000, 4000},
        .Title = "Graph Engine"
    };
    Engine::Init(env);
    Engine::Loop();
    return 0;
}
