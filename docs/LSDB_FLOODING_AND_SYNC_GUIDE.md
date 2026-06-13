/**
 * @file LSDB_FLOODING_AND_SYNC_GUIDE.md
 * @brief Guide for implementing LSA flooding and sync button widget
 * 
 * This document explains how to implement LSA flooding between routers
 * and integrate a sync button widget to trigger network synchronization.
 */

# LSDB Flooding and Network Synchronization Guide

## Overview

LSA flooding is the mechanism by which routers distribute their Link State Advertisements to neighbors, ensuring all routers have a synchronized view of the network topology. This guide shows how to implement flooding in your LSDB and create a sync button widget to trigger synchronization.

## 1. Adding Flooding Methods to LSDB

### Concepts

**Flooding Process:**
1. A router creates/updates an LSA describing its local links
2. It floods this LSA to all neighbors
3. Neighbors check if the LSA is new or updated (using sequence numbers)
4. If new/updated, neighbors add it to their database and flood to their neighbors
5. Process continues until all routers have the LSA

### Implementation Steps

#### Step 1: Add Flooding Tracking to LSDB

Add to `LSDB.hpp`:

```cpp
class LSDB {
private:
    std::set<uint16_t> lastFloodedLSAs;  // Track which LSAs were recently flooded
    time_t lastFloodTime{0};
    
public:
    // ... existing methods ...
    
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
     * @brief Get LSAs that haven't been recently flooded
     * @return Vector of LSAs needing to be flooded
     */
    std::vector<LinkStateAdvertisement> GetNewLSAsToFlood() const;
    
    /**
     * @brief Manually trigger a full synchronization with all neighbors
     * @return Number of LSAs flooded
     */
    uint32_t SyncWithNeighbors();
};
```

#### Step 2: Implement Flooding in LSDB.cpp

```cpp
LinkStateAdvertisement LSDB::CreateLocalLSA(const std::vector<LinkStateEntry>& localLinks) {
    LinkStateAdvertisement localLSA(LocalRouterId);
    
    for (const auto& link : localLinks) {
        localLSA.AddLink(link);
    }
    
    localLSA.IncrementSequenceNumber();
    localLSA.ResetAge();
    localLSA.CalculateChecksum();
    
    return localLSA;
}

std::vector<uint16_t> LSDB::FloodLSA(const LinkStateAdvertisement& lsa) {
    std::vector<uint16_t> floodedNeighbors;
    
    std::cout << "📡 Flooding LSA from Router " << lsa.GetOriginatingRouterId() 
              << " to " << NeighborRouterIds.size() << " neighbors\n";
    
    // In a real implementation, you would send LSA packets to each neighbor
    // For simulation, we mark LSA as flooded and increment statistics
    for (uint16_t neighborId : NeighborRouterIds) {
        std::cout << "   → Sending to neighbor: " << neighborId << "\n";
        floodedNeighbors.push_back(neighborId);
        statistics.TotalLSAsFlooded++;
    }
    
    lastFloodTime = time(nullptr);
    lastFloodedLSAs.insert(lsa.GetOriginatingRouterId());
    
    return floodedNeighbors;
}

std::vector<LinkStateAdvertisement> LSDB::GetNewLSAsToFlood() const {
    std::vector<LinkStateAdvertisement> newLSAs;
    
    for (const auto& [routerId, lsa] : database) {
        // Return LSAs that haven't been recently flooded
        if (lastFloodedLSAs.find(routerId) == lastFloodedLSAs.end()) {
            newLSAs.push_back(lsa);
        }
    }
    
    return newLSAs;
}

uint32_t LSDB::SyncWithNeighbors() {
    uint32_t lsasFlooded = 0;
    
    std::cout << "\n" << std::string(70, '=') << "\n";
    std::cout << "🔄 NETWORK SYNCHRONIZATION - Router " << LocalRouterId << "\n";
    std::cout << std::string(70, '=') << "\n";
    
    // Flood all LSAs in the database to neighbors
    for (const auto& [routerId, lsa] : database) {
        if (FloodLSA(lsa).size() > 0) {
            lsasFlooded++;
        }
    }
    
    std::cout << "✓ Synchronization complete. " << lsasFlooded 
              << " LSAs flooded to " << NeighborRouterIds.size() 
              << " neighbors\n";
    std::cout << std::string(70, '=') << "\n\n";
    
    return lsasFlooded;
}
```

## 2. Creating a Sync Button Widget

### Implementation

Create `Engine/include/SyncButton.hpp`:

