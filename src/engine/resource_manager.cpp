#include "resource_manager.hpp"
#include <memory>

void ResourceManager::loadResources() {
    singletonInstance = std::make_unique<ResourceManager>();
}

void ResourceManager::unloadResources() {
    singletonInstance = nullptr;
}

std::unique_ptr<ResourceManager> ResourceManager::singletonInstance{nullptr};
