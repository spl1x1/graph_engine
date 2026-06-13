#ifndef SANDBOXSAVE_HPP
#define SANDBOXSAVE_HPP

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <Node.hpp>
#include <EngineTypes.hpp>

struct SandboxVariablePointers{
    NodeNetwork* Network;
    SandboxData* Sandbox;
    std::unordered_map<std::string, std::function<std::unique_ptr<INode>(Vec2 position)>>* NodeFactory;
};

class SandboxSave {
    static std::unique_ptr<SandboxSave> instance;

    SandboxVariablePointers pointers;

    std::string saveFile;
    void Load();

public:
    static void SetPointers(SandboxVariablePointers pointers);
    static void Init(std::string saveFile, SandboxVariablePointers pointers);
    static void Save();

};




#endif // SANDBOXSAVE_HPP
