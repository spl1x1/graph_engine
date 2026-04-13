#ifndef ENGINE_HPP
#define ENGINE_HPP

#include "EngineTypes.hpp"

#include <memory>


class Engine{
static std::unique_ptr<Engine> instance;

Enviroment env;

Engine(Enviroment env);

// Delete copy and move constructors and assignment operators
Engine(const Engine&) = delete;
Engine& operator=(const Engine&) = delete;
Engine(Engine&&) = delete;
Engine& operator=(Engine&&) = delete;

public:
    static void Init(Enviroment env);
    static void Loop();

};

#endif // ENGINE_HPP
