#include <LSDB.hpp>
#include <iostream>
#include <numeric>
#include <cstring>
#include <iomanip>

// ==================== LinkStateAdvertisement Implementation ====================

LinkStateAdvertisement::LinkStateAdvertisement(uint16_t routerId) 
    : OriginatingRouterId(routerId) {
    header.AdvertisingRouter = routerId;
    header.CreationTime = time(nullptr);
    header.SequenceNumber = 0;
    header.Age = 0;
    header.Checksum = 0;
}

const LSAHeader& LinkStateAdvertisement::GetHeader() const {
    return header;
}

LSAHeader& LinkStateAdvertisement::GetHeader() {
    return header;
}

const std::vector<LinkStateEntry>& LinkStateAdvertisement::GetLinks() const {
    return links;
}

std::vector<LinkStateEntry>& LinkStateAdvertisement::GetLinks() {
    return links;
}

uint16_t LinkStateAdvertisement::GetOriginatingRouterId() const {
    return OriginatingRouterId;
}

void LinkStateAdvertisement::AddLink(const LinkStateEntry& entry) {
    // Check if link already exists
    for (auto& link : links) {
        if (link.LinkId == entry.LinkId) {
            std::cerr << "Error: Link with ID " << entry.LinkId << " already exists in LSA\n";
            return;
        }
    }
    links.push_back(entry);
}

void LinkStateAdvertisement::RemoveLink(uint16_t linkId) {
    auto it = std::find_if(links.begin(), links.end(),
        [linkId](const LinkStateEntry& entry) { return entry.LinkId == linkId; });
    
    if (it != links.end()) {
        links.erase(it);
    } else {
        std::cerr << "Warning: Link with ID " << linkId << " not found in LSA\n";
    }
}

bool LinkStateAdvertisement::UpdateLink(uint16_t linkId, const LinkStateEntry& updatedEntry) {
    for (auto& link : links) {
        if (link.LinkId == linkId) {
            link = updatedEntry;
            link.LinkId = linkId; // Preserve the original link ID
            return true;
        }
    }
    return false;
}

const LinkStateEntry* LinkStateAdvertisement::GetLink(uint16_t linkId) const {
    for (const auto& link : links) {
        if (link.LinkId == linkId) {
            return &link;
        }
    }
    return nullptr;
}

void LinkStateAdvertisement::IncrementSequenceNumber() {
    header.SequenceNumber++;
}

uint32_t LinkStateAdvertisement::GetSequenceNumber() const {
    return header.SequenceNumber;
}

void LinkStateAdvertisement::UpdateAge() {
    header.Age++;
}

void LinkStateAdvertisement::ResetAge() {
    header.Age = 0;
    header.CreationTime = time(nullptr);
}

uint16_t LinkStateAdvertisement::GetAge() const {
    return header.Age;
}

void LinkStateAdvertisement::CalculateChecksum() {
    // Simple checksum calculation: XOR of all link costs and IDs
    uint32_t checksum = 0;
    checksum ^= header.SequenceNumber;
    checksum ^= header.AdvertisingRouter;
    
    for (const auto& link : links) {
        checksum ^= link.LinkId;
        checksum ^= link.SourceNodeId;
        checksum ^= link.DestinationNodeId;
        // XOR the bits of the double cost value
        uint64_t costBits;
        std::memcpy(&costBits, &link.LinkCost, sizeof(double));
        checksum ^= static_cast<uint32_t>(costBits);
        checksum ^= static_cast<uint32_t>(costBits >> 32);
    }
    
    header.Checksum = checksum;
}

uint32_t LinkStateAdvertisement::GetChecksum() const {
    return header.Checksum;
}

bool LinkStateAdvertisement::VerifyChecksum() const {
    LinkStateAdvertisement temp = *this;
    temp.CalculateChecksum();
    return temp.header.Checksum == header.Checksum;
}

bool LinkStateAdvertisement::IsStale(uint16_t maxAge) const {
    return header.Age >= maxAge;
}

size_t LinkStateAdvertisement::GetLinkCount() const {
    return links.size();
}

