#include <Node.hpp>
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <iostream>
#include <string>
#include <ranges>

double GetSpeedMbps(LinkSpeed speed = LinkSpeed::MEDIUM) {
    #define ENTRY(name, speedValue) if(LinkSpeed::name == speed) return speedValue;
    LinkSpeeds
    #undef ENTRY
    return 100.0; // Default to MEDIUM if speed not found
}

Edge::Edge(EdgeKey key, LinkSpeed speed) : Key(key), Speed(speed) {
    constexpr double referenceBandwidth{10000.0}; // Reference bandwidth in Mbps for weight calculation
    weight = referenceBandwidth / GetSpeedMbps(speed);
}

double Edge::GetWeight() const {
    return weight;
}

//NodePosition
Vec2 NodePosition::TransformPosition(const Vec2 Camera) const{
    return Position - Camera;
}

//BasicNodeOperations
void BasicNodeOperations::SendMessage(Message message, class NodeNetwork& nodes) {
    auto senderNode{nodes.GetNode(message.SenderAddress)};
    auto receiverNode{nodes.GetNode(message.ReceiverAddress)};
    if (senderNode == nullptr || receiverNode == nullptr) {
        std::cerr << "Message send failed: sender or receiver node not found by IP address.\n";
        return;
    }

    auto it = std::find(senderNode->GetData().Edges.begin(), senderNode->GetData().Edges.end(), receiverNode->GetData().Id);
    if (it == senderNode->GetData().Edges.end()) {
        std::cout << "NodeNetwork " << senderNode->GetData().Id << " and " << receiverNode->GetData().Id << " are not directly connected ..." << std::endl;
        return;
    };

    nodes.GetEdge(*it)->Active = true;
    senderNode->GetData().FlashTimer = 0.25f;
    nodes.GetNode(message.ReceiverAddress)->PushMessage(message);
}

Message BasicNodeOperations::GetNextMessage(INode &node) {
    auto data{node.GetData()};
    if (data.MessageQueue.empty()) return Message{0, 0, 0};

    data.currentMessage = data.MessageQueue.front();
    data.MessageQueue.pop();
    return data.currentMessage;
}

Message BasicNodeOperations::GetCurrentMessage(INode &node) {
    return node.GetData().currentMessage;
}

bool BasicNodeOperations::IsColliding(const Vec2 point, INode &node) {
    const auto nodePos{node.GetPosition()};
    const auto radius{node.GetRadius()};

    const auto dx{point[0] - nodePos[0]};
    const auto dy{point[1] - nodePos[1]};
    return (dx * dx + dy * dy) <= (radius * radius);
}

//IPAddress
std::string IPAddress::ToString() const {
    auto result{std::to_string(octets[0])};
    for (size_t i = 1; i < 4; ++i) {
        result += "." + std::to_string(octets[i]);
    }
    return result;
}

IPAddress IPAddress::FromString(const std::string& str) {
    IPAddress result{};
    size_t start = 0;
    size_t end = str.find('.');

    for (size_t i = 0; i < 4; ++i) {
        if (end == std::string::npos && i < 3) {
            std::cerr << "Invalid IP address format: " << str << "\n";
            break;
        }
        result.octets[i] = static_cast<uint8_t>(std::stoi(str.substr(start, end - start)));
        start = end + 1;
        end = str.find('.', start);
    }
    return result;
}

bool IPAddress::compareArea(const IPAddress& other) const {
    return octets[0] == other.octets[0] && octets[1] == other.octets[1];
}

bool IPAddress::operator==(const IPAddress& other) const {
    for (size_t i = 0; i < 4; ++i) {
        if (octets[i] != other.octets[i]) return false;
    }
    return true;
}

//NodeNetwork
void NodeNetwork::AddNode(std::unique_ptr<INode> node, IPAddress networkArea) {

    auto CheckForIPConflict = [&](const IPAddress& address) -> bool {
        for (const auto& node : nodes | std::views::values) {
            if (node->GetData().Address == address) return true;
        }
        return false;
    };

    auto areaSize{GetArea(networkArea).size()};
    auto& data = node->GetData();

    //Loaded node from save file
    if (data.Id > 0){
        nodeIdCounter = data.Id;
    }

    while (areaSize >=255) {
        if (networkArea.octets[2] >= 255) {
            std::cerr << "Error: Network area " << networkArea.ToString() << " is full. Cannot add more nodes to this area.\n";
            return;
        }
        networkArea.octets[2]++;
        areaSize -= 255;
    }

    //Assign its own ip while loading from save file, to prevent ip conflicts, if octet 3 is 0, assign it to areaSize + 1, otherwise keep the loaded ip, but check if it's not conflicting with existing nodes in the area
    networkArea.octets[3] = networkArea.octets[3] == 0 ? static_cast<uint8_t>(areaSize + 1): networkArea.octets[3];
    while (CheckForIPConflict(networkArea)) {
        networkArea.octets[3]++;
        if (networkArea.octets[3] == 0) {
            std::cerr << "Error: Network area " << networkArea.ToString() << " is full. Cannot add more nodes to this area.\n";
            return;
        }
    }

    data.Id = nodeIdCounter;
    data.Address = networkArea;
    data.Network = this;



    nodes.insert_or_assign(nodeIdCounter, std::move(node));
    ipAddressMap.insert_or_assign(data.Address.ToString(), data.Id);
    nodeIdCounter++;
};

