#include <raylib.h>
#include <raymath.h>
#include <time.h>
#include <stdbool.h>
#include <stdarg.h>
#include <m-lib/m-array.h>
#include <m-lib/m-dict.h>
#include <m-lib/m-string.h>
#include "mesh.h"
#include "block.h"
#include "chunk.h"
#include "world.h"

// Game

void saveScreenshot(void) {
    // https://stackoverflow.com/questions/1531055/time-into-string-with-hhmmss-format-c-programming
    time_t currentTime;
    struct tm* local;
    time(&currentTime);
    local = localtime(&currentTime);
    char fileName[64];
    strftime(fileName, sizeof(fileName) - 1, "screenshot%FT%T.png", local);
    fileName[sizeof(fileName) - 1] = '\0';
    TakeScreenshot(fileName);
}

Font font;

void renderText(int x, int y, const char* format, ...) {
    va_list args;
    va_start(args, format);

    string_t string;
    string_init_vprintf(string, format, args);

    va_end(args);

    DrawTextEx(font, string_get_cstr(string), (Vector2){x, y}, 16, 2, WHITE);

    string_clear(string);
}

int main(void) {
    //SetConfigFlags(FLAG_VSYNC_HINT);
    InitWindow(1280, 720, "Hello");

    Shader shader = LoadShader("shader.vs", "shader.fs");
    if (!IsShaderValid(shader)) {
        TraceLog(LOG_FATAL, "Invalid shader");
    }

    //Font font = LoadFont("alagard.png");
    font = LoadFont("default.png");

    Texture2D terrain = LoadTexture("alphaTerrain.png");

    Vector3 cubePos = {.x = 0, .y = 0, .z = 0};

    Camera3D cam = {
        .up = {.x = 0.f, .y = 1.f, .z = 0.f},
        .fovy = 90.f,
        .target = cubePos,
        .position = {.x = 0.2f, .y = 23.f, .z = 0.2f},
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

    /*Mesh blockMeshes[] = {
        [BLOCK_AIR] = {0},
        [BLOCK_STONE] = cubeMesh(1, 0),
        [BLOCK_DIRT] = cubeMesh(2, 0),
        [BLOCK_PLANKS] = cubeMesh(4, 0),
    };*/

    for (int i = BLOCK_AIR + 1; i < BLOCK_COUNT; ++i) {
        BlockProperties* prop = &blocks[i];
        prop->mesh = cubeMesh(prop->texCoordX, prop->texCoordY);
    }

    randomizeSeed();

    World world;
    worldInit(&world);

    //SetTargetFPS(60);
    SetExitKey(KEY_ESCAPE);
    DisableCursor();
    while (!WindowShouldClose()) {
        float delta = GetFrameTime();

        if (IsKeyPressed(KEY_F2)) {
            saveScreenshot();
        }

        UpdateCamera(&cam, CAMERA_FREE);

        Vector3 lookDir = Vector3Normalize(Vector3Subtract(cam.target, cam.position));
        lookDir = Vector3Scale(lookDir, 2.f);

        Collision rayCast = worldWalkRay(&world, cam.position, lookDir, 8.f);
        cubePos = rayCast.collisionAt;

        if (rayCast.collided) {
            cubePos = (Vector3){floor(cubePos.x), floor(cubePos.y), floor(cubePos.z)};
            cubePos = Vector3Add(cubePos, Vector3Scale(Vector3One(), 0.5f));

            if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
                worldSetBlock(&world, vector3ToPoint(rayCast.collisionBefore), selectedBlock);
            }

            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                worldSetBlock(&world, vector3ToPoint(rayCast.collisionAt), BLOCK_AIR);
            }

            if (IsMouseButtonPressed(MOUSE_BUTTON_MIDDLE)) {
                selectedBlock = worldGetBlock(&world, vector3ToPoint(rayCast.collisionAt));
            }
        }

        for (KeyboardKey key = KEY_ONE; key <= KEY_NINE; ++key) {
            Block block = key - KEY_ONE + 1;
            if (IsKeyPressed(key) && block < BLOCK_COUNT) {
                selectedBlock = block;
            }
        }

        BeginDrawing();
        ClearBackground(SKYBLUE);

        BeginMode3D(cam);

        //drawChunk(&chunk, material);
        worldDraw(&world, material);

        if (rayCast.collided) {
            DrawCubeWires(cubePos, 1, 1, 1, WHITE);
            DrawCubeWires(cubePos, 1.002, 1.002, 1.002, WHITE);
        }

        //DrawSphere(rayCast.collisionAt, 0.3, RED);

        DrawLine3D(Vector3Zero(), (Vector3){0, CHUNK_HEIGHT, 0}, BLUE);


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
        renderText(0, 20, formatVector3(cam.position));

        //if (rayCast.collided) {
        //}
        //renderText(0, 60, formatVector3(rayCast.collisionBefore));
        //renderText(0, 80, formatPoint(vector3ToPoint(rayCast.collisionBefore)));
        //renderText(0, 100, formatVector3(worldToLocalV(rayCast.collisionBefore)));
        //renderText(0, 120, formatPoint(worldToChunkV(rayCast.collisionBefore)));

        //DrawText(TextFormat("%d", chunk.totalVertexCount), 0, 20, 20, WHITE);
        //DrawText(TextFormat("%d: %s", selectedBlock, blocks[selectedBlock].name), 0, 20, 20, WHITE);

        //DrawLine(screenMiddleX - 4, screenMiddleY, screenMiddleX + 4, screenMiddleY, WHITE);
        //DrawLine(screenMiddleX, screenMiddleY - 4, screenMiddleX, screenMiddleY + 4, WHITE);
        DrawRectangleLines(screenMiddleX - 2, screenMiddleY - 2, 5, 5, WHITE);


        EndDrawing();
    }

    UnloadTexture(terrain);
}


