#ifndef NODE_HPP
#define NODE_HPP

#include <map>
#include <queue>
#include <vector>
#include <cstdint>
#include "Vector.hpp"


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

struct NodePosition{
    Vec2 Position;
    Vec2 TransformPosition(const Vec2 Camera) const;
    float Radius = 20.0f;
};

struct Message{
    uint16_t SenderId{0};
    uint16_t ReceiverId{0};
    uint16_t LastNodeId{0};

    std::string Content;
};

enum class ClickEvent{
    REMOVE,
    ADD_EDGE,
    NONE
};

struct NodeData{
  uint16_t Id = 0; // Unique identifier for the node, basically ip address
  std::string Type = "Undefined";
  std::vector<uint16_t> Edges{};
  std::queue<Message> MessageQueue{};
  Message currentMessage;
  class Nodes* Network{nullptr}; // Pointer to the network the node is in, used for sending messages
};

// Basic operations that can be performed on nodes, such as sending messages and checking for collisions
class BasicNodeOperations{
public:

    static void SendMessage(Message message, class Nodes& nodes);
    static Message GetNextMessage(class INode &node);
    static Message GetCurrentMessage(INode &node);
    static bool IsColliding(const Vec2 point, INode &node);
};

class INode{
public:
    virtual void UpdateNodeData(NodeData data) = 0;
    virtual float GetRadius() const = 0;
    virtual Vec2 GetPosition() const = 0;
    virtual Vec2 GetScreenPosition(const Vec2 Camera) const = 0;
    virtual void SendMessage(Message message) = 0;
    virtual void ProccesNextMessage() = 0;
    virtual void PushMessage(Message message) = 0;
    virtual NodeData& GetData() = 0;
    virtual void NodeClicked() = 0;

    virtual ~INode() = default;

    INode& operator=(const INode&) = default;
};

class Nodes{
    std::vector<std::unique_ptr<INode>> nodeList{};
    std::vector<Edge> edgeList{};

    std::map<uint16_t, int> nodes{};
    std::map<uint16_t, int> edges{};

    uint16_t nodeIdCounter{1};
    uint16_t edgeIdCounter{1};

    uint16_t selectedNodeId{0};

public:
    void AddNode(std::unique_ptr<INode> node);
    void RemoveNode(const uint16_t id);

    void AddEdge(Edge edge);
    void RemoveEdge(const uint16_t id);

    INode* GetNode(const uint16_t id) const;
    Edge* GetEdge(const uint16_t id);

    std::vector<std::unique_ptr<INode>>& GetNodeList();
    std::vector<Edge> GetEdgeList();

    bool ProcessNodeClick(const Vec2 MousePos, ClickEvent clickEvent = ClickEvent::NONE);

    void ClearSelectedNode();
    void ClearEdgesFromSelectedNode();
};


#endif // NODE_HPP