void NodeNetwork::RemoveNode(const uint16_t id){
    if (!nodes.contains(id)) return;

    ipAddressMap.erase(nodes.at(id)->GetData().Address.ToString());
    std::vector<uint16_t> edgesToRemove = GetNode(id)->GetData().Edges;

    for (const auto& edgeId : edgesToRemove)
        RemoveEdge(edgeId);

    nodes.erase(id);
};

void NodeNetwork::AddEdge(EdgeData edge){
    auto nodeA = GetNode(edge.NodeA);
    auto nodeB = GetNode(edge.NodeB);
    if (nodeA == nullptr || nodeB == nullptr) {
        std::cerr << "Error: One or both nodes not found for edge. NodeA id: " << edge.NodeA << ", NodeB id: " << edge.NodeB << ". Cannot add edge.\n";
        return;
    }

    Edge::EdgeKey key{
        .Id = edgeIdCounter++,
        .NodeA = edge.NodeA,
        .NodeB = edge.NodeB,
        .AddressA = nodeA->GetData().Address,
        .AddressB = nodeB->GetData().Address,

    };

    for (auto Edge : edges | std::views::values)
        if ((Edge.Key.NodeA == key.NodeA && Edge.Key.NodeB == key.NodeB) ||
            (Edge.Key.NodeA == key.NodeB && Edge.Key.NodeB == key.NodeA)) {
            std::cerr << "Error: Edge between node " << key.NodeA << " and node " << key.NodeB << " already exists. Cannot add duplicate edge.\n";
            return;
        }

    auto speed{(edge.Speed >= 0 && edge.Speed < LinkSpeedsCount) ? static_cast<LinkSpeed>(edge.Speed): SelectedLinkSpeed};
    auto edgeObj{Edge(key, speed)};


    edges.insert_or_assign(key.Id, edgeObj);

    //Push edge to both nodes
    GetNode(edge.NodeA)->GetData().Edges.push_back(key.Id);
    GetNode(edge.NodeB)->GetData().Edges.push_back(key.Id);

    const Edge& storedEdge = edges.at(key.Id);
    GetNode(edge.NodeA)->OnEdgeAdded(GetNode(edge.NodeB), storedEdge);
    GetNode(edge.NodeB)->OnEdgeAdded(GetNode(edge.NodeA), storedEdge);

    // Guarantee full LSA propagation across all routers after topology change
    SyncNetwork();
}

void NodeNetwork::RemoveEdge(const uint16_t id){
    if (edges.find(id) == edges.end()) return;

    const auto edge{edges.at(id)};

    auto* nodeA = GetNode(edge.Key.NodeA);
    auto* nodeB = GetNode(edge.Key.NodeB);

    //Remove edge from both nodes
    auto& nodeAEdges{nodeA->GetData().Edges};
    nodeAEdges.erase(std::remove(nodeAEdges.begin(), nodeAEdges.end(), id), nodeAEdges.end());

    auto& nodeBEdges{nodeB->GetData().Edges};
    nodeBEdges.erase(std::remove(nodeBEdges.begin(), nodeBEdges.end(), id), nodeBEdges.end());

    edges.erase(id);

    // Notify both nodes so they can update their topology databases
    nodeA->OnEdgeRemoved(nodeB, edge);
    nodeB->OnEdgeRemoved(nodeA, edge);

    // Re-sync the whole network so everyone learns about the removed link
    SyncNetwork();
}

INode* NodeNetwork::GetNode(const uint16_t id) const {
    if (nodes.find(id) == nodes.end()) return nullptr;
    return nodes.at(id).get();
};

INode* NodeNetwork::GetNode(const IPAddress address) const {
    const auto ipstring {address.ToString()};
    if (ipAddressMap.find(ipstring) == ipAddressMap.end()) return nullptr;

    return GetNode(ipAddressMap.at(ipstring));
}

Edge* NodeNetwork::GetEdge(const uint16_t id) {
    if (edges.find(id) == edges.end()) return nullptr;

    return &edges.at(id);
}


