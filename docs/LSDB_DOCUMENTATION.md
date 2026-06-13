# Link State Database (LSDB) Documentation

## Overview

The Link State Database (LSDB) is a core component of link-state routing protocols like OSPF (Open Shortest Path First). It maintains a synchronized database of the network topology across all routers. This implementation provides a comprehensive LSDB class that can be composed into each network node to enable distributed routing capabilities.

## Architecture

### Design Pattern: Composition

The LSDB is designed to be **composed into each node** rather than inherited, following the composition-over-inheritance principle. This allows:

- **Flexibility**: Different node types can have different LSDB implementations or none at all
- **Separation of Concerns**: Network topology management is separate from node behavior
- **Reusability**: The same LSDB implementation can be used across different node types (Routers, Switches, Hosts, etc.)

### Class Hierarchy

```
LinkStateAdvertisement (LSA)
    └─ Contains: LSAHeader + vector<LinkStateEntry>

LinkStateDatabase (LSDB)
    └─ Contains: map<RouterId, LinkStateAdvertisement>
              + Neighbor tracking
              + Statistics collection
```

## Core Components

### 1. LinkStateAdvertisement (LSA)

An LSA is a packet that describes the link state of a router. It contains information about all the links connected to that router.

#### Structure:

```cpp
class LinkStateAdvertisement {
private:
    LSAHeader header;                       // Metadata
    std::vector<LinkStateEntry> links;      // All adjacent links
    uint16_t OriginatingRouterId;           // Which router created this
};
```

#### LSAHeader Fields:

- **SequenceNumber**: Used to detect duplicate or outdated LSAs
  - Increments with each new LSA generation
  - Helps prevent routing loops caused by stale information
  
- **Age**: Time in seconds since the LSA was created
  - Increments periodically
  - LSAs older than MaxAge (typically 3600s) are considered stale
  
- **Checksum**: Data integrity verification
  - Detects corrupted LSAs
  - Calculated as XOR of sequence number, router ID, and link information
  
- **AdvertisingRouter**: The ID of the router that created this LSA
  - Used as the key to identify LSAs in the database

#### LinkStateEntry Fields:

Each entry describes a single link from the originating router:

```cpp
struct LinkStateEntry {
    uint16_t LinkId;                    // Unique identifier for this link
    uint16_t SourceNodeId;              // Source router ID
    uint16_t DestinationNodeId;         // Destination router ID
    IPAddress SourceAddress;            // IP address of source
    IPAddress DestinationAddress;       // IP address of destination
    double LinkCost;                    // Cost metric (inverse of bandwidth)
    bool LinkActive;                    // Current link state
    LinkSpeed Speed;                    // Link speed class (SLOW/MEDIUM/FAST)
};
```

### 2. LinkStateDatabase (LSDB)

The LSDB is the main class that maintains the complete topology database for a single router.

#### Key Data Structures:

```cpp
class LSDB {
private:
    uint16_t LocalRouterId;
    std::map<uint16_t, LinkStateAdvertisement> database;
    std::set<uint16_t> NeighborRouterIds;
    LSDBStats statistics;
};
```

#### LSDBStats:

Tracks various metrics for monitoring and debugging:

```cpp
struct LSDBStats {
    uint32_t TotalLSAsReceived;              // Total LSAs ever received
    uint32_t TotalLSAsFlooded;               // Total LSAs forwarded
    uint32_t DuplicateAdversmentRejections;  // Duplicate LSA rejections
    uint32_t StaleAdversmentRejections;      // Outdated LSA rejections
    time_t LastUpdateTime;                   // Timestamp of last update
};
```

## How It Works

### 1. Initialization

When a node is created with routing capabilities:

```cpp
LSDB lsdb(nodeId);  // Create and initialize LSDB for this node
```

The LSDB:
- Sets the local router ID
- Clears all previous data
- Initializes statistics

### 2. LSA Generation and Addition

When a router detects a change in its local links (link up/down):

1. **Create LSA**: Generate a new LinkStateAdvertisement with the router's ID
2. **Add Links**: Add all current adjacent links to the LSA
3. **Update Sequence**: Increment the sequence number
4. **Calculate Checksum**: Generate checksum for integrity
5. **Add to Database**: Call `AddOrUpdateLSA(lsa)`