void LinkStateAdvertisement::PrintLSA() const {
    std::cout << "\n========== LSA ==========\n";
    std::cout << "Originating Router ID: " << OriginatingRouterId << "\n";
    std::cout << "Sequence Number: " << header.SequenceNumber << "\n";
    std::cout << "Age: " << header.Age << " seconds\n";
    std::cout << "Checksum: 0x" << std::hex << header.Checksum << std::dec << "\n";
    std::cout << "Number of Links: " << links.size() << "\n";
    
    if (!links.empty()) {
        std::cout << "\nLinks:\n";
        std::cout << std::setw(8) << "LinkID" << std::setw(12) << "Source" 
                  << std::setw(12) << "Dest" << std::setw(10) << "Cost" 
                  << std::setw(10) << "Active\n";
        std::cout << std::string(50, '-') << "\n";
        
        for (const auto& link : links) {
            std::cout << std::setw(8) << link.LinkId
                      << std::setw(12) << link.SourceNodeId
                      << std::setw(12) << link.DestinationNodeId
                      << std::setw(10) << std::fixed << std::setprecision(2) << link.LinkCost
                      << std::setw(10) << (link.LinkActive ? "Yes" : "No") << "\n";
        }
    }
    std::cout << "=======================\n";
}

// ==================== LSDB Implementation ====================

LSDB::LSDB(uint16_t routerId) : LocalRouterId(routerId) {
    statistics.LastUpdateTime = time(nullptr);
}

void LSDB::Initialize(uint16_t routerId) {
    LocalRouterId = routerId;
    database.clear();
    NeighborRouterIds.clear();
    statistics = {};
    statistics.LastUpdateTime = time(nullptr);
}

bool LSDB::AddOrUpdateLSA(const LinkStateAdvertisement& lsa) {
    uint16_t routerId = lsa.GetOriginatingRouterId();
    
    // Check if LSA already exists
    auto it = database.find(routerId);
    if (it != database.end()) {
        // Compare sequence numbers
        if (lsa.GetSequenceNumber() < it->second.GetSequenceNumber()) {
            statistics.StaleAdversmentRejections++;
            std::cerr << "Rejecting stale LSA from router " << routerId << "\n";
            return false;
        }
        
        if (lsa.GetSequenceNumber() == it->second.GetSequenceNumber()) {
            statistics.DuplicateAdversmentRejections++;
            std::cout << "Duplicate LSA from router " << routerId << " (same sequence number)\n";
            return false;
        }
    }
    
    database[routerId] = lsa;
    statistics.TotalLSAsReceived++;
    statistics.LastUpdateTime = time(nullptr);
    
    // Add router as neighbor if not already present
    AddNeighbor(routerId);
    
    return true;
}

bool LSDB::RemoveLSA(uint16_t routerId) {
    auto it = database.find(routerId);
    if (it != database.end()) {
        database.erase(it);
        return true;
    }
    return false;
}

const LinkStateAdvertisement* LSDB::GetLSA(uint16_t routerId) const {
    auto it = database.find(routerId);
    if (it != database.end()) {
        return &it->second;
    }
    return nullptr;
}

LinkStateAdvertisement* LSDB::GetLSA(uint16_t routerId) {
    auto it = database.find(routerId);
    if (it != database.end()) {
        return &it->second;
    }
    return nullptr;
}

const std::map<uint16_t, LinkStateAdvertisement>& LSDB::GetAllLSAs() const {
    return database;
}

const std::set<uint16_t>& LSDB::GetNeighbors() const {
    return NeighborRouterIds;
}

bool LSDB::IsKnownNeighbor(uint16_t routerId) const {
    return NeighborRouterIds.count(routerId) > 0;
}

std::vector<LinkStateEntry> LSDB::GetLinksToDestination(uint16_t destinationId) const {
    std::vector<LinkStateEntry> result;
    
    for (const auto& [routerId, lsa] : database) {
        const auto& links = lsa.GetLinks();
        for (const auto& link : links) {
            if (link.DestinationNodeId == destinationId && link.LinkActive) {
                result.push_back(link);
            }
        }
    }
    
    return result;
}

