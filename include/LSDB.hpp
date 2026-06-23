#ifndef LSDB_HPP
#define LSDB_HPP

#include <map>
#include <vector>
#include <set>
#include <cstdint>
#include <ctime>
#include "Node.hpp"


struct LSAHeader {
    uint32_t SequenceNumber{0};
    uint16_t Age{0};              // Age in seconds
    uint32_t Checksum{0};
    uint32_t AdvertisingRouter{0};  // ID of router advertising this LSA
    time_t CreationTime{0};
};

struct LinkStateEntry {
    uint16_t LinkId{0};
    uint16_t SourceNodeId{0};
    uint16_t DestinationNodeId{0};
    IPAddress SourceAddress;
    IPAddress DestinationAddress;
    double LinkCost{1.0};           // Weight/cost of the link
    bool LinkActive{true};
    LinkSpeed Speed{LinkSpeed::MEDIUM};
};

class LinkStateAdvertisement {
private:
    LSAHeader header;
    std::vector<LinkStateEntry> links;
    uint16_t OriginatingRouterId{0};

public:
    LinkStateAdvertisement() = default;
    explicit LinkStateAdvertisement(uint16_t routerId);
    ~LinkStateAdvertisement() = default;

    // Getters
    const LSAHeader& GetHeader() const;
    LSAHeader& GetHeader();
    const std::vector<LinkStateEntry>& GetLinks() const;
    std::vector<LinkStateEntry>& GetLinks();
    uint16_t GetOriginatingRouterId() const;

    // Link management
    void AddLink(const LinkStateEntry& entry);
    void RemoveLink(uint16_t linkId);
    bool UpdateLink(uint16_t linkId, const LinkStateEntry& updatedEntry);
    const LinkStateEntry* GetLink(uint16_t linkId) const;

    // Sequence number management
    void IncrementSequenceNumber();
    uint32_t GetSequenceNumber() const;

    // Age management
    void UpdateAge();
    void ResetAge();
    uint16_t GetAge() const;

    // Checksum operations
    void CalculateChecksum();
    uint32_t GetChecksum() const;
    bool VerifyChecksum() const;

    // Utility
    bool IsStale(uint16_t maxAge = 3600) const;
    size_t GetLinkCount() const;
    void PrintLSA() const;
};

/**
 * @class LinkStateDatabase (LSDB)
 * @brief Complete database of link state advertisements for a node
 *
 * The LSDB maintains:
 * - A collection of LSAs from all known routers
 * - Topology information for route calculation
 * - Methods to add, update, and query LSAs
 * - Support for flooding new LSAs to neighbors
 *
 * This class is designed to be composed into each node via composition,
 * allowing each node to maintain its own view of the network topology.
 */
class LSDB {
private:
    uint16_t LocalRouterId{0};
    std::map<uint16_t, LinkStateAdvertisement> database;  // Key: Originating Router ID
    std::set<uint16_t> NeighborRouterIds;
    std::set<uint16_t> lastFloodedLSAs;  // Track which LSAs were recently flooded
    time_t lastFloodTime{0};

    struct LSDBStats {
        uint32_t TotalLSAsReceived{0};
        uint32_t TotalLSAsFlooded{0};
        uint32_t DuplicateAdversmentRejections{0};
        uint32_t StaleAdversmentRejections{0};
        time_t LastUpdateTime{0};
    } statistics;

public:
    LSDB() = default;
    explicit LSDB(uint16_t routerId);
    ~LSDB() = default;

    // Initialization
    void Initialize(uint16_t routerId);

    // LSA Management
    bool AddOrUpdateLSA(const LinkStateAdvertisement& lsa);
    bool RemoveLSA(uint16_t routerId);

    const LinkStateAdvertisement* GetLSA(uint16_t routerId) const;
    LinkStateAdvertisement* GetLSA(uint16_t routerId);

    const std::map<uint16_t, LinkStateAdvertisement>& GetAllLSAs() const;
    const std::set<uint16_t>& GetNeighbors() const;

    bool IsKnownNeighbor(uint16_t routerId) const;

    std::vector<LinkStateEntry> GetLinksToDestination(uint16_t destinationId) const;
    std::vector<uint16_t> GetShortestPath(uint16_t sourceRouterId, uint16_t destinationRouterId) const;

    std::vector<uint16_t> GetDirectNeighbors() const;

    // Neighbor management
    void AddNeighbor(uint16_t routerId);
    void RemoveNeighbor(uint16_t routerId);

    // Database operations
    void ClearDatabase();
    size_t GetLSACount() const;

    // Age management
    void UpdateAllLSAges();

    uint32_t FlushStaleAdvertisements(uint16_t maxAge = 3600);

    // Flooding
    LinkStateAdvertisement CreateLocalLSA(const std::vector<LinkStateEntry>& localLinks);
    std::vector<uint16_t> FloodLSA(const LinkStateAdvertisement& lsa);
    std::vector<LinkStateAdvertisement> GetLSAsToFlood() const;
    std::vector<LinkStateAdvertisement> GetNewLSAsToFlood() const;
    uint32_t SyncWithNeighbors();

    // Statistics and diagnostics
    const LSDBStats& GetStatistics() const;
    void ResetStatistics();
    void PrintDatabase() const;
    void PrintStatistics() const;
    uint16_t GetLocalRouterId() const;
};

#endif // LSDB_HPP
