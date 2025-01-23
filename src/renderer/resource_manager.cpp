
#include "resource_manager.h"
#include <iostream>

ResourceManager::~ResourceManager()
{
    unloadAll();
}

bool ResourceManager::loadModel(const std::string &name, const char *filename)
{
    if (!FileExists(filename))
    {
        std::cerr << "Error: Could not find model file: " << filename << std::endl;
        return false;
    }

    try
    {
        Model model = LoadModel(filename);
        models[name] = model;
        return true;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error loading model: " << e.what() << std::endl;
        return false;
    }
}

bool ResourceManager::loadTexture(const std::string &name, const char *filename)
{
    if (!FileExists(filename))
    {
        std::cerr << "Error: Could not find texture file: " << filename << std::endl;
        return false;
    }

    Texture2D texture = LoadTexture(filename);
    textures[name] = texture;
    return true;
}

Model *ResourceManager::getModel(const std::string &name)
{
    auto it = models.find(name);
    return it != models.end() ? &it->second : nullptr;
}

Texture2D *ResourceManager::getTexture(const std::string &name)
{
    auto it = textures.find(name);
    return it != textures.end() ? &it->second : nullptr;
}

void ResourceManager::unloadAll()
{
    for (auto &[name, model] : models)
    {
        UnloadModel(model);
    }
    for (auto &[name, texture] : textures)
    {
        UnloadTexture(texture);
    }
    models.clear();
    textures.clear();
}