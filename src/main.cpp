#include <map>
#include <format>
#include <print>
#include <string>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/fwd.hpp>

#include "block.hpp"
#include "chunk.hpp"
#include "world.hpp"
#include "util.hpp"
#include "collision.hpp"
#include "entity.hpp"
#include "serialize.hpp"
#include "logger.hpp"
#include "graphics.hpp"
#include "input.hpp"
#include "text.hpp"


void drawThickWireCube(ShaderProgram& shader, glm::vec3 position, float lineWidth) {
    float axisOffset = 0.f;
    float epsilon = 0.010;

    float longLineLength = 1.f + 1.f * epsilon;

    float boxWidth = (1.f - lineWidth + epsilon) / 2.f;
    float offsets[] = {boxWidth, -boxWidth};

    float shortLineLength = longLineLength - 2.f * lineWidth;

    for (int i = 0; i < 2; ++i) {
        for (int j = 0; j < 2; ++j) {
            float offsetA = offsets[i];
            float offsetB = offsets[j];

            glm::vec3 xPos = position + glm::vec3{axisOffset, offsetA, offsetB};
            drawCube(shader, xPos, {shortLineLength, lineWidth, lineWidth});

            glm::vec3 yPos = position + glm::vec3{offsetA, axisOffset, offsetB};
            drawCube(shader, yPos, {lineWidth, longLineLength, lineWidth});

            glm::vec3 zPos = position + glm::vec3{offsetA, offsetB, axisOffset};
            drawCube(shader, zPos, {lineWidth, lineWidth, shortLineLength});
        }
    }
}


void errorCallbackGlfw(int error, const char* description) {
    Logger::error(std::format("GLFW Error {}: {}", error, description));
}

void checkOpenglErrors() {
    std::map<GLenum, std::string> openglErrors{
        {GL_INVALID_ENUM,                   "GL_INVALID_ENUM"},
        {GL_INVALID_VALUE,                  "GL_INVALID_VALUE"},
        {GL_INVALID_OPERATION,              "GL_INVALID_OPERATION"},
        {GL_STACK_OVERFLOW,                 "GL_STACK_OVERFLOW"},
        {GL_STACK_UNDERFLOW,                "GL_STACK_UNDERFLOW"},
        {GL_OUT_OF_MEMORY,                  "GL_OUT_OF_MEMORY"},
        {GL_INVALID_FRAMEBUFFER_OPERATION,  "GL_INVALID_FRAMEBUFFER_OPERATION"},
    };

    GLenum errorCode = 0;
    while ((errorCode = glGetError()) != GL_NO_ERROR) {
        Logger::error(std::format("OpenGL Error: {}", openglErrors[errorCode]));
    }
}

// From learnopengl
void openglDebugCallback(GLenum source, GLenum type, unsigned int id,
        GLenum severity, GLsizei length, const char *message, const void *userParam
) {
    (void)length;
    (void)userParam;

    static const std::map<GLenum, std::string_view> sources {
        {GL_DEBUG_SOURCE_API,             "API"},
        {GL_DEBUG_SOURCE_WINDOW_SYSTEM,   "Window System"},
        {GL_DEBUG_SOURCE_SHADER_COMPILER, "Shader Compiler"},
        {GL_DEBUG_SOURCE_THIRD_PARTY,     "Third Party"},
        {GL_DEBUG_SOURCE_APPLICATION,     "Application"},
        {GL_DEBUG_SOURCE_OTHER,           "Other"},
    };

    static const std::map<GLenum, std::string_view> types {
        {GL_DEBUG_TYPE_ERROR,               "Error"},
        {GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR, "Deprecated Behaviour"},
        {GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR,  "Undefined Behaviour"},
        {GL_DEBUG_TYPE_PORTABILITY,         "Portability"},
        {GL_DEBUG_TYPE_PERFORMANCE,         "Performance"},
        {GL_DEBUG_TYPE_MARKER,              "Marker"},
        {GL_DEBUG_TYPE_PUSH_GROUP,          "Push Group"},
        {GL_DEBUG_TYPE_POP_GROUP,           "Pop Group"},
        {GL_DEBUG_TYPE_OTHER,               "Other"},
    };

    static const std::map<GLenum, LogLevel> severities {
        {GL_DEBUG_SEVERITY_HIGH,         LogLevel::fatal},
        {GL_DEBUG_SEVERITY_MEDIUM,       LogLevel::error},
        {GL_DEBUG_SEVERITY_LOW,          LogLevel::warning},
        {GL_DEBUG_SEVERITY_NOTIFICATION, LogLevel::info},
    };

    // ignore non-significant error/warning codes
    if (id == 131169 || id == 131185 || id == 131218 || id == 131204) {
        return;
    }

    std::string text = std::format(
        "OpenGL({}), source: {}, type: {}: {}",
        id, sources.at(source), types.at(type), message
    );

    Logger::log(severities.at(severity), text);
}

