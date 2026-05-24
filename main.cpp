#include <filesystem>
#include "Assets.h"
#include "base64.hpp"
#include "FileSystem.h"
#include "raylib.h"
#include "raymath.h"
#include "imgui/imgui.h"
#include "rlimgui/rlImGui.h"
#include "MinecraftSkin.h"
#include "State.h"
#include "UserInterface.h"
#include "imgui_internal.h"
#include "Renderer.h"

RenderTexture skinRender;
Color gridColor = {128, 128, 128, 255};

RenderTexture viewportTarget;
ImVec2 mainWindowSize;

void SetupDefaultDockLayout(ImGuiID mainDockspaceId) {
    ImGui::DockBuilderRemoveNode(mainDockspaceId);
    ImGui::DockBuilderAddNode(mainDockspaceId, ImGuiDockNodeFlags_PassthruCentralNode | ImGuiDockNodeFlags_DockSpace);

    ImGui::DockBuilderSetNodeSize(mainDockspaceId, ImGui::GetMainViewport()->WorkSize);

    ImGuiID dockIdLeft;
    ImGuiID dockIdRemaining;
    ImGuiID dockIdRight;
    ImGuiID dockIdBottomRight;

    ImGui::DockBuilderSplitNode(mainDockspaceId, ImGuiDir_Left, 0.17f, &dockIdLeft, &dockIdRemaining);
    ImGui::DockBuilderSplitNode(dockIdRemaining, ImGuiDir_Right, 0.20f, &dockIdRight, &dockIdRemaining);
    ImGui::DockBuilderSplitNode(dockIdRight, ImGuiDir_Down, 0.35f, &dockIdBottomRight, &dockIdRight);

    ImGui::DockBuilderDockWindow("Model properties", dockIdLeft);
    ImGui::DockBuilderDockWindow("Texture", dockIdBottomRight);
    ImGui::DockBuilderDockWindow("Environment properties", dockIdRight);
    ImGui::DockBuilderDockWindow("Render", dockIdBottomRight);

    ImGui::DockBuilderDockWindow("3D viewport", dockIdRemaining);

    ImGui::DockBuilderFinish(mainDockspaceId);
}

int main(int argc, char* argv[])
{
    InitWindow(1600, 900, "Skin viewer | Default skin");
    SetWindowState(FLAG_VSYNC_HINT | FLAG_WINDOW_RESIZABLE);
    SetExitKey(0);

    rlImGuiSetup(true);

    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    State.camera = { 0 };
    State.camera.position = (Vector3){ 3.0f, 3.0f, -3.0f };
    State.camera.target = (Vector3){ 0.0f, 1.0f, 0.0f };
    State.camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    State.camera.fovy = 45.0f;
    State.camera.projection = CAMERA_PERSPECTIVE;

    // unpack models to memory, raylib doesnt support loading models from a string in case of .gltf files
    std::string playerClassicPath = FileSystem::GetSystemTempPath() + "player_classic.gltf";
    std::string playerSlimPath = FileSystem::GetSystemTempPath() + "player_slim.gltf";

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

    skinRender = LoadRenderTexture(640, 640);
    viewportTarget = LoadRenderTexture(1920, 1080);

    State.steveSkin = MinecraftSkin::LoadSkinIntoStruct(steveSkin);
    State.alexSkin = MinecraftSkin::LoadSkinIntoStruct(alexSkin);
    SetMaterialTexture(&State.classicModel.materials[1], MATERIAL_MAP_DIFFUSE, skinRender.texture);
    SetMaterialTexture(&State.slimModel.materials[1], MATERIAL_MAP_DIFFUSE, skinRender.texture);

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
        if (State.hotReloadEnabled && State.needsReload && State.reloadCooldownTimer <= 0.0f) {
            TraceLog(LOG_INFO, "Skin file modified on disk! Need reload");

            switch (State.loadedSkin.source) {
                case SkinSource_File:
                    MinecraftSkin::LoadSkinFromPNG(std::string(State.loadedSkin.path));
                    break;
                case SkinSource_Minecraft:
                    MinecraftSkin::LoadSkinFromMinecraft(State.loadedSkin.path);
                    break;
                case SkinSource_URL:
                    MinecraftSkin::LoadSkinFromURL(State.loadedSkin.path);
                    break;
            }

            State.needsReload = false;
            State.reloadCooldownTimer = 0.1f;
        }

        if (State.reloadCooldownTimer > 0.0f) State.reloadCooldownTimer -= GetFrameTime();

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

        if (IsCursorHidden()) {
            UpdateCamera(&State.camera, CAMERA_FREE);

            if (IsKeyPressed(KEY_ESCAPE)) {
                EnableCursor();
            }
        }

        BeginTextureMode(skinRender);
        ClearBackground(BLANK);

        if (State.loadedSkin.texture.id == 0) {
            DrawTexturePro(State.isSlim ? State.alexSkin.texture : State.steveSkin.texture, {0, 0, 64, -64}, {0, 0, 640, 640}, {0, 0}, 0, WHITE);
        } else {
            DrawTexturePro(State.loadedSkin.texture, {0, 0, 64, -64}, {0, 0, 640, 640}, {0, 0}, 0, WHITE);
        }

        EndTextureMode();

        if (mainWindowSize.x < 1) mainWindowSize.x = 1;
        if (mainWindowSize.y < 1) mainWindowSize.y = 1;

        if ((int)mainWindowSize.x != viewportTarget.texture.width ||
            (int)mainWindowSize.y != viewportTarget.texture.height)
        {
            UnloadRenderTexture(viewportTarget);
            viewportTarget = LoadRenderTexture((int)mainWindowSize.x, (int)mainWindowSize.y);
        }

        BeginTextureMode(viewportTarget);

        Renderer::RenderSkin(State.enableGrid);

        EndTextureMode();

        BeginDrawing();
        rlImGuiBegin();

        ImGuiID mainDockspaceId = ImGui::DockSpaceOverViewport();

        static bool isLayoutInitialized = false;
        if (!isLayoutInitialized) {
            SetupDefaultDockLayout(mainDockspaceId);
            isLayoutInitialized = true;
        }

        UserInterface::RunUI();

        if (ImGui::Begin("3D viewport")) {
            mainWindowSize = ImGui::GetContentRegionAvail();

            Rectangle sourceRec = { 0.0f, 0.0f, (float)viewportTarget.texture.width, -(float)viewportTarget.texture.height };
            rlImGuiImageRect(&viewportTarget.texture, (int)mainWindowSize.x, (int)mainWindowSize.y, sourceRec);

            if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
                ImGui::SetWindowFocus();
                DisableCursor();
            }

            ImGui::End();
        }

        rlImGuiEnd();
        EndDrawing();
    }
    UnloadModel(State.classicModel);
    UnloadModel(State.slimModel);
    UnloadTexture(State.steveSkin.texture);
    UnloadTexture(State.alexSkin.texture);
    UnloadTexture(State.loadedSkin.texture);
    rlImGuiShutdown();
    CloseWindow();

    return 0;
}