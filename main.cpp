#include <iostream>
#include "raylib.h"
#include "imgui/imgui.h"
#include "rlimgui/rlImGui.h"
#include "MinecraftSkin.h"
#include "State.h"
#include "UserInterface.h"

int main(int argc, char* argv[])
{
    InitWindow(1280, 720, "Skin viewer | Default skin");
    SetWindowState(FLAG_VSYNC_HINT | FLAG_WINDOW_RESIZABLE);
    SetExitKey(0);

    rlImGuiSetup(true);

    State.camera = { 0 };
    State.camera.position = (Vector3){ 3.0f, 3.0f, -3.0f };
    State.camera.target = (Vector3){ 0.0f, 1.0f, 0.0f };
    State.camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    State.camera.fovy = 45.0f;
    State.camera.projection = CAMERA_PERSPECTIVE;

    State.classicModel = LoadModel("assets/player_wide.gltf");
    State.slimModel = LoadModel("assets/player_slim.gltf");

    Vector3 position = { 0.0f, 0.0f, 0.0f };
    Shader alphaShader = LoadShader(0, TextFormat("assets/discard.fs"));

    State.classicModel.materials[1].shader = alphaShader;
    State.slimModel.materials[1].shader = alphaShader;

    Image steveSkin = LoadImage("assets/steve.png");
    Image alexSkin = LoadImage("assets/alex.png");
    State.steve = MinecraftSkin::LoadSkinIntoStruct(steveSkin);
    State.alex = MinecraftSkin::LoadSkinIntoStruct(alexSkin);
    UnloadImage(steveSkin);
    UnloadImage(alexSkin);

    SetMaterialTexture(&State.classicModel.materials[1], MATERIAL_MAP_DIFFUSE, State.steve.texture);
    SetMaterialTexture(&State.slimModel.materials[1], MATERIAL_MAP_DIFFUSE, State.alex.texture);

    if (argc > 1) {
        if (IsFileExtension(argv[1], "png")) {
            if (!MinecraftSkin::LoadSkinFromPNG(argv[1])) {
                UserInterface::ShowInvalidTexturePopup();
            }
        } else {
            UserInterface::ShowInvalidFilePopup();
        }
    }

    while (!WindowShouldClose() && !State.quit)
    {
        if (IsFileDropped()) {
            FilePathList list = LoadDroppedFiles();
            if (IsFileExtension(list.paths[0], "png")) {
                if (!MinecraftSkin::LoadSkinFromPNG(list.paths[0])) {
                    UserInterface::ShowInvalidTexturePopup();
                }
            } else {
                UserInterface::ShowInvalidFilePopup();
            }
            UnloadDroppedFiles(list);
        }

        if (!IsCursorHidden()) {
            if (IsCursorOnScreen() && !ImGui::GetIO().WantCaptureMouse) {
                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                    DisableCursor();
                }
            }
        } else {
            UpdateCamera(&State.camera, CAMERA_FREE);

            if (IsKeyPressed(KEY_ESCAPE)) {
                EnableCursor();
            }
        }

        BeginDrawing();

        ClearBackground({50, 50, 50, 255});
        BeginMode3D(State.camera);
        DrawModel(State.isSlim ? State.slimModel : State.classicModel, position, 1.0f, WHITE);
        DrawGrid(10, 1.0f);
        EndMode3D();

        rlImGuiBegin();

        UserInterface::RunUI();

        rlImGuiEnd();

        EndDrawing();
    }
    UnloadModel(State.classicModel);
    UnloadModel(State.slimModel);
    UnloadTexture(State.steve.texture);
    UnloadTexture(State.alex.texture);
    UnloadTexture(State.loadedSkin.texture);
    rlImGuiShutdown();
    CloseWindow();

    return 0;
}