#include <raylib.h>
#include <raymath.h>
#include <stdbool.h>
#include "block.h"
#include "chunk.h"
#include "world.h"
#include "util.h"
#include "collision.h"
#include "entity.h"

#include <rlgl.h>
#include <stdio.h>

// Game

int main(void) {
    ChangeDirectory(GetApplicationDirectory());
    printf("%s\n", GetWorkingDirectory());

    //SetConfigFlags(FLAG_VSYNC_HINT);
    //SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(1280, 720, "Fakecraft");
    int targetFps = 60;
    SetTargetFPS(targetFps);

    SetExitKey(KEY_ESCAPE);
    DisableCursor();


    Shader shader = LoadShader("shader.vs", "shader.fs");
    assert(IsShaderValid(shader));
    int shaderCamUniform = GetShaderLocation(shader, "camPos");
    shaderModelUniform = GetShaderLocation(shader, "model");

    font = LoadFont("default.png");
    assert(IsFontValid(font));

    Texture2D terrain = LoadTexture("alphaTerrain.png");
    //terrain = LoadTexture("blank.png");
    assert(IsTextureValid(terrain));

    //Texture2D clouds = LoadTexture("resources/environment/clouds.png");
    //assert(IsTextureValid(clouds));

    Camera3D cam = {
        .up = {0.f, 1.f, 0.f},
        .fovy = 90.f,
        .target = {0.f, 23.f, 1.f},
        .position = {0.f, 22.62f, 0.f},
        .projection = CAMERA_PERSPECTIVE
    };

    Camera3D guiCam = {
        .up = {.x = 0.f, .y = 1.f, .z = 0.f},
        .fovy = 90.f,
        .target = {0, 0, 0},
        .position = {.x = 0.f, .y = 0.f, .z = -10.f},
        .projection = CAMERA_ORTHOGRAPHIC 
    };

    Block selectedBlock = BLOCK_PLANKS;

    Material material = LoadMaterialDefault();
    material.maps[MATERIAL_MAP_DIFFUSE].texture = terrain;
    material.shader = shader;
    assert(IsMaterialValid(material));

    registerBlocks();

#ifdef DEFAULT_SET_SEED
    srand(DEFAULT_SET_SEED);
#else
    randomizeSeed();
#endif

    World world;
    worldInit(&world);
    cam.position = (Vector3) {
        0.5f,
        worldGetChunk(&world, point(0, 0, 0))->surfaceHeight[0][0] + 2.62,
        0.5f
    };

    //Entity player;
    //entityInit(&player, ENTITY_PLAYER, &world, (Vector3){0, worldGetChunk(&world, point(0, 0, 0))->surfaceHeight[0][0] + PLAYER_EYE + 2.f, 0}, 0.6f, 1.8f);

    Vector3 playerPosition = (Vector3){
        0,
        worldGetChunk(&world, point(0, 0, 0))->surfaceHeight[0][0] + PLAYER_EYE + 2.f,
        0
    };

    Entity* player = spawnEntity(&world, ENTITY_PLAYER, playerPosition, 0.6f, 1.8f);

    float lookAngle = 0.f;
    float verticalLookAngle = 0.f;

    bool flying = true;

    float maxY = 0.f;

    while (!WindowShouldClose()) {
        if (IsKeyPressed(KEY_Q)) {
            targetFps -= 10;
            SetTargetFPS(targetFps);
        }

        if (IsKeyPressed(KEY_E)) {
            targetFps += 10;
            SetTargetFPS(targetFps);
        }

        float deltaTime = GetFrameTime();
        deltaTime *= 20.f;

        if (IsKeyPressed(KEY_F2)) {
            saveScreenshot();
        }

        if (IsKeyPressed(KEY_F6)) {
            world.showChunkBorders = !world.showChunkBorders;
        }

        if (IsKeyPressed(KEY_F11)) {
            ToggleBorderlessWindowed();
        }

        if (IsKeyPressed(KEY_F)) {
            flying = !flying;
        }

        Vector3 movement = Vector3Zero();
        if (IsKeyDown(KEY_W)) {
            movement.x += 1.f;
        }

        if (IsKeyDown(KEY_S)) {
            movement.x -= 1.f;
        }

        if (IsKeyDown(KEY_A)) {
            movement.z -= 1.f;
        }

        if (IsKeyDown(KEY_D)) {
            movement.z += 1.f;
        }

        if (IsKeyPressed(KEY_R)) {
            maxY = 0.f;
        }

        if (flying && IsKeyDown(KEY_LEFT_CONTROL)) {
            movement.y -= 1.f;
        }

        if (flying && IsKeyDown(KEY_SPACE)) {
            //movement.y += 0.5f;
        }

        /*float speed = flying ? 10.79f : 4.317;

        if (IsKeyDown(KEY_LEFT_SHIFT)) {
            speed = flying ? 21.58f : 5.612;
        }*/
        float speed = 0.098f * deltaTime;
        //float speed = 0.2f * delta;


        Vector2 mouseDelta = GetMouseDelta();
        lookAngle -= mouseDelta.x * SENSITIVITY;
        verticalLookAngle -= mouseDelta.y * SENSITIVITY;
        verticalLookAngle = Clamp(verticalLookAngle, -PI / 2 + 0.0001, PI / 2 - 0.0001);

        movement = Vector3RotateByAxisAngle(movement, (Vector3){0.f, 1.f, 0.f}, lookAngle);
        movement = Vector3Normalize(movement);
        movement = Vector3Scale(movement, speed);

        if (IsKeyPressed(KEY_SPACE)) {
            player->velocity.y += 0.5f;
        }

        player->velocity = Vector3Add(player->velocity, movement);

        worldUpdate(&world, deltaTime);

        cam.position = Vector3Add(player->position, (Vector3){0, PLAYER_EYE, 0});

        Vector3 lookVec = {1.f, 0.f, 0.f};
        lookVec = Vector3RotateByAxisAngle(lookVec, (Vector3){0.f, 0.f, 1.f}, verticalLookAngle);
        lookVec = Vector3RotateByAxisAngle(lookVec, (Vector3){0.f, 1.f, 0.f}, lookAngle);

        cam.target = Vector3Add(cam.position, lookVec);

        WalkCollision rayCast = ddaCastRay(&world, cam.position, lookVec, 8.f);
        Vector3 cubePos = Vector3AddValue(pointToVector3(rayCast.blockAt), 0.5f);

        if (rayCast.collided) {
            if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
                worldTryPlaceBlock(&world, rayCast.blockBefore, selectedBlock);
            }

            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                worldSetBlock(&world, rayCast.blockAt, BLOCK_AIR);
            }

            if (IsMouseButtonPressed(MOUSE_BUTTON_MIDDLE)) {
                selectedBlock = worldGetBlock(&world, rayCast.blockAt);
            }
        }

        for (KeyboardKey key = KEY_ONE; key <= KEY_NINE; ++key) {
            Block block = key - KEY_ONE + 1;
            if (IsKeyPressed(key) && block < BLOCK_COUNT) {
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
        ClearBackground(SKYBLUE);

        BeginMode3D(cam);

        //drawChunk(&chunk, material);
        worldDraw(&world, material);

        if (rayCast.collided) {
            float epsilon = 0.002;
            float low = 1.f - epsilon;
            float offset = 1.f + epsilon;
            DrawCubeWires(cubePos, 1, 1, 1, WHITE);
            DrawCubeWires(cubePos, offset, offset, offset, WHITE);
            DrawCubeWires(cubePos, low, low, offset, WHITE);
            DrawCubeWires(cubePos, low, offset, low, WHITE);
            DrawCubeWires(cubePos, offset, low, low, WHITE);
        }

        //DrawSphere(rayCast2.collisionAt, 0.3, RED);
        //DrawSphere(pointToVector3(rayCast2.blockAt), 0.3, BLUE);

        if (world.showChunkBorders) {
            DrawLine3D(Vector3Zero(), (Vector3){0, CHUNK_HEIGHT, 0}, BLUE);
        }

        //DrawCubeWires(Vector3Subtract(cam.position, (Vector3){0, PLAYER_EYE - 0.9f, 0}), 0.6f, 1.8f, 0.6f, WHITE);

        EndMode3D();

        int screenMiddleX = GetScreenWidth() / 2;
        int screenMiddleY = GetScreenHeight() / 2;

        BeginMode3D(guiCam);
        Matrix indicator = MatrixScale(5.f, 5.f, 5.f);
        indicator = MatrixMultiply(indicator, MatrixRotateY(PI / 4));
        indicator = MatrixMultiply(indicator, MatrixRotateX(-PI / 8));
        indicator = MatrixMultiply(indicator, MatrixTranslate(-75, 35, 0));
        DrawMesh(blocks[selectedBlock].mesh, material, indicator);


        EndMode3D();

        renderText(0, 0, "%d FPS", GetFPS());
        renderText(0, 20, "P: %s", formatVector3(player->position));
        renderText(0, 40, "V: %s", formatVector3(player->velocity));
        renderText(0, 60, "L: %s", formatVector3(lookVec));
        renderText(0, 100, "%.02f m/s", Vector3Length(player->velocity) * GetFPS());

        maxY = MAX(cam.position.y, maxY);
        renderText(0, 80, "%f", maxY - cam.position.y);

        //DrawText(TextFormat("%d: %s", selectedBlock, blocks[selectedBlock].name), 0, 20, 20, WHITE);

        //DrawLine(screenMiddleX - 4, screenMiddleY, screenMiddleX + 4, screenMiddleY, WHITE);
        //DrawLine(screenMiddleX, screenMiddleY - 4, screenMiddleX, screenMiddleY + 4, WHITE);
        DrawRectangle(screenMiddleX - 8, screenMiddleY - 1, 16, 2, WHITE);
        DrawRectangle(screenMiddleX - 1, screenMiddleY - 8, 2, 16, WHITE);


        EndDrawing();

#ifdef PROFILING_STARTUP
        break;
#endif
    }

    worldUnload(&world);

    UnloadMaterial(material);
    UnloadTexture(terrain);
}