```cpp
// Example: Router 1 creates an LSA describing its links
LinkStateAdvertisement lsa(1);

LinkStateEntry link1{
    .LinkId = 101,
    .SourceNodeId = 1,
    .DestinationNodeId = 2,
    .LinkCost = 10.0,
    .LinkActive = true
};
lsa.AddLink(link1);

lsdb.AddOrUpdateLSA(lsa);  // Add to local database
```

### 3. LSA Reception and Validation

When a router receives an LSA from a neighbor:

1. **Check Sequence Number**: Reject if it's older than what we have
2. **Check Checksum**: Verify data integrity
3. **Update Database**: Add/update in the LSDB
4. **Mark for Flooding**: Prepare to send to other neighbors

```cpp
// Receive LSA from neighbor
bool accepted = lsdb.AddOrUpdateLSA(receivedLSA);

if (accepted) {
    // Flood to all other neighbors
    std::vector<LinkStateAdvertisement> toFlood = lsdb.GetLSAsToFlood();
    for (const auto& lsa : toFlood) {
        SendToNeighbors(lsa);
    }
}
```

### 4. Age Management

Periodically (every second), the LSDB updates ages:

```cpp
void UpdateAges() {
    lsdb.UpdateAllLSAges();
}

// Periodically remove stale entries
uint32_t removed = lsdb.FlushStaleAdvertisements(3600);  // 1 hour max age
```

### 5. Topology Queries

The LSDB enables topology-aware routing by querying the database:

```cpp
// Find all links to a destination
std::vector<LinkStateEntry> routes = lsdb.GetLinksToDestination(destNodeId);

// Get direct neighbors
std::vector<uint16_t> neighbors = lsdb.GetDirectNeighbors();

// Check if a router is a neighbor
if (lsdb.IsKnownNeighbor(routerId)) {
    // Can communicate directly
}
```

## Integration with Nodes

### Composition Pattern

In your Router class or other node types:

```cpp
class Router : public INode {
private:
    LSDB topology;              // Composed LSDB member
    NodePosition position;
    NodeData data;

public:
    Router(Vec2 position) : position{position} {
        topology.Initialize(data.Id);
    }

    // Router can now manage its own topology database
    void SendMessage(Message message) override {
        // Use topology for intelligent routing
        auto routes = topology.GetLinksToDestination(message.DestinationId);
        // ... routing logic
    }
};
```

### Benefits of Composition

1. **Modularity**: Each node independently manages topology
2. **Scalability**: LSDB operations don't affect other nodes
3. **Flexibility**: Some nodes might not need LSDB (leaf nodes, etc.)
4. **Testing**: Easy to mock or test LSDB independently

## Key Algorithms

### LSA Acceptance Algorithm

When receiving an LSA, the LSDB implements the following logic:

```
1. If LSA with this OriginatingRouter not in database:
   → Accept and store
   
2. If incoming sequence number > stored sequence number:
   → Replace with new LSA (LSA is newer)
   
3. If incoming sequence number == stored sequence number:
   → Reject (duplicate)
   
4. If incoming sequence number < stored sequence number:
   → Reject (stale/outdated)
```

This ensures:
- No routing loops from stale information
- Efficient bandwidth usage (no duplicates)
- Convergence on newest topology information

### Checksum Validation

The checksum uses XOR of:
- Sequence number
- Advertising router ID
- All link IDs, source/destination, and costs

This provides a simple but effective corruption detection mechanism.

## Usage Examples

### Example 1: Create LSDB for a Router

```cpp
// Create router with LSDB
Router router(position);
router.GetData().Id = 1;

// Initialize topology database
LSDB& lsdb = router.GetLSDB();
lsdb.Initialize(1);
```

### Example 2: Simulate Network Discovery