std::vector<uint16_t> LSDB::GetDirectNeighbors() const {
    std::vector<uint16_t> neighbors;
    
    // Get the local router's LSA
    const auto* localLSA = GetLSA(LocalRouterId);
    if (localLSA == nullptr) {
        return neighbors;
    }
    
    // Extract all destination nodes from our own LSA
    const auto& links = localLSA->GetLinks();
    for (const auto& link : links) {
        if (link.LinkActive) {
            neighbors.push_back(link.DestinationNodeId);
        }
    }
    
    return neighbors;
}

void LSDB::AddNeighbor(uint16_t routerId) {
    NeighborRouterIds.insert(routerId);
}

void LSDB::RemoveNeighbor(uint16_t routerId) {
    NeighborRouterIds.erase(routerId);
}

void LSDB::ClearDatabase() {
    database.clear();
    NeighborRouterIds.clear();
}

size_t LSDB::GetLSACount() const {
    return database.size();
}

void LSDB::UpdateAllLSAges() {
    for (auto& [routerId, lsa] : database) {
        lsa.UpdateAge();
    }
    statistics.LastUpdateTime = time(nullptr);
}

uint32_t LSDB::FlushStaleAdvertisements(uint16_t maxAge) {
    uint32_t removed = 0;
    
    auto it = database.begin();
    while (it != database.end()) {
        if (it->second.IsStale(maxAge)) {
            std::cout << "Removing stale LSA from router " << it->first << "\n";
            it = database.erase(it);
            removed++;
        } else {
            ++it;
        }
    }
    
    if (removed > 0) {
        statistics.StaleAdversmentRejections += removed;
    }
    
    return removed;
}

std::vector<LinkStateAdvertisement> LSDB::GetLSAsToFlood() const {
    std::vector<LinkStateAdvertisement> toFlood;
    
    for (const auto& [routerId, lsa] : database) {
        // In a real implementation, you would track which LSAs are newly received
        // For now, we return all LSAs (or implement a separate tracking mechanism)
        toFlood.push_back(lsa);
    }
    
    return toFlood;
}

const LSDB::LSDBStats& LSDB::GetStatistics() const {
    return statistics;
}

void LSDB::ResetStatistics() {
    statistics = {};
    statistics.LastUpdateTime = time(nullptr);
}

void LSDB::PrintDatabase() const {
    std::cout << "\n\n================ LSDB for Router " << LocalRouterId << " ================\n";
    std::cout << "Total LSAs in Database: " << database.size() << "\n";
    std::cout << "Known Neighbors: " << NeighborRouterIds.size() << "\n";
    
    if (!NeighborRouterIds.empty()) {
        std::cout << "\nNeighbor IDs: ";
        for (uint16_t id : NeighborRouterIds) {
            std::cout << id << " ";
        }
        std::cout << "\n";
    }
    
    std::cout << "\n";
    
    for (const auto& [routerId, lsa] : database) {
        lsa.PrintLSA();
    }
    
    std::cout << "========================================================\n";
}

void LSDB::PrintStatistics() const {
    std::cout << "\n================ LSDB Statistics ================\n";
    std::cout << "Local Router ID: " << LocalRouterId << "\n";
    std::cout << "Total LSAs Received: " << statistics.TotalLSAsReceived << "\n";
    std::cout << "Total LSAs Flooded: " << statistics.TotalLSAsFlooded << "\n";
    std::cout << "Duplicate Rejections: " << statistics.DuplicateAdversmentRejections << "\n";
    std::cout << "Stale Rejections: " << statistics.StaleAdversmentRejections << "\n";
    
    // Print last update time
    if (statistics.LastUpdateTime > 0) {
        char timeStr[100];
        struct tm* timeinfo = localtime(&statistics.LastUpdateTime);
        strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", timeinfo);
        std::cout << "Last Update: " << timeStr << "\n";
    }
    
    std::cout << "================================================\n";
}

uint16_t LSDB::GetLocalRouterId() const {
    return LocalRouterId;
}
