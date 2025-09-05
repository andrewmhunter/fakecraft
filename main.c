#include <raylib.h>
#include <raymath.h>
#include <stdbool.h>
#include "block.h"
#include "chunk.h"
#include "world.h"
#include "util.h"

// Game

int main(void) {
    //SetConfigFlags(FLAG_VSYNC_HINT);
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(1280, 720, "Fakecraft");

    Shader shader = LoadShader("shader.vs", "shader.fs");
    assert(IsShaderValid(shader));
    int shaderCamUniform = GetShaderLocation(shader, "camPos");
    shaderModelUniform = GetShaderLocation(shader, "model");

    font = LoadFont("default.png");
    assert(IsFontValid(font));

    Texture2D terrain = LoadTexture("alphaTerrain.png");
    assert(IsTextureValid(terrain));

    Vector3 cubePos = {0, 0, 0};

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

    randomizeSeed();

    World world;
    worldInit(&world);

    //SetTargetFPS(60);
    SetExitKey(KEY_ESCAPE);
    DisableCursor();

    float lookAngle = 0.f;
    float verticalLookAngle = 0.f;

    while (!WindowShouldClose()) {
        float delta = GetFrameTime();

        if (IsKeyPressed(KEY_F2)) {
            saveScreenshot();
        }

        if (IsKeyPressed(KEY_F6)) {
            world.showChunkBorders = !world.showChunkBorders;
        }

        if (IsKeyPressed(KEY_F11)) {
            ToggleBorderlessWindowed();
        }

        //UpdateCamera(&cam, CAMERA_FREE);

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

        if (IsKeyDown(KEY_LEFT_CONTROL)) {
            movement.y -= 1.f;
        }

        if (IsKeyDown(KEY_SPACE)) {
            movement.y += 1.f;
        }

        //const float speed = 4.317;
        //const float speed = 5.612;
        float speed = 10.79f;

        if (IsKeyDown(KEY_LEFT_SHIFT)) {
            speed = 21.58f;
        }


        const float sensitivity = 0.0025f;
        Vector2 mouseDelta = GetMouseDelta();
        lookAngle -= mouseDelta.x * sensitivity;
        verticalLookAngle -= mouseDelta.y * sensitivity;
        verticalLookAngle = Clamp(verticalLookAngle, -PI / 2 + 0.0001, PI / 2 - 0.0001);

        movement = Vector3RotateByAxisAngle(movement, (Vector3){0.f, 1.f, 0.f}, lookAngle);
        movement = Vector3Normalize(movement);
        movement = Vector3Scale(movement, delta * speed);

        //UpdateCameraPro(&cam, movement, Vector3Zero(), 0.f);
        cam.position = Vector3Add(cam.position, movement);

        Vector3 lookVec = {1.f, 0.f, 0.f};
        lookVec = Vector3RotateByAxisAngle(lookVec, (Vector3){0.f, 0.f, 1.f}, verticalLookAngle);
        lookVec = Vector3RotateByAxisAngle(lookVec, (Vector3){0.f, 1.f, 0.f}, lookAngle);

        cam.target = Vector3Add(cam.position, lookVec);

        //Vector3 lookVec = Vector3Subtract(cam.target, cam.position);


        Collision rayCast = worldWalkRay(&world, cam.position, lookVec, 8.f);
        cubePos = rayCast.collisionAt;

        if (rayCast.collided) {
            cubePos = (Vector3){floor(cubePos.x), floor(cubePos.y), floor(cubePos.z)};
            cubePos = Vector3Add(cubePos, Vector3Scale(Vector3One(), 0.5f));

            if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
                worldTryPlaceBlock(&world, vector3ToPoint(rayCast.collisionBefore), selectedBlock);
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

        if (IsKeyPressed(KEY_Q)) {
            selectedBlock -= 1;
        }
        if (IsKeyPressed(KEY_E)) {
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
            DrawCubeWires(cubePos, 1, 1, 1, WHITE);
            DrawCubeWires(cubePos, 1.002, 1.002, 1.002, WHITE);
        }

        //DrawSphere(rayCast.collisionAt, 0.3, RED);

        if (world.showChunkBorders) {
            DrawLine3D(Vector3Zero(), (Vector3){0, CHUNK_HEIGHT, 0}, BLUE);
        }


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
        renderText(0, 20, "%s", formatVector3(cam.position));

        //DrawText(TextFormat("%d: %s", selectedBlock, blocks[selectedBlock].name), 0, 20, 20, WHITE);

        //DrawLine(screenMiddleX - 4, screenMiddleY, screenMiddleX + 4, screenMiddleY, WHITE);
        //DrawLine(screenMiddleX, screenMiddleY - 4, screenMiddleX, screenMiddleY + 4, WHITE);
        DrawRectangleLines(screenMiddleX - 2, screenMiddleY - 2, 5, 5, WHITE);

        EndDrawing();

        //break;
    }

    worldUnload(&world);

    UnloadMaterial(material);
    UnloadTexture(terrain);
}


