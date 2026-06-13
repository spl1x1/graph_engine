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

## 1. Flooding Methods in LSDB

### Available Methods

The LSDB now includes these flooding-related methods:

```cpp
// Create a local LSA from current network links
LinkStateAdvertisement CreateLocalLSA(const std::vector<LinkStateEntry>& localLinks);

// Flood an LSA to all neighbors
std::vector<uint16_t> FloodLSA(const LinkStateAdvertisement& lsa);

// Get LSAs that need to be flooded
std::vector<LinkStateAdvertisement> GetLSAsToFlood() const;
std::vector<LinkStateAdvertisement> GetNewLSAsToFlood() const;

// Trigger full synchronization with neighbors
uint32_t SyncWithNeighbors();
```

## 2. Creating and Using a Sync Button

### Step 1: Add Sync Button to Engine UI

In your Engine initialization (e.g., `DrawUI()` or `DrawMenuGUI()` method):

```cpp
// Create sync button widget
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
    .Text = "Sync LSDB",
    .OnClick = [this]() {
        // Get currently selected router
        if (auto selectedRouter = GetSelectedRouter()) {
            Router* router = dynamic_cast<Router*>(selectedRouter);
            if (router) {
                // Trigger synchronization
                uint32_t flooded = router->GetLSDB().SyncWithNeighbors();
                std::cout << "✓ Synced " << flooded << " LSAs\n";
            }
        } else {
            std::cout << "⚠ No router selected. Click a router first.\n";
        }
    }
};

auto syncButton = std::make_unique<Button>(syncBtnData, syncBtnInfo);
Widget::Register("main_sync_btn", std::move(syncButton));
```

### Step 2: Draw the Sync Button

In your render loop (e.g., in `DrawUI()` or `DrawMenuGUI()`):

```cpp
Widget::Draw("main_sync_btn");
```

### Step 3: Track Selected Router

Add a member variable to your Engine class:

```cpp
class Engine {
private:
    INode* selectedRouter{nullptr};
    
    INode* GetSelectedRouter() const { return selectedRouter; }
    void SetSelectedRouter(INode* node) { selectedRouter = node; }
};
```

Update `ProcessNodeClick()` to set the selected router:

```cpp
void Engine::ProcessNodeClick() {
    // ... existing collision detection code ...
    
    if (BasicNodeOperations::IsColliding(MousePos, *node)) {
        nodeId = node->GetData().Id;
        SetSelectedRouter(node.get());  // Track selection
        break;
    }
}
```

## 3. Workflow: Step-by-Step Usage

### Scenario: Synchronizing a 3-Router Network

**Step 1: Create Network**
```
[Router 1] ←→ [Router 2]
    ↓             ↓
    └─→ [Router 3] ←┘
```

**Step 2: Click on Router 1**
- Router 1 is now selected
- Console shows: Router 1 basic info and LSDB state

**Step 3: Click the "Sync LSDB" Button**
- Button triggers `Router1->GetLSDB().SyncWithNeighbors()`
- Console output:
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

## 4. Integration with Router Click Events

The Router's `NodeClicked()` method now automatically prints:

1. **Router basic information**
   - Router ID, IP address, type
   - Connected edges, message queue size

2. **Topology Database State**
   - All LSAs in the LSDB
   - Each LSA's links and their costs
   - Direct neighbors
   - Known neighbors from received LSAs

3. **LSDB Statistics**
   - Total LSAs received
   - LSAs flooded
   - Duplicate/stale rejections
   - Last update time

### Example Output When Clicking a Router

