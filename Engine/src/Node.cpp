#include "Node.hpp"
#include <cstddef>
#include <cstdint>
#include <memory>
#include <utility>
#include <iostream>


//NodePosition
Vec2 NodePosition::TransformPosition(const Vec2 Camera) const{
    return Position - Camera;
}

//BasicNodeOperations
void BasicNodeOperations::SendMessage(Message message, class Nodes& nodes) {
    return; //Not implemented yet, need to implement routing first
    //Get path from LSDB
    if (nodes.GetNode(message.ReceiverId) == nullptr) return;
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
    nodeList.push_back(std::move(node));
    nodes.insert_or_assign(nodeIdCounter, nodeList.size() - 1);

    nodeIdCounter++;
};

void Nodes::RemoveNode(const uint16_t id){
    if (!nodes.contains(id)) return;

    std::vector<uint16_t> edgesToRemove = GetNode(id)->GetData().Edges;

    for (const auto& edgeId : edgesToRemove)
        RemoveEdge(edgeId);

    const auto index{nodes.at(id)};
    nodeList.at(index).reset();
    nodes.erase(id);
};

void Nodes::AddEdge(Edge edge){
    edge.Id = edgeIdCounter++;
    edgeList.push_back(edge);
    edges.insert_or_assign(edge.Id, edgeList.size() - 1);

    //Push edge to both nodes
    GetNode(edge.NodeA)->GetData().Edges.push_back(edge.Id);
    GetNode(edge.NodeB)->GetData().Edges.push_back(edge.Id);
}

void Nodes::RemoveEdge(const uint16_t id){
    if (edges.find(id) == edges.end()) return;

    const auto index{edges.at(id)};
    const auto edge{edgeList.at(index)};

    //Remove edge from both nodes
    auto& nodeAEdges{GetNode(edge.NodeA)->GetData().Edges};
    nodeAEdges.erase(std::remove(nodeAEdges.begin(), nodeAEdges.end(), id), nodeAEdges.end());

    auto& nodeBEdges{GetNode(edge.NodeB)->GetData().Edges};
    nodeBEdges.erase(std::remove(nodeBEdges.begin(), nodeBEdges.end(), id), nodeBEdges.end());

    edgeList.erase(edgeList.begin() + index);
    edges.erase(id);
}

INode* Nodes::GetNode(const uint16_t id) const {
    if (nodes.find(id) == nodes.end()) return nullptr;

    const auto index{nodes.at(id)};
    return nodeList.at(index).get();
};

Edge* Nodes::GetEdge(const uint16_t id) {
    if (edges.find(id) == edges.end()) return nullptr;

    const auto index{edges.at(id)};
    return &edgeList.at(index);
}

std::vector<std::unique_ptr<INode>>& Nodes::GetNodeList(){
    return nodeList;
};

std::vector<Edge> Nodes::GetEdgeList(){
    return edgeList;
};

bool Nodes::ProcessNodeClick(const Vec2 MousePos, ClickEvent clickEvent){
    uint16_t nodeId{0};

    for (auto& node: nodeList) {
        if (node == nullptr) continue;
        if (BasicNodeOperations::IsColliding(MousePos, *node)) {
            //TODO: Najde ID a implementovat NodeClicked() pro různé akce (odstranění, přidání hrany, atd.)
            nodeId = node->GetData().Id;
            break;
        }
    }
    if (nodeId == 0) return false;

    if (clickEvent == ClickEvent::NONE) GetNode(nodeId)->NodeClicked();
    else if (clickEvent == ClickEvent::REMOVE) RemoveNode(nodeId);
    else if (clickEvent == ClickEvent::ADD_EDGE){
        if (selectedNodeId == 0){
            selectedNodeId = nodeId;
            std::cout << "Selected node " << selectedNodeId << " for edge creation." << std::endl;
        }
        else{
            AddEdge(Edge{.NodeA = selectedNodeId, .NodeB = nodeId});
            selectedNodeId = 0;
        }
    }
    //Pridat vektor pro ukládání dvou kliknutých uzlů pro přidání hrany
    return true;
}

void Nodes::ClearSelectedNode() {
    selectedNodeId = 0;
}