```cpp
#ifndef SYNC_BUTTON_HPP
#define SYNC_BUTTON_HPP

#include "Widget.hpp"
#include "Button.hpp"
#include <functional>
#include <cstdint>

/**
 * @class SyncButton
 * @brief A specialized button for triggering LSDB synchronization on a router
 * 
 * When clicked, this button triggers LSA flooding for a specific router,
 * synchronizing its topology database with all neighbors.
 */
class SyncButton : public Button {
private:
    uint16_t TargetRouterId;
    std::function<void(uint16_t)> OnSyncCallback;

public:
    /**
     * @brief Create a new sync button
     * @param data Widget positioning and border data
     * @param buttonInfo Button appearance and text
     * @param routerId The router ID to synchronize
     * @param onSync Callback function when sync is triggered
     */
    SyncButton(WidgetData data, ButtonData buttonInfo, uint16_t routerId,
               std::function<void(uint16_t)> onSync)
        : Button(data, buttonInfo), TargetRouterId(routerId), OnSyncCallback(onSync) {
        // Override the default OnClick to call our sync logic
        ButtonInfo.OnClick = [this]() {
            if (OnSyncCallback) {
                OnSyncCallback(TargetRouterId);
            }
        };
    }

    uint16_t GetTargetRouterId() const { return TargetRouterId; }
    void SetTargetRouterId(uint16_t id) { TargetRouterId = id; }
};

#endif // SYNC_BUTTON_HPP
```

## 3. Integration with Engine and Router

### Method 1: Context Menu Approach

When a router is clicked, display a sync button:

```cpp
// In Engine.cpp or UI handler
void ShowRouterContextMenu(Router* router, Vec2 mousePos) {
    WidgetData buttonData{
        .PosX = mousePos.x,
        .PosY = mousePos.y,
        .Width = 120.0f,
        .Height = 40.0f,
        .Border = {2.0f, BLACK, true}
    };

    ButtonData btnInfo{
        .IdleColor = SKYBLUE,
        .HoverColor = YELLOW,
        .ActiveColor = GREEN,
        .Text = "Sync LSDB",
        .OnClick = [router]() {
            // Trigger sync on this router
            uint32_t flooded = router->GetLSDB().SyncWithNeighbors();
            std::cout << "Synced: " << flooded << " LSAs flooded\n";
        }
    };

    auto syncBtn = std::make_unique<Button>(buttonData, btnInfo);
    Widget::Register("router_sync_btn", std::move(syncBtn));
    Widget::Draw("router_sync_btn");
}
```

### Method 2: Persistent UI Approach

Add a sync button to the main UI that acts on the currently selected router:

```cpp
// In Engine initialization
void Engine::InitializeSyncButton() {
    WidgetData syncBtnData{
        .PosX = 10.0f,
        .PosY = 60.0f,
        .Width = 100.0f,
        .Height = 35.0f,
        .Border = {1.5f, DARKBLUE, true}
    };

    ButtonData syncBtnInfo{
        .IdleColor = LIGHTBLUE,
        .HoverColor = YELLOW,
        .ActiveColor = GREEN,
        .Text = "Sync",
        .OnClick = [this]() {
            // Get selected router and sync
            if (auto selectedRouter = GetSelectedRouter()) {
                uint32_t flooded = selectedRouter->GetLSDB().SyncWithNeighbors();
                std::cout << "✓ Synced " << flooded << " LSAs\n";
            }
        }
    };

    auto syncButton = std::make_unique<Button>(syncBtnData, syncBtnInfo);
    Widget::Register("main_sync_btn", std::move(syncButton));
}
```

## 4. Workflow: Manual Network Synchronization

### Step-by-Step Example

```cpp
// Scenario: 3 routers in a network, need to synchronize

// 1. Create network with routers
NodeNetwork network;
auto router1 = std::make_unique<Router>(Vec2{100, 100});
auto router2 = std::make_unique<Router>(Vec2{300, 100});
auto router3 = std::make_unique<Router>(Vec2{200, 250});

network.AddNode(std::move(router1));
network.AddNode(std::move(router2));
network.AddNode(std::move(router3));

// 2. Connect routers with edges
network.AddEdge({1, 2});
network.AddEdge({2, 3});
network.AddEdge({1, 3});

// 3. Get router references
Router* r1 = static_cast<Router*>(network.GetNode(1));
Router* r2 = static_cast<Router*>(network.GetNode(2));
Router* r3 = static_cast<Router*>(network.GetNode(3));

// 4. Create local LSAs describing their connections
LinkStateEntry link1to2{
    .LinkId = 100,
    .SourceNodeId = 1,
    .DestinationNodeId = 2,
    .LinkCost = 10.0,
    .LinkActive = true
};
LinkStateEntry link1to3{
    .LinkId = 101,
    .SourceNodeId = 1,
    .DestinationNodeId = 3,
    .LinkCost = 15.0,
    .LinkActive = true
};

LinkStateAdvertisement lsa1(1);
lsa1.AddLink(link1to2);
lsa1.AddLink(link1to3);

// 5. Add to each router's LSDB (in real system, would be via flooding)
r1->GetLSDB().AddOrUpdateLSA(lsa1);
r2->GetLSDB().AddOrUpdateLSA(lsa1);
r3->GetLSDB().AddOrUpdateLSA(lsa1);

// 6. User clicks sync button → triggers flooding
r1->GetLSDB().SyncWithNeighbors();
```

