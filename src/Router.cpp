#include "Router.hpp"
#include "Node.hpp"
#include <iostream>
#include <iomanip>

//Router

void Router::UpdateNodeData(NodeData data){
    Data = data;
    // Initialize LSDB with router ID when data is updated
    if (Data.Id > 0) {
        topology.Initialize(Data.Id);
    }
}

float Router::GetRadius() const{
    return Position.Radius;
}

Vec2 Router::GetPosition() const{
    return Position.Position;
}

Vec2 Router::GetScreenPosition(const Vec2 Camera) const{
    return Position.TransformPosition(Camera);
}

void Router::SendMessage(Message message){
    if (Data.Network == nullptr) {
        std::cerr << "Error: Router with id " << Data.Id << " is not connected to a network. Message not sent." << std::endl;
        return;
    };
    message.SenderAddress = Data.Address;
    BasicNodeOperations::SendMessage(message, *Data.Network);
}

void Router::PushMessage(Message message){
    Data.MessageQueue.push(message);
}

NodeData& Router::GetData(){
    return Data;
}

LSDB& Router::GetLSDB() {
    return topology;
}

const LSDB& Router::GetLSDB() const {
    return topology;
}

void Router::InitializeLSDB() {
    if (Data.Id > 0) {
        topology.Initialize(Data.Id);
    }
}

void Router::PrintTopologyDatabase() const {
    std::cout << "\n" << std::string(70, '=') << "\n";
    std::cout << "TOPOLOGY DATABASE STATE FOR ROUTER " << Data.Id << " (" << Data.Address.ToString() << ")\n";
    std::cout << std::string(70, '=') << "\n";
    
    topology.PrintDatabase();
    
    // Print direct neighbors
    std::vector<uint16_t> neighbors = topology.GetDirectNeighbors();
    std::cout << "\nDirect Neighbors from Local LSA: ";
    if (neighbors.empty()) {
        std::cout << "None\n";
    } else {
        for (uint16_t neighbor : neighbors) {
            std::cout << neighbor << " ";
        }
        std::cout << "\n";
    }
    
    // Print known neighbors from neighbor tracking
    const auto& knownNeighbors = topology.GetNeighbors();
    std::cout << "\nKnown Neighbors (from LSAs received): ";
    if (knownNeighbors.empty()) {
        std::cout << "None\n";
    } else {
        for (uint16_t neighbor : knownNeighbors) {
            std::cout << neighbor << " ";
        }
        std::cout << "\n";
    }
    
    std::cout << std::string(70, '=') << "\n";
}

void Router::PrintLSDBStatistics() const {
    std::cout << "\n" << std::string(70, '=') << "\n";
    std::cout << "LSDB STATISTICS FOR ROUTER " << Data.Id << " (" << Data.Address.ToString() << ")\n";
    std::cout << std::string(70, '=') << "\n";
    
    topology.PrintStatistics();
    
    // Additional router-specific information
    std::cout << "\nRouter Information:\n";
    std::cout << "  Router ID: " << Data.Id << "\n";
    std::cout << "  Router Address: " << Data.Address.ToString() << "\n";
    std::cout << "  Router Type: " << Data.Type << "\n";
    std::cout << "  Number of Edges: " << Data.Edges.size() << "\n";
    std::cout << "  Message Queue Size: " << Data.MessageQueue.size() << "\n";
    std::cout << "  Total LSAs in Database: " << topology.GetLSACount() << "\n";
    
    std::cout << std::string(70, '=') << "\n";
}

void Router::NodeClicked() {
    std::cout << "\n╔════════════════════════════════════════════════════════════════════╗\n";
    std::cout << "║                    ROUTER CLICK EVENT                               ║\n";
    std::cout << "╚════════════════════════════════════════════════════════════════════╝\n";
    
    // Basic router information
    std::cout << "Router ID: " << Data.Id << "\n";
    std::cout << "Router Address: " << Data.Address.ToString() << "\n";
    std::cout << "Router Type: " << Data.Type << "\n";
    std::cout << "Connected Edges: " << Data.Edges.size() << "\n";
    std::cout << "Message Queue Size: " << Data.MessageQueue.size() << "\n";
    
    // Print LSDB state
    PrintTopologyDatabase();
    
    // Print LSDB statistics
    PrintLSDBStatistics();
}
