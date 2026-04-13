#ifndef ASSET_MANAGER_HPP
#define ASSET_MANAGER_HPP

#include <string>
#include "raylib.h"

class AssetManager{
    private:
        void LoadAssets();
        void UnloadAssets();
public:
    AssetManager();
    ~AssetManager();

    Texture2D GetTexture(const std::string& name);
};

#endif // ASSET_MANAGER_HPP
