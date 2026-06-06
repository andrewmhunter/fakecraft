#include <raylib.h>
#include <raymath.h>
#include <stdbool.h>
#include <glad/glad.h>
#include <rlgl.h>
#include "block.h"
#include "world.h"
#include "util.h"
#include "collision.h"
#include "entity.h"
#include "serialize.h"
#include "logger.h"
#include "point.h"


void drawThickWireCube(Vector3 position, Color color, float lineWidth) {
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

            Vector3 xPos = Vector3Add(position, (Vector3){axisOffset, offsetA, offsetB});
            DrawCube(xPos, shortLineLength, lineWidth, lineWidth, color);

            Vector3 yPos = Vector3Add(position, (Vector3){offsetA, axisOffset, offsetB});
            DrawCube(yPos, lineWidth, longLineLength, lineWidth, color);

            Vector3 zPos = Vector3Add(position, (Vector3){offsetA, offsetB, axisOffset});
            DrawCube(zPos, lineWidth, lineWidth, shortLineLength, color);
        }
    }
}

int main(void) {
    // Pass raylibs logs through our own logger
    SetTraceLogCallback(raylibLogCallback);

    // We only want to show raylibs warnings as it emits way to many
    // info logs
    SetTraceLogLevel(LOG_WARNING);

    setLogLevel(LOG_INFO);

    // No matter where you run the executable from it will still be able to
    // access the resources in its directory
    ChangeDirectory(GetApplicationDirectory());

    //SetConfigFlags(FLAG_VSYNC_HINT);
    InitWindow(1280, 720, "Fakecraft");
    int targetFps = 60;
    //SetTargetFPS(targetFps);

    SetExitKey(KEY_ESCAPE);
    DisableCursor();

    makeSaveDirectories();

    Shader shader = LoadShader("shader.vs.glsl", "shader.fs.glsl");
    ASSERT(IsShaderValid(shader));

    // TODO: This doesn't check if a shader compilation error occured
    // TODO: Need a better way to store uniform locations and switch out model
    // uniform for the one used by raylib
    int shaderCamUniform = GetShaderLocation(shader, "camPos");
    shaderModelUniform = GetShaderLocation(shader, "model");
    shaderSkylight = GetShaderLocation(shader, "skyLight");
    shaderFogColor = GetShaderLocation(shader, "fogColor");
    shaderFogDistance = GetShaderLocation(shader, "fogDistance");
    shaderFogDropoff = GetShaderLocation(shader, "fogDropoff");

    // TODO: Custon font loading for minecrafts font format
    font = LoadFont("resources/defaultSpritefont.png");
    ASSERT(IsFontValid(font));

    Texture2D terrain = LoadTexture("resources/alphaTerrain.png");
    ASSERT(IsTextureValid(terrain));

    // TODO: Figure out mipmaps
    GenTextureMipmaps(&terrain);

    //Texture2D clouds = LoadTexture("resources/environment/clouds.png");
    //ASSERT(IsTextureValid(clouds));

    Camera3D cam = {
        .position = {0.f, 0.f, 0.f},
        .target = {0.f, 0.f, 0.f},
        .up = {0.f, 1.f, 0.f},
        .fovy = 90.f,
        .projection = CAMERA_PERSPECTIVE
    };

    // TODO: Prerender all the block sprites somehow
    Camera3D guiCam = {
        .position = {.x = 0.f, .y = 0.f, .z = -10.f},
        .target = {0, 0, 0},
        .up = {.x = 0.f, .y = 1.f, .z = 0.f},
        .fovy = 90.f,
        .projection = CAMERA_ORTHOGRAPHIC 
    };

    Block selectedBlock = BLOCK_PLANKS;

    Material material = LoadMaterialDefault();
    material.maps[MATERIAL_MAP_DIFFUSE].texture = terrain;
    material.shader = shader;
    ASSERT(IsMaterialValid(material));

    registerBlocks();

#ifdef DEFAULT_SET_SEED
    srand(DEFAULT_SET_SEED);
#else
    randomizeSeed();
#endif

    World world;
    worldInit(&world);
    Entity* player = world.player;

    float maxY = 0.f;

    bool showGui = true;

    while (!WindowShouldClose()) {
        if (IsKeyPressed(KEY_Q) && targetFps > 0) {
            targetFps -= 10;
            SetTargetFPS(targetFps);
        }

        if (IsKeyPressed(KEY_E)) {
            targetFps += 10;
            SetTargetFPS(targetFps);
        }


        if (IsKeyPressed(KEY_R)) {
            maxY = 0.f;
        }

        float deltaTime = GetFrameTime();
        deltaTime *= 20.f;

        if (IsKeyPressed(KEY_F2)) {
            saveScreenshot();
        }

        if (IsKeyPressed(KEY_F1)) {
            showGui = !showGui;
        }

        if (IsKeyPressed(KEY_F6)) {
            world.showChunkBorders = !world.showChunkBorders;
        }

        if (IsKeyPressed(KEY_F11)) {
            ToggleBorderlessWindowed();
        }

        if (IsKeyPressed(KEY_F8) && world.renderDistance > 0) {
            world.renderDistance--;
        }

        if (IsKeyPressed(KEY_F9)) {
            world.renderDistance++;
        }

        worldUpdate(&world, deltaTime);

        cam.position = Vector3Add(player->position, (Vector3){0, PLAYER_EYE, 0});

        Vector3 lookVec = {1.f, 0.f, 0.f};
        lookVec = Vector3RotateByAxisAngle(lookVec, (Vector3){0.f, 0.f, 1.f}, player->pitch);
        lookVec = Vector3RotateByAxisAngle(lookVec, (Vector3){0.f, 1.f, 0.f}, player->yaw);

        cam.target = Vector3Add(cam.position, lookVec);

        WalkCollision rayCast = ddaCastRay(&world, cam.position, lookVec, 8.f);
        Vector3 cubePos = Vector3AddValue(pointToVector3(rayCast.blockAt), 0.5f);

        float initialBreakTime = 0.3f;
        float repeatedBreakTime = 0.15f;

        if (rayCast.collided) {
            if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
                worldTryPlaceBlock(&world, rayCast.blockBefore, selectedBlock);
                timerResetTime(&player->breakTimer, initialBreakTime);
            }

            if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT) && timerUpdate(&player->breakTimer)) {
                worldTryPlaceBlock(&world, rayCast.blockBefore, selectedBlock);
                timerResetTime(&player->breakTimer, repeatedBreakTime);
            }


            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                worldSetBlock(&world, rayCast.blockAt, BLOCK_AIR);
                timerResetTime(&player->breakTimer, initialBreakTime);
            }

            if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && timerUpdate(&player->breakTimer)) {
                worldSetBlock(&world, rayCast.blockAt, BLOCK_AIR);
                timerResetTime(&player->breakTimer, repeatedBreakTime);
            }


            if (IsMouseButtonPressed(MOUSE_BUTTON_MIDDLE)) {
                selectedBlock = worldGetBlock(&world, rayCast.blockAt);
            }
        }

        for (KeyboardKey key = KEY_ONE; key <= KEY_NINE; ++key) {
            if (IsKeyPressed(key)) {
                Block block = key - KEY_ONE + 1;
                selectedBlock = block;
            }
        }

        if (IsKeyPressed(KEY_H)) {
            Entity* mob = spawnEntity(&world, ENTITY_MOB, player->position, 0.6f, 1.8f);
            mob->velocity = Vector3Scale(player->velocity, 2.f);
        }

        if (GetMouseWheelMove() < 0) {
            selectedBlock -= 1;
        }
        if (GetMouseWheelMove() > 0) {
            selectedBlock += 1;
        }

        selectedBlock = wrapInt(selectedBlock, 2, BLOCK_COUNT);

        SetShaderValue(shader, shaderCamUniform, &cam.position, SHADER_UNIFORM_VEC3);

        BeginDrawing();
        ClearBackground(world.skyColor);

        BeginMode3D(cam);

        worldDraw(&world, material);

        unsigned char inversionColorValue = 200;
        Color inversionColor = (Color){.r = inversionColorValue, .g = inversionColorValue, .b = inversionColorValue};

        if (showGui) {
            if (rayCast.collided) {
                //BeginBlendMode(BLEND_SUBTRACT_COLORS);
                glEnable(GL_BLEND);
                glBlendFunc(GL_ONE, GL_ONE);
                glBlendEquation(GL_FUNC_SUBTRACT);

                drawThickWireCube(cubePos, inversionColor, 0.02f);
                rlDrawRenderBatchActive();

                //EndBlendMode();
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                glBlendEquation(GL_FUNC_ADD);
            }

            if (world.showChunkBorders) {
                DrawLine3D(Vector3Zero(), (Vector3){0, CHUNK_HEIGHT, 0}, BLUE);
            }
        }

        EndMode3D();

        if (showGui) {
            int screenMiddleX = GetScreenWidth() / 2;
            int screenMiddleY = GetScreenHeight() / 2;

            BeginMode3D(guiCam);

            float fogDistance = 10000000.f;
            float fogDropoff = fogDistance;
            SetShaderValue(material.shader, shaderFogDistance, &fogDistance, SHADER_UNIFORM_FLOAT);
            SetShaderValue(material.shader, shaderFogDropoff, &fogDropoff, SHADER_UNIFORM_FLOAT);

            Matrix indicator = MatrixIdentity();
            indicator = MatrixMultiply(indicator, MatrixScale(5.f, 5.f, 5.f));
            indicator = MatrixMultiply(indicator, MatrixRotateY(PI / 4));
            indicator = MatrixMultiply(indicator, MatrixRotateX(-PI / 8));
            indicator = MatrixMultiply(indicator, MatrixTranslate(-75, 35, 0));
            DrawMesh(blocks[selectedBlock].mesh, material, indicator);

            EndMode3D();


            renderText(0, 0, "%d FPS", GetFPS());
            renderText(0, 20, "P: %s", formatVector3(player->position));
            renderText(0, 40, "V: %s", formatVector3(player->velocity));
            renderText(0, 60, "L: %s", formatVector3(lookVec));
            renderText(0, 80, "RD: %d", world.renderDistance);
            renderText(0, 100, "E: %lu", world.entities.length);

            maxY = MAX(cam.position.y, maxY);
            //renderText(0, 80, "%f", maxY - cam.position.y);

            //DrawText(TextFormat("%d: %s", selectedBlock, blocks[selectedBlock].name), 0, 20, 20, WHITE);

            //DrawLine(screenMiddleX - 4, screenMiddleY, screenMiddleX + 4, screenMiddleY, WHITE);
            //DrawLine(screenMiddleX, screenMiddleY - 4, screenMiddleX, screenMiddleY + 4, WHITE);

            BeginBlendMode(BLEND_SUBTRACT_COLORS);
            DrawRectangle(screenMiddleX - 8, screenMiddleY - 1, 16, 2, inversionColor);
            DrawRectangle(screenMiddleX - 1, screenMiddleY - 8, 2, 16, inversionColor);
            DrawRectangle(screenMiddleX - 1, screenMiddleY - 1, 2, 2, inversionColor);
            EndBlendMode();
        }


        EndDrawing();

#ifdef PROFILING_STARTUP
        break;
#endif
    }

    worldUnload(&world);

    UnloadMaterial(material);
    UnloadTexture(terrain);
}