## 5. Event Flow Diagram

```
User clicks Sync Button
        ↓
Button::OnClick() callback invoked
        ↓
Router::GetLSDB().SyncWithNeighbors() called
        ↓
LSDB loops through all LSAs in database
        ↓
For each LSA:
    - FloodLSA(lsa) called
    - For each neighbor:
        * Send LSA (or simulate sending)
        * Increment flood statistics
        * Mark LSA as recently flooded
        ↓
Print synchronization summary
        ↓
UI Updates with new statistics
```

## 6. Console Output Example

When sync button is clicked:

```
======================================================================
🔄 NETWORK SYNCHRONIZATION - Router 1
======================================================================
📡 Flooding LSA from Router 1 to 2 neighbors
   → Sending to neighbor: 2
   → Sending to neighbor: 3
📡 Flooding LSA from Router 2 to 2 neighbors
   → Sending to neighbor: 1
   → Sending to neighbor: 3
📡 Flooding LSA from Router 3 to 2 neighbors
   → Sending to neighbor: 1
   → Sending to neighbor: 2
✓ Synchronization complete. 3 LSAs flooded to 2 neighbors
======================================================================
```

## 7. Advanced Features

### Auto-Sync Timer

Automatically synchronize at intervals:

```cpp
class AutoSyncTimer {
private:
    LSDB* targetLSDB;
    float syncInterval{5.0f};  // Every 5 seconds
    float timeSinceLastSync{0.0f};

public:
    void Update(float deltaTime) {
        timeSinceLastSync += deltaTime;
        if (timeSinceLastSync >= syncInterval) {
            if (targetLSDB) {
                targetLSDB->SyncWithNeighbors();
            }
            timeSinceLastSync = 0.0f;
        }
    }
};
```

### Selective Flooding

Flood only to specific neighbors:

```cpp
std::vector<uint16_t> LSDB::FloodLSAToNeighbor(
    const LinkStateAdvertisement& lsa, 
    uint16_t neighborId) {
    
    if (!IsKnownNeighbor(neighborId)) {
        std::cerr << "Error: " << neighborId << " is not a known neighbor\n";
        return {};
    }
    
    std::cout << "📡 Flooding LSA " << lsa.GetOriginatingRouterId() 
              << " to neighbor " << neighborId << "\n";
    
    statistics.TotalLSAsFlooded++;
    return {neighborId};
}
```

## 8. Testing the Implementation

### Test Case 1: Simple Flood

```cpp
void TestSimpleFlood() {
    LSDB db1(1), db2(2);
    db1.AddNeighbor(2);
    db2.AddNeighbor(1);
    
    LinkStateAdvertisement lsa(1);
    LinkStateEntry link{.LinkId = 1, .SourceNodeId = 1, .DestinationNodeId = 2, .LinkCost = 10.0};
    lsa.AddLink(link);
    
    db1.AddOrUpdateLSA(lsa);
    
    // Simulate flood
    auto flooded = db1.FloodLSA(lsa);
    assert(flooded.size() == 1);
    assert(flooded[0] == 2);
    
    std::cout << "✓ Simple flood test passed\n";
}
```

## 9. Integration Checklist

- [ ] Add flooding methods to LSDB class
- [ ] Implement `CreateLocalLSA()` method
- [ ] Implement `FloodLSA()` method
- [ ] Implement `SyncWithNeighbors()` method
- [ ] Create SyncButton widget class
- [ ] Add sync button to Engine UI
- [ ] Connect button click to LSDB sync
- [ ] Test flooding with multiple routers
- [ ] Add auto-sync timer (optional)
- [ ] Add statistics tracking to UI

## 10. Troubleshooting

### Issue: Sync button doesn't appear
- **Solution**: Check widget registration and visibility flags
- **Verify**: `Widget::Register()` called and `Widget::Draw()` invoked in render loop

### Issue: Flooding not reaching all neighbors
- **Solution**: Ensure neighbors are added with `AddNeighbor()` before syncing
- **Verify**: Check `GetNeighbors()` returns expected routers

### Issue: Stale LSAs keep flooding
- **Solution**: Implement flood history tracking (already included)
- **Verify**: `lastFloodedLSAs` is being updated

## Summary

The LSA flooding mechanism allows your network simulation to maintain synchronized topology information across all routers. The sync button provides an intuitive way to trigger this synchronization from the UI, making it easy to test and visualize network convergence.

With this implementation, you can now:
- ✅ Manually trigger network synchronization
- ✅ Observe LSA flooding in real-time via console output
- ✅ Track synchronization statistics
- ✅ Test routing protocol convergence
- ✅ Visualize topology propagation through the network
