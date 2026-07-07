#ifndef RESOURCE_MANAGER_HPP
#define RESOURCE_MANAGER_HPP

#include "entity_model.hpp"
#include "graphics.hpp"
#include "logger.hpp"
#include "text.hpp"
#include <memory>

class ResourceManager {
private:
    static std::unique_ptr<ResourceManager> singletonInstance;

public:
    static inline ResourceManager& instance() {
        Logger::assertion(singletonInstance.get());
        return *singletonInstance;
    }

    static void loadResources();
    static void unloadResources();

    struct {
        Texture terrain{"assets/resources/alphaTerrain.png"};
        Texture human{"assets/resources/mob/char.png"};
        Texture pigman{"assets/resources/mob/pigman.png"};
    } texture;

    struct {
        ShaderProgram terrainShader = ShaderProgram::loadFiles("assets/terrain.vs.glsl", "assets/terrain.fs.glsl");
        ShaderProgram simpleShader = ShaderProgram::loadFiles("assets/simple.vs.glsl", "assets/simple.fs.glsl");
        ShaderProgram entityShader = ShaderProgram::loadFiles("assets/entity.vs.glsl", "assets/entity.fs.glsl");
    } shader;

    struct {
        HumanModel human{};
    } entityModel;

    Font font{"assets/resources/font/default.png"};
};

#endif