```
╔════════════════════════════════════════════════════════════════════╗
║                    ROUTER CLICK EVENT                               ║
╚════════════════════════════════════════════════════════════════════╝
Router ID: 1
Router Address: 192.168.1.1
Router Type: Router
Connected Edges: 2
Message Queue Size: 0

======================================================================
TOPOLOGY DATABASE STATE FOR ROUTER 1 (192.168.1.1)
======================================================================

================ LSDB for Router 1 ================
Total LSAs in Database: 3
Known Neighbors: 3

Neighbor IDs: 1 2 3 

========== LSA ==========
Originating Router ID: 1
Sequence Number: 5
Age: 12 seconds
Checksum: 0xabcdef12
Number of Links: 2

Links:
  LinkID      Source        Dest      Cost    Active
--------------------------------------------------
     100           1           2      10.00        Yes
     102           1           3      15.50        Yes
=======================

[Additional LSAs from Router 2 and Router 3...]

Direct Neighbors from Local LSA: 2 3 

Known Neighbors (from LSAs received): 1 2 3 
======================================================================

======================================================================
LSDB STATISTICS FOR ROUTER 1 (192.168.1.1)
======================================================================

================ LSDB Statistics ================
Local Router ID: 1
Total LSAs Received: 47
Total LSAs Flooded: 45
Duplicate Rejections: 2
Stale Rejections: 0
Last Update: 2026-06-13 14:23:15
================================================

Router Information:
  Router ID: 1
  Router Address: 192.168.1.1
  Router Type: Router
  Number of Edges: 2
  Message Queue Size: 0
  Total LSAs in Database: 3
======================================================================
```

## 5. Complete Integration Example

### Full Example: Engine with Sync Button

```cpp
class Engine {
private:
    INode* selectedRouter{nullptr};
    
    void InitializeSyncButton() {
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
            .Text = "Sync LSDB",
            .OnClick = [this]() {
                if (selectedRouter) {
                    if (auto router = dynamic_cast<Router*>(selectedRouter)) {
                        uint32_t flooded = router->GetLSDB().SyncWithNeighbors();
                        std::cout << "✓ Synced " << flooded << " LSAs\n";
                    }
                } else {
                    std::cout << "⚠ Select a router first\n";
                }
            }
        };

        auto syncButton = std::make_unique<Button>(syncBtnData, syncBtnInfo);
        Widget::Register("sync_btn", std::move(syncButton));
    }

    void ProcessNodeClick() {
        Vec2 mousePos = GetMousePosition();
        
        for (auto& [id, node] : nodes.GetNodeMap()) {
            if (BasicNodeOperations::IsColliding(mousePos, *node)) {
                selectedRouter = node.get();
                node->NodeClicked();  // Prints LSDB state
                return;
            }
        }
    }

    void DrawUI() {
        Widget::Draw("sync_btn");
    }

public:
    Engine() {
        InitializeSyncButton();
    }
};
```

## 6. Adding Auto-Sync Timer (Optional)

For automatic periodic synchronization:

```cpp
class AutoSyncManager {
private:
    Router* targetRouter;
    float syncInterval{5.0f};  // Every 5 seconds
    float timeSinceLastSync{0.0f};
    bool enabled{false};

public:
    void Update(float deltaTime) {
        if (!enabled || !targetRouter) return;
        
        timeSinceLastSync += deltaTime;
        if (timeSinceLastSync >= syncInterval) {
            targetRouter->GetLSDB().SyncWithNeighbors();
            timeSinceLastSync = 0.0f;
        }
    }

    void SetTarget(Router* router) { targetRouter = router; }
    void Enable() { enabled = true; }
    void Disable() { enabled = false; }
};
```

## 7. Summary

You now have:

✅ **LSA Flooding System**
- Routers can flood their LSAs to neighbors
- Sequence numbers prevent duplicate/stale updates
- Statistics tracking for monitoring

✅ **Sync Button Widget**
- Easy UI integration using your widget system
- Triggers network synchronization with one click
- Provides visual feedback via console output

✅ **LSDB State Inspection**
- Click any router to see its topology database
- View all LSAs, links, and neighbors
- Monitor LSDB statistics and health

✅ **Network Synchronization**
- Manual sync with the sync button
- Optional auto-sync with timers
- Real-time console visualization of flooding

## Next Steps

1. Integrate sync button into your Engine UI
2. Test with multiple routers
3. Implement auto-sync timer (optional)
4. Add UI indicators for sync status (future enhancement)
5. Implement actual packet sending (advanced)