int windowWidth = 800;
int windowHeight = 600;

glm::mat4 cameraProjection{};
glm::mat4 guiCameraProjection{};

void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    (void)window;

    windowWidth = width;
    windowHeight = height;

    cameraProjection = glm::perspective(
        glm::radians(90.f),
        static_cast<float>(windowWidth) / static_cast<float>(windowHeight),
        0.1f,
        10000.f
    );

    guiCameraProjection = glm::ortho(
        -windowWidth / 2.f, windowWidth / 2.f,
        -windowHeight / 2.f, windowHeight / 2.f
    );

    glViewport(0, 0, windowWidth, windowHeight);
}

void toggleFullscreen(GLFWwindow* window) {
    static bool isFullscreen = false;

    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);

    if (isFullscreen) {
        glfwSetWindowMonitor(window,
                nullptr,
                10, 10,
                800, 600,
                mode->refreshRate
            );
        framebufferSizeCallback(window, 800, 600);
    } else {
        glfwSetWindowMonitor(window,
                monitor,
                0, 0,
                mode->width, mode->height,
                mode->refreshRate
            );
        framebufferSizeCallback(window, mode->width, mode->height);
    }

    isFullscreen = !isFullscreen;
}

void runGame(GLFWwindow* window) {
    initMeshes();

    framebufferSizeCallback(window, windowWidth, windowHeight);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);

    blendModeNormal();

    int targetFps = 60;

    // Hide the cursor and lock it to the middle of the screen
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    makeSaveDirectories();

    ShaderProgram terrainShader = ShaderProgram::loadFiles("assets/terrain.vs.glsl", "assets/terrain.fs.glsl");
    ShaderProgram simpleShader = ShaderProgram::loadFiles("assets/simple.vs.glsl", "assets/simple.fs.glsl");

    Texture terrainTexture{"assets/resources/alphaTerrain.png"};
    Font font{"assets/resources/font/default.png"};

    glm::vec3 up{0.f, 1.f, 0.f};

    // TODO: Prerender all the block sprites somehow

    int selectedBlock = BLOCK_PLANKS;

    registerBlocks();

#ifdef DEFAULT_SET_SEED
    srand(DEFAULT_SET_SEED);
#else
    randomizeSeed();
