#include <iostream>
#include "raylib.h"
#include "imgui/imgui.h"
#include "rlimgui/rlImGui.h"
#include "MinecraftSkin.h"

int skinType = 0;
bool freeCam = false;
Skin custom;
char username[17] = "";
Model classicModel, slimModel;
Camera camera;

bool invalidFilePopup, invalidSizePopup;

void runImGui() {
    if (invalidFilePopup) {
        ImGui::OpenPopup("File Error");
        invalidFilePopup = false;
    }

    if (invalidSizePopup) {
        ImGui::OpenPopup("Texture Error");
        invalidSizePopup = false;
    }

    bool open = true;
    ImGui::Begin("Options", &open, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize);

    if (IsCursorHidden()) {
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "-- CURSOR LOCKED --");
    }

    ImGui::Text("Camera coordinates: ");
    ImGui::Text("%f %f %f", camera.position.x, camera.position.y, camera.position.z);

    ImGui::Text("Camera target: ");
    ImGui::Text("%f %f %f", camera.target.x, camera.target.y, camera.target.z);

    ImGui::Separator();

    bool inputSubmitted = ImGui::InputText(" Minecraft username", username, sizeof(username), ImGuiInputTextFlags_EnterReturnsTrue);

    if (ImGui::Button("Get", ImVec2(64, 0)) || inputSubmitted)
    {
        custom = MinecraftSkin::LoadSkinFromMinecraft(username);
        if (custom.texture.id > 0) {
            SetWindowTitle(std::string("Skin viewer | Minecraft: " + std::string(username)).c_str());
            SetMaterialTexture(&classicModel.materials[1], MATERIAL_MAP_DIFFUSE, custom.texture);
            SetMaterialTexture(&slimModel.materials[1], MATERIAL_MAP_DIFFUSE, custom.texture);
        }
    }

    ImGui::Separator();

    ImGui::RadioButton("Steve Model (Classic)", &skinType, 0);
    ImGui::RadioButton("Alex Model (Slim)", &skinType, 1);

    if (custom.texture.id > 0) {
        ImGui::Separator();

        ImGui::Text("Custom skin");
        ImGui::Text("Skin type: %s", custom.isOldType ? "Pre-1.8" : "1.8+");
    }

    ImGui::End();

    if (ImGui::BeginPopupModal("File Error", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {

        ImGui::Text("Provided file is invalid. It needs to be a .png file.");

        if (ImGui::Button("OK", ImVec2(64, 0))) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    if (ImGui::BeginPopupModal("Texture Error", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {

        ImGui::Text("Texture must be 64x64 or 64x32.");

        if (ImGui::Button("OK", ImVec2(64, 0))) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

int main(int argc, char* argv[])
{
    InitWindow(1280, 720, "Skin viewer | Default skin");
    SetWindowState(FLAG_VSYNC_HINT | FLAG_WINDOW_RESIZABLE);
    SetExitKey(0);

    rlImGuiSetup(true);

    camera = { 0 };
    camera.position = (Vector3){ 3.0f, 3.0f, -3.0f };
    camera.target = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    classicModel = LoadModel("assets/player_wide.gltf");
    slimModel = LoadModel("assets/player_slim.gltf");

    Vector3 position = { 0.0f, 0.0f, 0.0f };
    Shader alphaShader = LoadShader(0, TextFormat("assets/discard.fs"));

    classicModel.materials[1].shader = alphaShader;
    slimModel.materials[1].shader = alphaShader;

    Texture2D steveTexture = MinecraftSkin::LoadSkinTexture("assets/steve.png").texture;
    Texture2D alexTexture = MinecraftSkin::LoadSkinTexture("assets/alex.png").texture;
    SetMaterialTexture(&classicModel.materials[1], MATERIAL_MAP_DIFFUSE, steveTexture);
    SetMaterialTexture(&slimModel.materials[1], MATERIAL_MAP_DIFFUSE, alexTexture);

    if (argc > 1) {
        if (IsFileExtension(argv[1], "png")) {
            Image skin = LoadImage(argv[1]);

            if (skin.width != 64 && skin.height != 64 && skin.height != 32) {
                invalidSizePopup = true;
            } else {
                custom = MinecraftSkin::LoadSkinTexture(argv[1]);
                SetWindowTitle(std::string("Skin viewer | " + std::string(argv[1])).c_str());
                SetMaterialTexture(&classicModel.materials[1], MATERIAL_MAP_DIFFUSE, custom.texture);
                SetMaterialTexture(&slimModel.materials[1], MATERIAL_MAP_DIFFUSE, custom.texture);
            }
            UnloadImage(skin);
        } else {
            invalidFilePopup = true;
        }
    }

    while (!WindowShouldClose())
    {
        if (IsFileDropped()) {
            FilePathList list = LoadDroppedFiles();
            if (IsFileExtension(list.paths[0], "png")) {
                Image skin = LoadImage(list.paths[0]);

                if (skin.width != 64 && skin.height != 64 && skin.height != 32) {
                    invalidSizePopup = true;
                } else {
                    UnloadTexture(custom.texture);
                    custom = MinecraftSkin::LoadSkinTexture(list.paths[0]);
                    SetWindowTitle(std::string("Skin viewer | " + std::string(list.paths[0])).c_str());
                    SetMaterialTexture(&classicModel.materials[1], MATERIAL_MAP_DIFFUSE, custom.texture);
                    SetMaterialTexture(&slimModel.materials[1], MATERIAL_MAP_DIFFUSE, custom.texture);
                }
                UnloadImage(skin);
            } else {
                std::cout << "Invalid file: " << list.paths[0] << std::endl;
                invalidFilePopup = true;
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
            UpdateCamera(&camera, CAMERA_FREE);

            if (IsKeyPressed(KEY_ESCAPE)) {
                EnableCursor();
            }
        }

        BeginDrawing();

        ClearBackground(DARKGRAY);
        BeginMode3D(camera);
        DrawModel(skinType == 1 ? slimModel : classicModel, position, 1.0f, WHITE);
        DrawGrid(10, 1.0f);
        EndMode3D();

        rlImGuiBegin();

        runImGui();

        rlImGuiEnd();

        EndDrawing();
    }
    UnloadModel(classicModel);
    UnloadModel(slimModel);
    UnloadTexture(steveTexture);
    UnloadTexture(alexTexture);
    UnloadTexture(custom.texture);
    rlImGuiShutdown();
    CloseWindow();

    return 0;
}