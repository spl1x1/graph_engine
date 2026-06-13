#ifndef AUTOSYNCMANAGER_HPP
#define AUTOSYNCMANAGER_HPP

#include <Router.hpp>

class AutoSyncManager {
private:
    static AutoSyncManager instance;

    Router* targetRouter;
    float syncInterval{5.0f};  // Every 5 seconds
    float timeSinceLastSync{0.0f};
    bool enabled{false};

public:



    static void Update(float deltaTime);
    static void SetTarget(Router* router);
    static void Enable();
    static void Disable();
};

#endif // AUTOSYNCMANAGER_HPP
