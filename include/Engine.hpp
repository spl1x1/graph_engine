#ifndef ENGINE_HPP
#define ENGINE_HPP

#include <functional>
#include <memory>
#include <vector>
#include <functional>


class Engine{
static std::unique_ptr<Engine> instance;

std::vector<std::function<void()>> hooks;

Engine();

// Delete copy and move constructors and assignment operators
Engine(const Engine&) = delete;
Engine& operator=(const Engine&) = delete;
Engine(Engine&&) = delete;
Engine& operator=(Engine&&) = delete;

public:
    static void Init();
    static void Loop();
};

#endif // ENGINE_HPP
