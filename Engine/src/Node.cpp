#include "Node.hpp"
#include <cstddef>
#include <cstdint>
#include <memory>
#include <iostream>
#include <ranges>


//NodePosition
Vec2 NodePosition::TransformPosition(const Vec2 Camera) const{
    return Position - Camera;
}

//BasicNodeOperations
void BasicNodeOperations::SendMessage(Message message, class Nodes& nodes) {
    auto senderNode{nodes.GetNode(message.SenderId)};
    auto receiverNode{nodes.GetNode(message.ReceiverId)};

    auto it = std::find(senderNode->GetData().Edges.begin(), senderNode->GetData().Edges.end(), receiverNode->GetData().Id);
    if (it == senderNode->GetData().Edges.end()) {
        std::cout << "Nodes " << senderNode->GetData().Id << " and " << receiverNode->GetData().Id << " are not directly connected. Message cannot be sent." << std::endl;
        //TODO: Implement routing algorithm to find path from sender to receiver and push message to the first node in the path
        return;
    };

    nodes.GetEdge(*it)->Active = true;
    nodes.GetNode(message.ReceiverId)->PushMessage(message);
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

//Nodes
void Nodes::AddNode(std::unique_ptr<INode> node){
    NodeData data{
        .Id = nodeIdCounter,
        .Network = this};

    node->UpdateNodeData(data);
    nodes.insert_or_assign(nodeIdCounter, std::move(node));

    nodeIdCounter++;
};

void Nodes::RemoveNode(const uint16_t id){
    if (!nodes.contains(id)) return;

    std::vector<uint16_t> edgesToRemove = GetNode(id)->GetData().Edges;

    for (const auto& edgeId : edgesToRemove)
        RemoveEdge(edgeId);

    nodes.erase(id);
};

void Nodes::AddEdge(Edge edge){
    edge.Id = edgeIdCounter++;
    edges.insert_or_assign(edge.Id, edge);

    //Push edge to both nodes
    GetNode(edge.NodeA)->GetData().Edges.push_back(edge.Id);
    GetNode(edge.NodeB)->GetData().Edges.push_back(edge.Id);
}

void Nodes::RemoveEdge(const uint16_t id){
    if (edges.find(id) == edges.end()) return;

    const auto edge{edges.at(id)};

    //Remove edge from both nodes
    auto& nodeAEdges{GetNode(edge.NodeA)->GetData().Edges};
    nodeAEdges.erase(std::remove(nodeAEdges.begin(), nodeAEdges.end(), id), nodeAEdges.end());

    auto& nodeBEdges{GetNode(edge.NodeB)->GetData().Edges};
    nodeBEdges.erase(std::remove(nodeBEdges.begin(), nodeBEdges.end(), id), nodeBEdges.end());

    edges.erase(id);
}

INode* Nodes::GetNode(const uint16_t id) const {
    if (nodes.find(id) == nodes.end()) return nullptr;

    return nodes.at(id).get();
};

Edge* Nodes::GetEdge(const uint16_t id) {
    if (edges.find(id) == edges.end()) return nullptr;

    return &edges.at(id);
}


bool Nodes::ProcessNodeClick(const Vec2 MousePos, ClickEvent clickEvent){
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

    if (clickEvent == ClickEvent::NONE) {
        GetNode(nodeId)->NodeClicked();
        ClearSelectedNodes();
        selectedNodes.NodeA = nodeId;
    }
    else if (clickEvent == ClickEvent::REMOVE)
    {
        selectedNodes = UpdateNodeSelection();
        RemoveNode(nodeId);
        ClearSelectedNodes();
    }
    else if (clickEvent == ClickEvent::ADD_EDGE){
        selectedNodes = UpdateNodeSelection();
        if (selectedNodes.NodeA != 0 && selectedNodes.NodeB != 0){
        AddEdge(Edge{.NodeA = selectedNodes.NodeA, .NodeB = selectedNodes.NodeB});
        ClearSelectedNodes();
        }
    }
    //Pridat vektor pro ukládání dvou kliknutých uzlů pro přidání hrany
    return true;
}

void Nodes::ClearSelectedNodes() {
    selectedNodes.NodeA = 0;
    selectedNodes.NodeB = 0;
}

bool Nodes::IsNodeSelected(uint16_t id) const{
    return selectedNodes.NodeA == id || selectedNodes.NodeB == id;
}

std::map<uint16_t, std::unique_ptr<INode>>& Nodes::GetNodeMap() {
    return nodes;
}

std::map<uint16_t, Edge>& Nodes::GetEdgeMap() {
    return edges;
}
