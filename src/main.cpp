
#include "../include/Engine.hpp"
#include <iostream>

int main() {
    Engine::Init();
    Engine::Loop();
    std::cout << "Hello, Graph Engine!" << std::endl;
    return 0;
}
