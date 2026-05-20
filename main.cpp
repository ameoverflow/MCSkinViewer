#include <filesystem>

#include "Assets.h"
#include "base64.hpp"
#include "raylib.h"
#include "raymath.h"
#include "imgui/imgui.h"
#include "rlimgui/rlImGui.h"
#include "MinecraftSkin.h"
#include "State.h"
#include "UserInterface.h"

Vector3 position = { 0.0f, 0.0f, 0.0f };

std::string GetSystemTempPath() {
#ifdef _WIN32
    char tempFolder[MAX_PATH];
    GetTempPathA(MAX_PATH, tempFolder);
    return std::string(tempFolder);
    #else
    return "/tmp/";
#endif
}

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


    // unpack models to memory, raylib doesnt support loading models from a string in case of .gltf files
    std::string playerClassicPath = GetSystemTempPath() + "player_classic.gltf";
    std::string playerSlimPath = GetSystemTempPath() + "player_slim.gltf";

    TraceLog(LOG_INFO, "Unpacking classic model...");
    SaveFileText(playerClassicPath.c_str(), playerClassic.c_str());

    TraceLog(LOG_INFO, "Unpacking slim model...");
    SaveFileText(playerSlimPath.c_str(), playerSlim.c_str());

    State.classicModel = LoadModel(playerClassicPath.c_str());
    State.slimModel = LoadModel(playerSlimPath.c_str());

    Shader alphaShader = LoadShaderFromMemory(nullptr, outerShader.c_str());
    State.classicModel.materials[1].shader = alphaShader;
    State.slimModel.materials[1].shader = alphaShader;

    std::string steveTextureData = base64::from_base64(steveTexture);
    std::string alexTextureData = base64::from_base64(alexTexture);
    Image steveSkin = LoadImageFromMemory(".png", (const unsigned char*)steveTextureData.data(), (int)steveTextureData.size());
    Image alexSkin = LoadImageFromMemory(".png", (const unsigned char*)alexTextureData.data(), (int)alexTextureData.size());

    State.steve = MinecraftSkin::LoadSkinIntoStruct(steveSkin);
    State.alex = MinecraftSkin::LoadSkinIntoStruct(alexSkin);
    SetMaterialTexture(&State.classicModel.materials[1], MATERIAL_MAP_DIFFUSE, State.steve.texture);
    SetMaterialTexture(&State.slimModel.materials[1], MATERIAL_MAP_DIFFUSE, State.alex.texture);

    UnloadImage(steveSkin);
    UnloadImage(alexSkin);

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

        ClearBackground({State.backgroundColor[0] * 255.0f, State.backgroundColor[1] * 255.0f, State.backgroundColor[2] * 255.0f, 255});
        BeginMode3D(State.camera);
        //DrawModel(State.isSlim ? State.slimModel : State.classicModel, position, 1.0f, WHITE);

        Matrix transform = MatrixScale(1, 1, 1);
        transform = MatrixMultiply(transform, MatrixTranslate(position.x, position.y, position.z));
        for (int i = 0; i < 12; i++) {
            if (!State.enabledMeshes[i]) continue;
            int matIndex = (State.isSlim ? State.slimModel : State.classicModel).meshMaterial[i];
            DrawMesh( (State.isSlim ? State.slimModel : State.classicModel).meshes[i],  (State.isSlim ? State.slimModel : State.classicModel).materials[matIndex], transform);
        }

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