#endif

    World world{};

    glClearColor(world.skyColor.r, world.skyColor.g, world.skyColor.b, world.skyColor.a);

    Player* player = world.player;

    bool showGui = true;

    glfwSwapInterval(0);

    double clock = glfwGetTime();
    while (!glfwWindowShouldClose(window)) {
        double time = glfwGetTime();
        float deltaTime = time - clock;
        clock = time;
        deltaTime *= 20.f;

        if (keyPressed(GLFW_KEY_Q) && targetFps > 0) {
            targetFps -= 10;
            //SetTargetFPS(targetFps);
        }

        if (keyPressed(GLFW_KEY_E)) {
            targetFps += 10;
            //SetTargetFPS(targetFps);
        }

        if (keyPressed(GLFW_KEY_F2)) {
            //saveScreenshot();
        }

        if (keyPressed(GLFW_KEY_F1)) {
            showGui = !showGui;
        }

        if (keyPressed(GLFW_KEY_F6)) {
            world.showChunkBorders = !world.showChunkBorders;
        }

        if (keyPressed(GLFW_KEY_F11)) {
            toggleFullscreen(window);
        }

        if (keyPressed(GLFW_KEY_F8) && world.renderDistance > 1) {
            world.renderDistance--;
        }

        if (keyPressed(GLFW_KEY_F9)) {
            world.renderDistance++;
        }

        if (keyPressed(GLFW_KEY_ESCAPE)) {
            glfwSetWindowShouldClose(window, true);
        }

        glm::vec3 camPosition = player->position + glm::vec3{0, PLAYER_EYE, 0};

        glm::vec3 lookVec{1.f, 0.f, 0.f};
        lookVec = glm::vec3{glm::rotate(glm::mat4{1.f}, player->pitch, {0.f, 0.f, 1.f}) * glm::vec4{lookVec, 1.f}};
        lookVec = glm::vec3{glm::rotate(glm::mat4{1.f}, player->yaw, {0.f, 1.f, 0.f}) * glm::vec4{lookVec, 1.f}};

        glm::vec3 target = camPosition + lookVec;

        WalkCollision rayCast = ddaCastRay(&world, camPosition, lookVec, 8.f);
        glm::vec3 cubePos = glm::vec3{rayCast.blockAt} + 0.5f;

        glm::mat4 view = glm::lookAt(camPosition, target, up);

        float initialBreakTime = 0.3f * 20.f;
        float repeatedBreakTime = 0.15f * 20.f;

        if (rayCast.collided) {
            if (mouseButtonPressed(GLFW_MOUSE_BUTTON_RIGHT)) {
                world.tryPlaceBlock(rayCast.blockBefore, static_cast<Block>(selectedBlock));
                timerResetTime(&player->breakTimer, initialBreakTime);
            }

            if (mouseButtonDown(GLFW_MOUSE_BUTTON_RIGHT) && timerUpdate(&player->breakTimer, deltaTime)) {
                world.tryPlaceBlock(rayCast.blockBefore, static_cast<Block>(selectedBlock));
                timerResetTime(&player->breakTimer, repeatedBreakTime);
            }


            if (mouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT)) {
                world.setBlock(rayCast.blockAt, BLOCK_AIR);
                timerResetTime(&player->breakTimer, initialBreakTime);
            }

            if (mouseButtonDown(GLFW_MOUSE_BUTTON_LEFT) && timerUpdate(&player->breakTimer, deltaTime)) {
                world.setBlock(rayCast.blockAt, BLOCK_AIR);
                timerResetTime(&player->breakTimer, repeatedBreakTime);
            }


            if (mouseButtonPressed(GLFW_MOUSE_BUTTON_MIDDLE)) {
                selectedBlock = world.getBlock(rayCast.blockAt);
            }
        }

        for (int key = GLFW_KEY_1; key <= GLFW_KEY_9; ++key) {
            if (keyPressed(key)) {
                Block block = static_cast<Block>(key - GLFW_KEY_1 + 1);
                selectedBlock = block;
            }
        }

        if (keyPressed(GLFW_KEY_H)) {
            Entity& mob = world.spawnEntity<Entity>(player->position, glm::vec3{0.6f, 1.8f, 0.6f});// spawnEntity(&world, ENTITY_MOB, player->position, 0.6f, 1.8f);
            mob.velocity = player->velocity * 2.f;
        }

        if (getMouseScroll() < 0) {
            selectedBlock -= 1;
        }

        if (getMouseScroll() > 0) {
            selectedBlock += 1;
        }

        selectedBlock = wrapInt(selectedBlock, 2, BLOCK_COUNT);

        world.update(deltaTime);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        terrainShader.setUniformMat4("projectionView", cameraProjection * view);
        terrainShader.setUniformVec3("camPos", camPosition);
        simpleShader.setUniformMat4("projectionView", cameraProjection * view);

        terrainTexture.bind();

        world.draw(terrainShader, simpleShader);

        glm::vec4 inversionColor = color::fromRGB(0xc8c8c8);

        if (showGui) {
            if (rayCast.collided) {
                blendModeInvert();

                simpleShader.use();
                simpleShader.setUniformVec4("color", inversionColor);
                drawThickWireCube(simpleShader, cubePos, 0.02f);
                //drawCube(simpleShader, cubePos, glm::vec3{1.05f});

                blendModeNormal();
            }

            if (world.showChunkBorders) {
                //DrawLine3D(Vector3Zero(), (Vector3){0, CHUNK_HEIGHT, 0}, BLUE);
                simpleShader.setUniformVec4("color", color::fromRGBA(0xffffff80));
                glDisable(GL_CULL_FACE);
                glm::ivec3 chunkPosition = worldToChunkV(player->position);
                chunkPosition += glm::ivec3{1, 0, 1};
                drawCube(simpleShader, glm::vec3{chunkPosition} * glm::vec3{chunkSize} + glm::vec3{chunkSize} / glm::vec3{-2.f, 2.f, -2.f}, glm::vec3{chunkSize});
                glEnable(GL_CULL_FACE);
                
            }
        }

        if (showGui) {
            glDisable(GL_DEPTH_TEST);

            int screenMiddleX = windowWidth / 2;
            int screenMiddleY = windowHeight / 2;

            glm::mat4 guiView{1.f};
            terrainShader.use();
            terrainShader.setUniformMat4("projectionView", guiCameraProjection * guiView);
            simpleShader.setUniformMat4("projectionView", guiCameraProjection * guiView);

            float iconScale = 48.f;
            float cornerOffset = 75.f;
            glm::mat4 indicator{1.f};
            indicator = glm::translate(indicator, {screenMiddleX - cornerOffset, screenMiddleY - cornerOffset, 0});
            indicator = glm::scale(indicator, {iconScale, iconScale, 0.1f});
            indicator = glm::rotate(indicator, glm::pi<float>() / 8.f, {1.f, 0.f, 0.f});
            indicator = glm::rotate(indicator, glm::pi<float>() / 4.f, {0.f, 1.f, 0.f});
            terrainShader.setUniformMat4("model", indicator);
            blocks[selectedBlock]->mesh.draw();

            glm::mat4 cornerTransform = glm::translate(glm::mat4{1.f}, {-windowWidth / 2.f, windowHeight / 2.f, 0.f});
            terrainShader.setUniformMat4("projectionView", guiCameraProjection * guiView * cornerTransform);
            terrainShader.setUniformMat4("model", glm::mat4{1.f});
            font.texture.bind();

            TextBatch text{font};
            text.drawString({5, -20}, "{} FPS", static_cast<int>(1.f / (deltaTime / 20.f)));
            text.drawString({5, -40}, "P: {:.2f}", player->position);
            text.drawString({5, -60}, "V: {:.2f}", player->velocity);
            text.drawString({5, -80}, "L: {:.2f}", lookVec);
            text.drawString({5, -100}, "RD: {}", world.renderDistance);
            text.drawString({5, -120}, "E: {}", world.entities.size());
            text.drawHighlighted(terrainShader);

            blendModeInvert();

            simpleShader.use();
            simpleShader.setUniformVec4("color", color::white);
            drawRectangle(simpleShader, {0.f, 0.f}, {16.f, 2.f});
            drawRectangle(simpleShader, {0.f, 0.f}, {2.f, 16.f});
            drawRectangle(simpleShader, {0.f, 0.f}, {2.f, 2.f});

            blendModeNormal();

            glEnable(GL_DEPTH_TEST);
        }


        checkOpenglErrors();

        glfwSwapBuffers(window);

        inputStatePrepare();
        glfwPollEvents();
    }

    unloadMeshes();
    unregisterBlocks();
}


int main() {
    Config::settings = Config{"config.ini"};

    if (!glfwInit()) {
        Logger::fatal("GLFW failed to initialize");
    }

    glfwSetErrorCallback(errorCallbackGlfw);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);

    GLFWwindow* window = glfwCreateWindow(windowWidth, windowHeight, "Fakecraft", nullptr, nullptr);
    if (window == nullptr) {
        Logger::fatal("Failed to create GLFW window");
    }

    glfwMakeContextCurrent(window);

    glfwSetKeyCallback(window, keyCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, cursorPosCallback);
    glfwSetScrollCallback(window, scrollCallback);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        Logger::fatal("Failed to initialize GLAD");
    }

    int glFlags = 0;
    glGetIntegerv(GL_CONTEXT_FLAGS, &glFlags);
    if (glFlags & GL_CONTEXT_FLAG_DEBUG_BIT) {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(openglDebugCallback, nullptr);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, true);
    } else {
        Logger::warning("No debug context");
    }

    runGame(window);
    
    glfwTerminate();
}