```cpp
// Router 1 announces itself
LinkStateAdvertisement lsa1(1);
lsa1.AddLink(LinkStateEntry{
    .LinkId = 100,
    .SourceNodeId = 1,
    .DestinationNodeId = 2,
    .LinkCost = 10.0,
    .LinkActive = true
});
lsdb.AddOrUpdateLSA(lsa1);

// Router 2 announces itself
LinkStateAdvertisement lsa2(2);
lsa2.AddLink(LinkStateEntry{
    .LinkId = 101,
    .SourceNodeId = 2,
    .DestinationNodeId = 1,
    .LinkCost = 10.0,
    .LinkActive = true
});
lsdb.AddOrUpdateLSA(lsa2);
```

### Example 3: Track Statistics

```cpp
// Monitor LSDB health
const auto& stats = lsdb.GetStatistics();
std::cout << "LSAs Received: " << stats.TotalLSAsReceived << "\n";
std::cout << "Rejections: " << stats.DuplicateAdversmentRejections + 
                             stats.StaleAdversmentRejections << "\n";

lsdb.PrintStatistics();  // Print formatted statistics
lsdb.PrintDatabase();    // Print entire database contents
```

## Performance Considerations

### Time Complexity

| Operation | Complexity | Notes |
|-----------|-----------|-------|
| AddOrUpdateLSA | O(log n) | n = number of LSAs (routers) |
| GetLSA | O(log n) | Binary search in map |
| AddLink (to LSA) | O(1) | Vector push_back |
| FindLink (in LSA) | O(m) | m = number of links in LSA |
| GetDirectNeighbors | O(m) | m = max links in any LSA |
| UpdateAllLSAges | O(n) | n = number of LSAs |

### Space Complexity

- **Per Router**: O(m) where m = average number of links per router
- **Database**: O(n × m) where n = number of routers in network
- For a network with 100 routers, ~5 links each: ~500 entries

## Thread Safety

The current implementation is **NOT thread-safe**. For multi-threaded environments:

1. Add mutex protection around database access
2. Use atomic operations for statistics
3. Consider read-write locks for mixed workloads

```cpp
class LSDB {
private:
    mutable std::shared_mutex dbMutex;
    
    bool AddOrUpdateLSA(const LinkStateAdvertisement& lsa) {
        std::unique_lock lock(dbMutex);
        // ... existing logic
    }
};
```

## Debugging and Diagnostics

### Print Database State

```cpp
lsdb.PrintDatabase();  // Shows all LSAs with all links
```

Output:
```
================ LSDB for Router 1 ================
Total LSAs in Database: 2
Known Neighbors: 2

Neighbor IDs: 1 2 

========== LSA ==========
Originating Router ID: 1
Sequence Number: 5
Age: 45 seconds
Checksum: 0xabcdef12
Number of Links: 2

Links:
   LinkID      Source        Dest      Cost    Active
--------------------------------------------------
     100           1           2      10.00        Yes
     102           1           3      15.50        Yes
=======================
```

### Print Statistics

```cpp
lsdb.PrintStatistics();  // Shows stats since initialization or last reset
```

Output:
```
================ LSDB Statistics ================
Local Router ID: 1
Total LSAs Received: 47
Total LSAs Flooded: 45
Duplicate Rejections: 2
Stale Rejections: 0
Last Update: 2026-06-13 14:23:15
================================================
```

## Future Enhancements

### Possible Improvements

1. **Incremental Updates**: Instead of full LSAs, send only changed links
2. **Persistent Storage**: Save LSDB to disk for faster recovery
3. **Encryption**: Add signature validation for LSA origin verification
4. **Differential Flooding**: Track which neighbors have which LSAs
5. **Priority Queue**: Manage LSA flooding with priority handling
6. **TTL/Hop Limit**: Add hop-count limiting for large networks

## Related Protocols and Standards

This implementation is inspired by:

- **OSPF (RFC 3309)**: Open Shortest Path First routing protocol
- **IS-IS (RFC 7490)**: Intermediate System to Intermediate System protocol
- **Link-State Routing**: General link-state routing algorithm family

## Conclusion

The LSDB provides a robust foundation for implementing link-state routing protocols in your graph engine. By composing it into nodes, you enable:

- Distributed topology awareness
- Intelligent routing decisions
- Network convergence mechanisms
- Debugging and monitoring capabilities

The modular design allows for easy integration with existing node types and simple extension for future routing algorithms.
