#ifndef NODE_HPP
#define NODE_HPP

#include <map>
#include <queue>
#include <vector>
#include <cstdint>
#include <Vector.hpp>

struct IPAddress{
    uint8_t octets[4] = {0, 0, 0, 0};

    std::string ToString() const;
    static IPAddress FromString(const std::string& str);

    bool operator==(const IPAddress& other) const;
    bool compareArea(const IPAddress& other) const; // Compare only the first two octets, used for routing
};

#define LinkSpeeds \
        ENTRY(SLOW, 10.0) \
        ENTRY(MEDIUM,100.0) \
        ENTRY(FAST,1000.0)
#define LinkSpeedsCount 3


enum class LinkSpeed{
    #define ENTRY(name,speed) name,
    LinkSpeeds
    #undef ENTRY
};

double GetSpeedMbps(LinkSpeed speed);


class Edge{
    double weight{1.0};

public:
    //Used for weight calculation, not actual speed of the connection, used for routing algorithms
    LinkSpeed Speed;

    struct EdgeKey{
        uint16_t Id;

        uint16_t NodeA;
        uint16_t NodeB;

        IPAddress AddressA;
        IPAddress AddressB;

    } Key;

    bool Active{false};

    Edge(EdgeKey key, LinkSpeed speed = LinkSpeed::MEDIUM);
    double GetWeight() const;
};

struct NodePosition{
    Vec2 Position;
    Vec2 TransformPosition(const Vec2 Camera) const;
    float Radius = 20.0f;
};

struct Message{
    IPAddress SenderAddress;
    IPAddress ReceiverAddress;

    uint16_t DestinationId{0}; // Final destination of the message, used for routing
    uint16_t OriginId{0}; // Original sender of the message, used for routing and response messages

    std::string Content;
};

#define ClickEvents \
        ENTRY(REMOVE) \
        ENTRY(ADD_EDGE) \
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
  IPAddress Address; // IP address of the node, used for routing and identification
  std::string Type;
  std::vector<uint16_t> Edges{};
  std::queue<Message> MessageQueue{};
  Message currentMessage;
  class NodeNetwork* Network{nullptr}; // Pointer to the network the node is in, used for sending messages
};

// Basic operations that can be performed on nodes, such as sending messages and checking for collisions
class BasicNodeOperations{
public:

    static void SendMessage(Message message, class NodeNetwork& nodes);
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


class NodeNetwork{
    std::map<uint16_t, std::unique_ptr<INode>> nodes{};
    std::map<uint16_t, Edge> edges{};

    std::unordered_map<std::string, uint16_t> ipAddressMap{};


    uint16_t nodeIdCounter{1};
    uint16_t edgeIdCounter{1};

    struct SelectedNodes{
        uint16_t NodeA{0};
        uint16_t NodeB{0};
    } selectedNodes;

public:
    void AddNode(std::unique_ptr<INode> node, IPAddress networkArea = IPAddress{});
    void RemoveNode(const uint16_t id);
    LinkSpeed SelectedLinkSpeed{LinkSpeed::MEDIUM};

    struct EdgeData{
        uint16_t NodeA;
        uint16_t NodeB;
        int Speed{-1};
    };

    void AddEdge(EdgeData edge);
    void RemoveEdge(const uint16_t id);

    INode* GetNode(const uint16_t id) const;
    INode* GetNode(const IPAddress address) const;
    Edge* GetEdge(const uint16_t id);

    // Get all nodes in the same area as the given IP address, used for routing, first two octets of the IP address is used to determine the area
    std::vector<INode*> GetArea(const IPAddress address) const;

    bool IsNodeSelected(uint16_t id) const;

    std::map<uint16_t, std::unique_ptr<INode>>& GetNodeMap();
    std::map<uint16_t,Edge>& GetEdgeMap();

    bool ProcessNodeClick(const Vec2 MousePos, Event::Click clickEvent = Event::Click::NONE);

    double GetWeight(const uint16_t nodeA, const uint16_t nodeB) const;
    double GetWeight(const IPAddress addressA, const IPAddress addressB) const;

    void ClearSelectedNodes();
    void ClearEdgesFromSelectedNode();
};


#endif // NODE_HPP
