#ifndef LSDB_HPP
#define LSDB_HPP

#include <map>
#include <vector>
#include <set>
#include <cstdint>
#include <memory>
#include <ctime>
#include <algorithm>
#include "Node.hpp"

/**
 * @class LSAHeader
 * @brief Represents the header information for a Link State Advertisement
 * 
 * Contains metadata about a link state advertisement including:
 * - Sequence number for detecting stale advertisements
 * - Age tracking for timeout management
 * - Checksum for data integrity verification
 */
struct LSAHeader {
    uint32_t SequenceNumber{0};
    uint16_t Age{0};              // Age in seconds
    uint32_t Checksum{0};
    uint32_t AdvertisingRouter{0};  // ID of router advertising this LSA
    time_t CreationTime{0};
};

/**
 * @class LinkStateEntry
 * @brief Represents a single link in a Link State Advertisement
 * 
 * Describes a connection between two nodes including:
 * - Source and destination node IDs
 * - Cost/metric of the link (inverse of bandwidth)
 * - Current state of the link (active/inactive)
 */
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

/**
 * @class LinkStateAdvertisement (LSA)
 * @brief Represents a complete Link State Advertisement
 * 
 * Contains:
 * - Header information with sequence numbers and timing
 * - A collection of all links known by the advertising router
 * - Methods to manage and query the link state data
 */
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
    /**
     * @brief Add or update an LSA in the database
     * @param lsa The Link State Advertisement to add
     * @return true if LSA was added/updated, false if it was rejected (duplicate/stale)
     */
    bool AddOrUpdateLSA(const LinkStateAdvertisement& lsa);
    
    /**
     * @brief Remove an LSA from the database
     * @param routerId The ID of the router that originated the LSA
     * @return true if LSA was removed, false if not found
     */
    bool RemoveLSA(uint16_t routerId);
    
    /**
     * @brief Get an LSA from the database
     * @param routerId The ID of the router that originated the LSA
     * @return Pointer to the LSA, or nullptr if not found
     */
    const LinkStateAdvertisement* GetLSA(uint16_t routerId) const;
    LinkStateAdvertisement* GetLSA(uint16_t routerId);
    
    // Query operations
    /**
     * @brief Get all LSAs in the database
     * @return Const reference to the LSA map
     */
    const std::map<uint16_t, LinkStateAdvertisement>& GetAllLSAs() const;
    
    /**
     * @brief Get all neighbor router IDs known to this database
     * @return Set of neighbor router IDs
     */
    const std::set<uint16_t>& GetNeighbors() const;
    
    /**
     * @brief Check if a neighbor is known
     * @param routerId The ID of the router to check
     * @return true if router is a neighbor, false otherwise
     */
    bool IsKnownNeighbor(uint16_t routerId) const;
    
    /**
     * @brief Get all links to a specific destination from the topology
     * @param destinationId The target node ID
     * @return Vector of link state entries leading to destination
     */
    std::vector<LinkStateEntry> GetLinksToDestination(uint16_t destinationId) const;
    
    /**
     * @brief Get all directly connected neighbors
     * @return Vector of neighbor router IDs
     */
    std::vector<uint16_t> GetDirectNeighbors() const;
    
    // Neighbor management
    void AddNeighbor(uint16_t routerId);
    void RemoveNeighbor(uint16_t routerId);
    
    // Database operations
    /**
     * @brief Flush all LSAs from the database
     */
    void ClearDatabase();
    
    /**
     * @brief Get the number of LSAs in the database
     * @return Count of LSAs
     */
    size_t GetLSACount() const;
    
    // Age management
    /**
     * @brief Update ages of all LSAs in the database
     * Typically called periodically (e.g., every second)
     */
    void UpdateAllLSAges();
    
    /**
     * @brief Remove stale LSAs from the database
     * @param maxAge Maximum age in seconds before an LSA is considered stale
     * @return Number of LSAs removed
     */
    uint32_t FlushStaleAdvertisements(uint16_t maxAge = 3600);
    
    // Flooding
    /**
     * @brief Prepare local LSA from current network links for flooding
     * @param localLinks Vector of links connected to this router
     * @return LinkStateAdvertisement ready to flood
     */
    LinkStateAdvertisement CreateLocalLSA(const std::vector<LinkStateEntry>& localLinks);
    
    /**
     * @brief Flood an LSA to all neighbors
     * @param lsa The advertisement to flood
     * @return Vector of neighbor IDs that received the LSA
     */
    std::vector<uint16_t> FloodLSA(const LinkStateAdvertisement& lsa);
    
    /**
     * @brief Get LSAs that need to be flooded to neighbors
     * Typically returns newly received or updated LSAs
     * @return Vector of LSAs to be flooded
     */
    std::vector<LinkStateAdvertisement> GetLSAsToFlood() const;
    
    /**
     * @brief Get LSAs that haven't been recently flooded
     * @return Vector of LSAs needing to be flooded
     */
    std::vector<LinkStateAdvertisement> GetNewLSAsToFlood() const;
    
    /**
     * @brief Manually trigger a full synchronization with all neighbors
     * @return Number of LSAs flooded
     */
    uint32_t SyncWithNeighbors();
    
    // Statistics and diagnostics
    /**
     * @brief Get database statistics
     * @return LSDBStats structure with various metrics
     */
    const LSDBStats& GetStatistics() const;
    
    /**
     * @brief Reset all statistics
     */
    void ResetStatistics();
    
    /**
     * @brief Print the entire database (for debugging)
     */
    void PrintDatabase() const;
    
    /**
     * @brief Print database statistics
     */
    void PrintStatistics() const;
    
    /**
     * @brief Get local router ID
     * @return The ID of the router owning this LSDB
     */
    uint16_t GetLocalRouterId() const;
};

#endif // LSDB_HPP
