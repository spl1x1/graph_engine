#include <SandboxSave.hpp>
#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <document.h>
#include <istreamwrapper.h>
#include <rapidjson.h>
#include <stringbuffer.h>
#include <writer.h>

std::unique_ptr<SandboxSave> SandboxSave::instance = std::make_unique<SandboxSave>();

using namespace rapidjson;

void SandboxSave::SetPointers(SandboxVariablePointers pointers) {
    instance->pointers = pointers;
}

void SandboxSave::Init(std::string saveFile, SandboxVariablePointers pointers) {
    instance->pointers = pointers;
    if (saveFile.empty()) {
        saveFile = "save.json";
    }

    instance->saveFile = saveFile;


    std::filesystem::path save = std::filesystem::current_path() / saveFile;
    if (!std::filesystem::exists(save)) {
        save = std::filesystem::current_path().parent_path() / saveFile;
    }
    if (!std::filesystem::exists(save)) {
        std::cerr << "Warning: Save file " << saveFile << " does not exist. A new save file will be created." << std::endl;
        std::fstream(saveFile, std::ios::out | std::ios::trunc).close();
        return;
    }
    else instance->saveFile = save.string();

    instance->Load();
}

void SandboxSave::Save() {
    assert(instance != nullptr && "SandboxSave must be initialized before calling Save");
    assert (instance->pointers.Network != nullptr && "SandboxSave pointers must be set before calling Save");
    //Dev assert crashne program při nullptr

    //Prod return early if nullptr, to prevent crashing in release build, but still catch the error in debug
    if (instance->pointers.Network == nullptr || instance->pointers.Sandbox == nullptr) {
        std::cerr << "Error: SandboxSave pointers not set. Save aborted." << std::endl;
        return;
    }

    Document document;
    document.SetObject();
    Document::AllocatorType& allocator = document.GetAllocator();
    auto data{instance->pointers};
    auto cameraPointer{data.Sandbox->Camera};

    document.AddMember("CameraX", cameraPointer[0], allocator);
    document.AddMember("CameraY", cameraPointer[1], allocator);

    Value Nodes(kObjectType);
    for(auto& [id,node] : data.Network->GetNodeMap())
    {
        Value nodeValue(kObjectType);
        nodeValue.AddMember("X", node->GetPosition()[0], allocator);
        nodeValue.AddMember("Y", node->GetPosition()[1], allocator);
        nodeValue.AddMember("Type", Value(node->GetData().Type.c_str(), allocator), allocator);
        nodeValue.AddMember("IP", Value(node->GetData().Address.ToString().c_str(), allocator), allocator);
        Nodes.AddMember(Value(std::to_string(id).c_str(), allocator), nodeValue, allocator);
    }
    document.AddMember("Nodes", Nodes, allocator);

    Value Edges(kObjectType);
    for (auto& [id, edge] : data.Network->GetEdgeMap())
    {
        Value edgeValue(kObjectType);
        edgeValue.AddMember("NodeA", edge.Key.NodeA, allocator);
        edgeValue.AddMember("NodeB", edge.Key.NodeB, allocator);
        edgeValue.AddMember("Speed", static_cast<int>(edge.Speed), allocator);
        Edges.AddMember(Value(std::to_string(id).c_str(), allocator), edgeValue, allocator);
    }
    document.AddMember("Edges", Edges, allocator);

    StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    document.Accept(writer);

    std::ofstream out(instance->saveFile, std::ios::out | std::ios::trunc);
    out << buffer.GetString();
    out.close();
}

void SandboxSave::Load() {
    assert (instance->pointers.Network != nullptr && "SandboxSave pointers must be set before calling Save");
    //Dev assert crashne program při nullptr

    //Prod return early if nullptr, to prevent crashing in release build, but still catch the error in debug
    if (instance->pointers.Network == nullptr || instance->pointers.Sandbox == nullptr) {
        std::cerr << "Error: SandboxSave pointers not set. Save aborted." << std::endl;
        return;
    }

    // Load the sandbox state from a file and deserialize it to populate the sandbox
    std::fstream fileStream(instance->saveFile);
    IStreamWrapper wrappedStream(fileStream);

    Document document;
    document.ParseStream(wrappedStream);

    if (document.HasParseError()) {
        std::cerr << "Error parsing save file: " << instance->saveFile << ". Load aborted.\n";
        return;
    }

    auto data{instance->pointers};

    //Set camera position
    data.Sandbox->Camera[0] = document["CameraX"].GetFloat();
    data.Sandbox->Camera[1] = document["CameraY"].GetFloat();

    //Load nodes
    const auto& nodes = document["Nodes"].GetObject();
    for (auto& [id, nodeValue] : nodes)
    {
        Vec2 position{nodeValue["X"].GetFloat(), nodeValue["Y"].GetFloat()};
        std::string type{nodeValue["Type"].GetString()};
        std::string ip{nodeValue["IP"].GetString()};

        if (data.NodeFactory->find(type) == data.NodeFactory->end()) {
            std::cerr << "Error: Node type " << type << " not registered. Node with id " << id.GetString() << " not loaded." << std::endl;
            continue;
        }

        auto node = data.NodeFactory->at(type)(position);
        auto& nodeData = node->GetData();
        nodeData.Address = IPAddress::FromString(ip);
        nodeData.Id = static_cast<uint16_t>(std::stoi(id.GetString()));

        data.Network->AddNode(std::move(node), nodeData.Address);
    }

    //Load edges
    const auto& edges = document["Edges"].GetObject();
    for (auto& [id, edgeValue] : edges)
    {
        uint16_t nodeA = edgeValue["NodeA"].GetUint();
        uint16_t nodeB = edgeValue["NodeB"].GetUint();
        int speed = edgeValue["Speed"].GetInt();

        NodeNetwork::EdgeData edgeData{
            .NodeA = nodeA,
            .NodeB = nodeB,
            .Speed = speed
        };

        data.Network->AddEdge(edgeData);
    }

    // After all topology is restored, run a full network-wide sync so every
    // router has a complete view of the network before any message is sent.
    data.Network->SyncNetwork();

    return;
}