bool NodeNetwork::ProcessNodeClick(const Vec2 MousePos, Event::Click clickEvent){
    uint16_t nodeId{0};

    auto UpdateNodeSelection = [&]() -> SelectedNodes{
        uint16_t nodeA = selectedNodes.NodeA== 0 ?  nodeId : selectedNodes.NodeA;
        uint16_t nodeB =  nodeA == nodeId ? 0 : nodeId;
        return {
            .NodeA = nodeA,
            .NodeB = nodeB
        };
    };

    for (auto& node: nodes | std::views::values) {
        if (node == nullptr) continue;
        if (BasicNodeOperations::IsColliding(MousePos, *node)) {
            //TODO: Najde ID a implementovat NodeClicked() pro různé akce (odstranění, přidání hrany, atd.)
            nodeId = node->GetData().Id;
            break;
        }
    }
    if (nodeId == 0) return false;

    if (clickEvent == Event::Click::NONE) {
        GetNode(nodeId)->NodeClicked();
        ClearSelectedNodes();
        selectedNodes.NodeA = nodeId;
    }
    else if (clickEvent == Event::Click::REMOVE)
    {
        selectedNodes = UpdateNodeSelection();
        RemoveNode(nodeId);
        ClearSelectedNodes();
    }
    else if (clickEvent == Event::Click::ADD_EDGE){
        selectedNodes = UpdateNodeSelection();
        if (selectedNodes.NodeA != 0 && selectedNodes.NodeB != 0){
        AddEdge(EdgeData{.NodeA = selectedNodes.NodeA, .NodeB = selectedNodes.NodeB});
        ClearSelectedNodes();
        }
    }
    else if (clickEvent == Event::Click::SEND_MESSAGE){
        selectedNodes = UpdateNodeSelection();
        if (selectedNodes.NodeA != 0 && selectedNodes.NodeB != 0) {
            pendingMessageSelection = selectedNodes;
            ClearSelectedNodes();
        }
    }
    return true;
}

void NodeNetwork::ClearSelectedNodes() {
    selectedNodes.NodeA = 0;
    selectedNodes.NodeB = 0;
}

bool NodeNetwork::IsNodeSelected(uint16_t id) const{
    return selectedNodes.NodeA == id || selectedNodes.NodeB == id;
}

std::map<uint16_t, std::unique_ptr<INode>>& NodeNetwork::GetNodeMap() {
    return nodes;
}

std::map<uint16_t, Edge>& NodeNetwork::GetEdgeMap() {
    return edges;
}

double NodeNetwork::GetWeight(const uint16_t nodeA, const uint16_t nodeB) const{
    double result{-1.0};
    auto edgesA = GetNode(nodeA)->GetData().Edges;
    for (const auto& edgeId : edgesA)
        if (const auto& edge = edges.at(edgeId); edge.Key.NodeB == nodeB) {
            result = edge.GetWeight();
            break;
        }

    return result;
};

double NodeNetwork::GetWeight(const IPAddress addressA, const IPAddress addressB) const{
    const auto nodeA {GetNode(addressA)};
    const auto nodeB {GetNode(addressB)};

    if (nodeA == nullptr || nodeB == nullptr) return -1.0;

    return GetWeight(nodeA->GetData().Id, nodeB->GetData().Id);
};

std::vector<INode*> NodeNetwork::GetArea(const IPAddress address) const {
    std::vector<INode*> result;
    for (const auto& [id, node] : nodes)
        if (node->GetData().Address.octets[0] == address.octets[0] &&
            node->GetData().Address.octets[1] == address.octets[1])
            result.push_back(node.get());

    return result;
}

INode* NodeNetwork::GetSelectedNode() const {
    return nodes.find(selectedNodes.NodeA) != nodes.end() ? nodes.at(selectedNodes.NodeA).get() : nullptr;
}

std::optional<SelectedNodes> NodeNetwork::ConsumeMessageSelection() {
    if (!pendingMessageSelection.has_value()) return std::nullopt;
    auto selection = pendingMessageSelection;
    pendingMessageSelection.reset();
    return selection;
}

void NodeNetwork::FlashEdgeBetween(uint16_t nodeA, uint16_t nodeB, float durationSeconds) {
    for (auto& [edgeId, edge] : edges) {
        const bool sameDirection = edge.Key.NodeA == nodeA && edge.Key.NodeB == nodeB;
        const bool oppositeDirection = edge.Key.NodeA == nodeB && edge.Key.NodeB == nodeA;
        if (sameDirection || oppositeDirection) {
            edge.RouteFlashTimer = std::max(edge.RouteFlashTimer, durationSeconds);
            return;
        }
    }
}

void NodeNetwork::SyncNetwork() {
    bool anyUpdates = true;
    while (anyUpdates) {
        anyUpdates = false;
        for (const auto& [id, node] : nodes) {
            if (node && node->NetworkSync() > 0)
                anyUpdates = true;
        }
    }
}

