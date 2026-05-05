#ifndef ENGINE_TYPES_HPP
#define ENGINE_TYPES_HPP

#include <cstdint>
#include <string>
#include <sys/types.h>
#include <vector>
#include "Vector.hpp"
#include "raylib.h"

Vec2 operator+(Vec2 vec, Vector2 RaylibVec2);
Vec2 operator-(Vec2 vec, Vector2 RaylibVec2);
bool operator==(Vec2 vec, Vector2 RaylibVec2);
Vec2 ConvertRayVec2(Vector2 RaylibVec2);


struct EnvWindow{
    uint Width;
    uint Height;
    uint FrameRate;
};
struct EnvSandbox{
    const uint Width;
    const uint Height;
};

struct Background{
    std::string TexturePath;
    // Optional width and height for the background texture, if not provided, it will be loaded with its original size
    uint Width{0};
    // Optional width and height for the background texture, if not provided, it will be loaded with its original size
    uint Height{0};

    bool operator==(const Background& other);
};

struct Enviroment{
    Background Background;
    EnvWindow Window;
    EnvSandbox Sandbox;

    std::string Title;
    float DeltaTime;
};

struct EditMode{
    bool Enabled;
    std::string SelcectedNode;
};


struct SandboxData{
    Vec2 Camera;
    double Zoom;
    EditMode Edit;
};

struct Edge{
    enum class EdgeType{
        Directed,
        Undirected
    } Type;

    uint16_t NodeA;
    uint16_t NodeB;
    uint16_t Id;
    int Weight;
};

class NodeAbstract{
protected:
    Vec2 Position; // Position of the node in the sandbox
    uint16_t Id;
    std::vector<Edge*> Edges; // List of edges connected to this node

public:
    //Common Methods
    NodeAbstract(Vec2 position) : Position(position) {}

    Vec2 GetPosition() const;
    Vec2 GetScreenPosition(const Vec2 Camera) const;
    void SetId(const uint16_t id);

    // Virtual Methods
    virtual Color GetColor() = 0;
    virtual ~NodeAbstract() = default;
};

struct Nodes{
    // List of all nodes in the sandbox, index corresponds to node ID
    std::vector<std::unique_ptr<NodeAbstract>> NodeList;
    std::vector<uint16_t> FreeNodeIds; // List of free node IDs for reuse

    std::vector<Edge> EdgeList;
    std::vector<uint16_t> FreeEdgeIds; // List of free edge IDs for reuse

    void AddNode(std::unique_ptr<NodeAbstract> node);
    void RemoveNode(const uint nodeId);

    void AddEdge(const Edge::EdgeType type, const uint nodeAId, const uint nodeBId, const int weight);
    void RemoveEdge(const uint nodeAId, const uint nodeBId);

private:

    uint16_t NextNodeId = 0; // Counter for assigning unique node IDs
    uint16_t NextEdgeId = 0; // Counter for assigning unique edge IDs
    uint16_t GetNextNodeId();
    uint16_t GetNextEdgeId();
};

#endif // ENGINE_TYPES_HPP
