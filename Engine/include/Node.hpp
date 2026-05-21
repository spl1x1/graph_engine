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
    bool Active{false};
};

struct NodePosition{
    Vec2 Position;
    Vec2 TransformPosition(const Vec2 Camera) const;
    float Radius = 20.0f;
};

struct Message{
    uint16_t SenderId{0};
    uint16_t ReceiverId{0};

    uint16_t DestinationId{0}; // Final destination of the message, used for routing
    uint16_t OriginId{0}; // Original sender of the message, used for routing and response messages

    std::string Content;
};

#define ClickEvents \
        ENTRY(REMOVE) \
        ENTRY(ADD_EDGE) \
        ENTRY(ADD_EDGE_DIRECTED) \
        ENTRY(NONE)

struct Event{
    enum class Click{
    #define ENTRY(name) name,
    ClickEvents
    #undef ENTRY
    };

    static std::string ClickToString(Click click){
        switch (click){
        #define ENTRY(name) case Click::name: return #name;
        ClickEvents
        #undef ENTRY
        }
    }

    static Click StringToClick(const std::string& str){
        #define ENTRY(name) if (str == #name) return Click::name;
        ClickEvents
        #undef ENTRY
        return Click::NONE;
    }
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
    std::map<uint16_t, std::unique_ptr<INode>> nodes{};
    std::map<uint16_t, Edge> edges{};

    uint16_t nodeIdCounter{1};
    uint16_t edgeIdCounter{1};

    struct SelectedNodes{
        uint16_t NodeA{0};
        uint16_t NodeB{0};
    } selectedNodes;

public:
    void AddNode(std::unique_ptr<INode> node);
    void RemoveNode(const uint16_t id);

    void AddEdge(Edge edge);
    void RemoveEdge(const uint16_t id);

    INode* GetNode(const uint16_t id) const;
    Edge* GetEdge(const uint16_t id);

    bool IsNodeSelected(uint16_t id) const;

    std::map<uint16_t, std::unique_ptr<INode>>& GetNodeMap();
    std::map<uint16_t,Edge>& GetEdgeMap();

    bool ProcessNodeClick(const Vec2 MousePos, Event::Click clickEvent = Event::Click::NONE);

    void ClearSelectedNodes();
    void ClearEdgesFromSelectedNode();
};


#endif // NODE_HPP
