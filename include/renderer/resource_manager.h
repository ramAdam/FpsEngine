#pragma once
#include "raylib.h"
#include "physics_manager.h"
#include <string>
#include <unordered_map>
#include <memory>

class ResourceManager
{
private:
    std::unordered_map<std::string, Model> models;
    std::unordered_map<std::string, Texture2D> textures;
    PhysicsManager &physicsManager;

public:
    ResourceManager(PhysicsManager &physics);
    ~ResourceManager();

    bool loadModel(const std::string &name, const char *filename);
    bool loadTexture(const std::string &name, const char *filename);
    void removeModel(const std::string &name);

    Model *getModel(const std::string &name);
    Texture2D *getTexture(const std::string &name);

    void unloadAll();
};