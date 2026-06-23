#include "Router.hpp"
#include "Node.hpp"
#include <iostream>

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
        std::cerr << "Error: Router with id " << Data.Id << " is not connected to a network. Message not sent. \n";
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

    std::cout << "\n════════════════════════════════════════════════════════════════════\n";

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

void Router::OnEdgeAdded(INode* neighbor, const Edge& edge) {
    if (Data.Id == 0 || neighbor == nullptr) return;

    // Build a LinkStateEntry for this new connection
    LinkStateEntry entry;
    entry.LinkId = edge.Key.Id;
    entry.SourceNodeId = Data.Id;
    entry.DestinationNodeId = neighbor->GetData().Id;
    entry.SourceAddress = Data.Address;
    entry.DestinationAddress = neighbor->GetData().Address;
    entry.LinkCost = edge.GetWeight();
    entry.LinkActive = true;
    entry.Speed = edge.Speed;

    // Get existing local LSA or start a fresh one
    const LinkStateAdvertisement* existing = topology.GetLSA(Data.Id);
    LinkStateAdvertisement updatedLSA = existing ? *existing : LinkStateAdvertisement(Data.Id);

    updatedLSA.AddLink(entry);
    updatedLSA.IncrementSequenceNumber();
    updatedLSA.ResetAge();
    updatedLSA.CalculateChecksum();

    topology.AddOrUpdateLSA(updatedLSA);
    topology.AddNeighbor(neighbor->GetData().Id);

    std::cout << "Router " << Data.Id << " registered new link to Router "
              << neighbor->GetData().Id << " (edge " << edge.Key.Id << ")\n";

    // Flood own LSA to all known neighbors
    SyncWithNetwork();
}

void Router::OnEdgeRemoved(INode* neighbor, const Edge& edge) {
    if (Data.Id == 0 || neighbor == nullptr) return;

    // Remove the link entry for this edge from our local LSA
    LinkStateAdvertisement* localLSA = topology.GetLSA(Data.Id);
    if (localLSA != nullptr) {
        localLSA->RemoveLink(edge.Key.Id);
        localLSA->IncrementSequenceNumber();
        localLSA->ResetAge();
        localLSA->CalculateChecksum();
        topology.AddOrUpdateLSA(*localLSA);
    }

    const uint16_t neighborId = neighbor->GetData().Id;

    // If we have no more links to this neighbor, remove their LSA and neighbor entry
    bool stillConnected = false;
    const LinkStateAdvertisement* updatedLSA = topology.GetLSA(Data.Id);
    if (updatedLSA != nullptr) {
        for (const auto& link : updatedLSA->GetLinks()) {
            if (link.DestinationNodeId == neighborId && link.LinkActive) {
                stillConnected = true;
                break;
            }
        }
    }

    if (!stillConnected) {
        topology.RemoveLSA(neighborId);
        topology.RemoveNeighbor(neighborId);
    }

    std::cout << "Router " << Data.Id << " removed link to Router "
              << neighborId << " (edge " << edge.Key.Id << ")\n";

    // Flood updated LSA to remaining neighbors
    SyncWithNetwork();
}

uint32_t Router::SyncWithNetwork() {
    if (Data.Network == nullptr) return 0;

    const auto& allLSAs = topology.GetAllLSAs();
    if (allLSAs.empty()) {
        std::cout << "Router " << Data.Id << ": no LSAs to flood\n";
        return 0;
    }

    const auto& neighbors = topology.GetNeighbors();
    uint32_t flooded = 0;

    std::cout << "\n" << std::string(70, '=') << "\n";
    std::cout << "FLOOD - Router " << Data.Id << " → " << neighbors.size() << " neighbor(s)\n";
    std::cout << std::string(70, '=') << "\n";

    for (uint16_t neighborId : neighbors) {
        if (neighborId == Data.Id) continue;
        INode* neighborNode = Data.Network->GetNode(neighborId);
        if (auto* neighborRouter = dynamic_cast<Router*>(neighborNode)) {
            for (const auto& [routerId, lsa] : allLSAs) {
                if (neighborRouter->GetLSDB().AddOrUpdateLSA(lsa)) {
                    neighborRouter->GetLSDB().AddNeighbor(Data.Id);
                    std::cout << "   → Flooded LSA (router=" << routerId << ", seq=" << lsa.GetSequenceNumber()
                              << ") to Router " << neighborId << "\n";
                    flooded++;
                }
            }
        }
    }

    std::cout << "✓ Flooded " << flooded << " LSA(s) to " << neighbors.size() << " neighbor(s)\n";
    std::cout << std::string(70, '=') << "\n\n";
    return flooded;
}
