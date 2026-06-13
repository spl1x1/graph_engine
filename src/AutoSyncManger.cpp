#include <AutoSyncManager.hpp>
#include <iostream>

AutoSyncManager AutoSyncManager::instance;

void AutoSyncManager::Update(float deltaTime) {
    if (!instance.enabled || !instance.targetRouter) return;

    std::cout << "AutoSyncManager: Updating, deltaTime = " << deltaTime << " seconds\n";
    instance.timeSinceLastSync += deltaTime;
    if (instance.timeSinceLastSync >= instance.syncInterval) {
        instance.targetRouter->GetLSDB().SyncWithNeighbors();
        instance.timeSinceLastSync = 0.0f;
    }
}

void AutoSyncManager::SetTarget(Router* router) {
    instance.targetRouter = router;
}
void AutoSyncManager::Enable() {
    instance.enabled = true;
}
void AutoSyncManager::Disable() {
    instance.enabled = false;
}
