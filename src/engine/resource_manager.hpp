#ifndef RESOURCE_MANAGER_HPP
#define RESOURCE_MANAGER_HPP

#include "entities/entity_model.hpp"
#include "graphics/graphics.hpp"
#include "logger.hpp"
#include "graphics/text.hpp"
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
        Texture terrain{"assets/resources/terrain.png"};
        Texture human{"assets/resources/mob/char.png"};
        Texture pigman{"assets/resources/mob/pigman.png"};
        Texture pig{"assets/resources/mob/pig.png"};
    } texture;

    struct {
        struct {
            Shader terrain = Shader::fromFile(GL_VERTEX_SHADER, "assets/terrain.vs.glsl");
            Shader entity = Shader::fromFile(GL_VERTEX_SHADER, "assets/entity.vs.glsl");
            Shader simple = Shader::fromFile(GL_VERTEX_SHADER, "assets/simple.vs.glsl");
        } vertex;
        struct {
            Shader terrain = Shader::fromFile(GL_FRAGMENT_SHADER, "assets/terrain.fs.glsl");
            Shader entity = Shader::fromFile(GL_FRAGMENT_SHADER, "assets/entity.fs.glsl");
            Shader simple = Shader::fromFile(GL_FRAGMENT_SHADER, "assets/simple.fs.glsl");
        } fragment;

        ShaderProgram terrain = ShaderProgram{vertex.terrain, fragment.terrain};
        ShaderProgram simple = ShaderProgram{vertex.simple, fragment.simple};
        ShaderProgram entity = ShaderProgram{vertex.entity, fragment.entity};
    } shader;

    struct {
        HumanModel human{};
        PigModel pig{};
    } entityModel;

    Font font{"assets/resources/font/default.png"};
};

#endif