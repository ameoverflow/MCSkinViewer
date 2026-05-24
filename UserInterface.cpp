//
// Created by void on 20/05/2026.
//

#include "UserInterface.h"

#include <cstring>
#include <raylib.h>
#include "imgui.h"
#include "imgui_internal.h"
#include "State.h"
#include "rlimgui/rlImGui.h"
#include "portable-file-dialogs.h"
#include "Renderer.h"

bool invalidFilePopup, invalidTexturePopup, renderedPopup, couldntSaveImagePopup, invalidPathPopup;
bool aboutPopup, minecraftSkinPopup, urlPopup, pngPopup, saveFilePopup;
bool showCameraWindow = true;
bool showTextureWindow = true;
bool showMeshWindow = true;
bool showRenderWindow = true;

char saveFilePath[256] = "";
bool openFileOnSave;

char renderFilePath[256] = "";
int renderResolution[2] = {1920, 1080};

void UserInterface::ShowInvalidFilePopup() {
    invalidFilePopup = true;
}

void UserInterface::ShowInvalidTexturePopup() {
    invalidTexturePopup = true;
}

void RunMinecraftSkinPopup() {
    if (minecraftSkinPopup) {
        ImGui::OpenPopup("Open from Minecraft");
        minecraftSkinPopup = false;
    }

    if (ImGui::BeginPopupModal("Open from Minecraft", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        bool inputSubmitted = ImGui::InputText(" Minecraft username", State.username, sizeof(State.username), ImGuiInputTextFlags_EnterReturnsTrue);

        if (ImGui::Button("Get", ImVec2(64, 0)) || inputSubmitted) {
            MinecraftSkin::LoadSkinFromMinecraft(std::string(State.username));
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(64, 0))) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

void RunURLSkinPopup() {
    if (urlPopup) {
        ImGui::OpenPopup("Open from URL");
        urlPopup = false;
    }

    if (ImGui::BeginPopupModal("Open from URL", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        bool inputSubmitted = ImGui::InputText(" URL", State.url, sizeof(State.url), ImGuiInputTextFlags_EnterReturnsTrue);

        if (ImGui::Button("Get", ImVec2(64, 0)) || inputSubmitted) {
            MinecraftSkin::LoadSkinFromURL(std::string(State.url));
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(64, 0))) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

void RunFileBrowser() {
    if (pngPopup) {
        ImGui::OpenPopup("Open from PNG file");
        pngPopup = false;
    }

    if (ImGui::BeginPopupModal("Open from PNG file", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        bool inputSubmitted = ImGui::InputText(" Path", State.skinPath, sizeof(State.skinPath), ImGuiInputTextFlags_EnterReturnsTrue);
        ImGui::SameLine();
        if (pfd::settings::available() && ImGui::Button("...", ImVec2(32, 0))) {
            auto f = pfd::open_file("Select a skin texture", pfd::path::home(),
                            { "PNG images (*.png)", "*.png" });

            if (!f.result().empty()) {
                std::memset(State.skinPath, 0, sizeof(State.skinPath));
                std::strncpy(State.skinPath, f.result()[0].c_str(), sizeof(State.skinPath) - 1);
                State.skinPath[sizeof(State.skinPath) - 1] = '\0';
            }
        }

        if (ImGui::Button("Open", ImVec2(64, 0)) || inputSubmitted) {
            if (IsFileNameValid(renderFilePath)) {
                if (!MinecraftSkin::LoadSkinFromPNG(std::string(State.skinPath))) {
                    UserInterface::ShowInvalidTexturePopup();
                }
                ImGui::CloseCurrentPopup();
            } else {
                invalidPathPopup = true;
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(64, 0))) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

void RunSavePopup() {
    if (saveFilePopup) {
        std::memset(saveFilePath, 0, sizeof(saveFilePath));
        ImGui::OpenPopup("Save skin");
        saveFilePopup = false;
    }

    if (ImGui::BeginPopupModal("Save skin", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        bool inputSubmitted = ImGui::InputText(" Path", saveFilePath, sizeof(saveFilePath), ImGuiInputTextFlags_EnterReturnsTrue);
        ImGui::SameLine();
        if (pfd::settings::available() && ImGui::Button("...", ImVec2(32, 0))) {
            auto f = pfd::save_file("Select save location", pfd::path::home(),
                            { "PNG images (*.png)", "*.png" });

            if (!f.result().empty()) {
                std::strncpy(saveFilePath, f.result().c_str(), sizeof(saveFilePath) - 1);
                saveFilePath[sizeof(saveFilePath) - 1] = '\0';
            }
        }

        ImGui::Checkbox("Open file when saved", &openFileOnSave);

        if (ImGui::Button("Save", ImVec2(64, 0)) || inputSubmitted) {
            Image output = LoadImageFromTexture(State.loadedSkin.original);
            if (ExportImage(output, saveFilePath)) {
                if (openFileOnSave) {
                    if (!MinecraftSkin::LoadSkinFromPNG(std::string(saveFilePath))) {
                        UserInterface::ShowInvalidTexturePopup();
                    }
                }
            } else {
                invalidPathPopup = true;
            }
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(64, 0))) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

void RunAboutPopup() {
    if (aboutPopup) {
        ImGui::OpenPopup("About");
        aboutPopup = false;
    }

    if (ImGui::BeginPopupModal("About", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        std::string aboutText = "Version " + std::string(VERSION);
        aboutText += "\n(c) 2026 ameOverflow, licensed under GNU GPL v3 License.\n";;
        aboutText += "Built on " + std::string(__DATE__) + " " + std::string(__TIME__);
        aboutText += "\nNot an official Minecraft project, not associated with Mojang or Microsoft.";
        ImGui::Text(aboutText.c_str());
        if (ImGui::Button("OK", ImVec2(64, 0))) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

void RunTextureWindow() {
    if (showTextureWindow) {
        ImGui::Begin("Texture", &showTextureWindow);

        ImVec2 availableSpace = ImGui::GetContentRegionAvail();

        Texture2D displaySkin;

        if (State.loadedSkin.original.id == 0) {
            displaySkin = State.isSlim ? State.alexSkin.original : State.steveSkin.original;
        } else {
            displaySkin = State.loadedSkin.original;
        }

        if (availableSpace.x > 0 && availableSpace.y > 0) {
            float texWidth = displaySkin.width;
            float texHeight = displaySkin.height;

            float textureAspect = texWidth / texHeight;
            float windowAspect = availableSpace.x / availableSpace.y;

            float targetWidth = 0.0f;
            float targetHeight = 0.0f;

            if (textureAspect > windowAspect) {
                targetWidth = availableSpace.x;
                targetHeight = targetWidth / textureAspect;
            } else {
                targetHeight = availableSpace.y;
                targetWidth = targetHeight * textureAspect;
            }
            rlImGuiImageSize(&displaySkin, targetWidth, targetHeight);
        }

        ImGui::End();
    }
}

void RunCameraWindow() {
    if (showCameraWindow) {
        ImGui::Begin("Environment properties", &showCameraWindow, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize);

        ImGui::Text("Camera coordinates: ");
        ImGui::Text("%f %f %f", State.camera.position.x, State.camera.position.y, State.camera.position.z);

        ImGui::Text("Camera target: ");
        ImGui::Text("%f %f %f", State.camera.target.x, State.camera.target.y, State.camera.target.z);

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        if (ImGui::SliderFloat("FOV", &State.cameraFov, 30.0f, 90.0f, "%.0f°")) {
            State.camera.fovy = State.cameraFov;
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::Checkbox("Draw grid under model", &State.enableGrid);

        ImGui::Text("Viewport background color:");
        ImGui::ColorPicker3("", State.backgroundColor, ImGuiColorEditFlags_DisplayHex | ImGuiColorEditFlags_DisplayRGB);
        if (ImGui::Button("Reset color to default")) {
            State.backgroundColor[0] = 0.1f;
            State.backgroundColor[1] = 0.1f;
            State.backgroundColor[2] = 0.1f;
        }

        ImGui::End();
    }
}

void RunModelPropertiesWindow() {
    if (showMeshWindow) {
        ImGui::Begin("Model properties", &showMeshWindow, ImGuiWindowFlags_NoResize);

        ImGui::Checkbox("Use slim model", &State.isSlim);

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::SetNextItemOpen(true, ImGuiCond_Once);
        if (ImGui::TreeNode("Model")) {
            if (ImGui::TreeNode("Head")) {
                ImGui::Checkbox("Head", &State.enabledMeshes[0]);
                ImGui::Checkbox("Hat", &State.enabledMeshes[1]);
                ImGui::TreePop();
            }
            if (ImGui::TreeNode("Body")) {
                ImGui::Checkbox("Body", &State.enabledMeshes[2]);
                if (!State.loadedSkin.isOldType) ImGui::Checkbox("Body Layer", &State.enabledMeshes[3]);
                ImGui::TreePop();
            }
            if (ImGui::TreeNode("Right Arm")) {
                ImGui::Checkbox("Right Arm", &State.enabledMeshes[4]);
                if (!State.loadedSkin.isOldType) ImGui::Checkbox("Right Arm Layer", &State.enabledMeshes[5]);
                ImGui::TreePop();
            }
            if (ImGui::TreeNode("Left Arm")) {
                ImGui::Checkbox("Left Arm", &State.enabledMeshes[6]);
                if (!State.loadedSkin.isOldType) ImGui::Checkbox("Left Arm Layer", &State.enabledMeshes[7]);
                ImGui::TreePop();
            }
            if (ImGui::TreeNode("Right Leg")) {
                ImGui::Checkbox("Right Leg", &State.enabledMeshes[8]);
                if (!State.loadedSkin.isOldType) ImGui::Checkbox("Right Leg Layer", &State.enabledMeshes[9]);
                ImGui::TreePop();
            }
            if (ImGui::TreeNode("Left Leg")) {
                ImGui::Checkbox("Left Leg", &State.enabledMeshes[10]);
                if (!State.loadedSkin.isOldType) ImGui::Checkbox("Left Leg Layer", &State.enabledMeshes[11]);
                ImGui::TreePop();
            }
            ImGui::TreePop();
        }

        ImGui::End();
    }
}

void RunRenderWindow() {
    if (showRenderWindow) {
        ImGui::Begin("Render", &showRenderWindow, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize);

        ImGui::Text("Render resolution:");
        ImGui::InputInt2("", renderResolution, 0);

        ImGui::InputText(" Path", renderFilePath, sizeof(renderFilePath));
        if (pfd::settings::available() && ImGui::Button("...", ImVec2(32, 0))) {
            auto f = pfd::save_file("Select save location", pfd::path::home(),
                            { "PNG images (*.png)", "*.png" });

            if (!f.result().empty()) {
                std::memset(renderFilePath, 0, sizeof(renderFilePath));
                std::strncpy(renderFilePath, f.result().c_str(), sizeof(renderFilePath) - 1);
                renderFilePath[sizeof(renderFilePath) - 1] = '\0';
            }
        }

        if (ImGui::Button("Render", ImVec2(64, 0))) {
            if (Renderer::RenderToFile(renderFilePath, renderResolution[0], renderResolution[1])) {
                renderedPopup = true;
            } else {
                couldntSaveImagePopup = true;
            }
        }

        ImGui::End();
    }
}

void UserInterface::RunUI() {
    if (invalidFilePopup) {
        ImGui::OpenPopup("Invalid file");
        invalidFilePopup = false;
    }

    if (invalidTexturePopup) {
        ImGui::OpenPopup("Invalid texture");
        invalidTexturePopup = false;
    }

    if (couldntSaveImagePopup) {
        ImGui::OpenPopup("Could not save image");
        couldntSaveImagePopup = false;
    }

    if (invalidPathPopup) {
        ImGui::OpenPopup("Invalid path");
        invalidPathPopup = false;
    }

    if (renderedPopup) {
        ImGui::OpenPopup("Rendered successfully");
        renderedPopup = false;
    }

    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Open from PNG...", "Ctrl+O")) pngPopup = true;
            if (ImGui::MenuItem("Open from URL...", "Ctrl+U")) urlPopup = true;
            if (ImGui::MenuItem("Open from Minecraft...", "Ctrl+M")) minecraftSkinPopup = true;
            ImGui::Separator();
            if (ImGui::MenuItem("Save...", "Ctrl+S", false, State.loadedSkin.source != SkinSource_Unknown && State.loadedSkin.source != SkinSource_File)) saveFilePopup = true;
            ImGui::Separator();
            ImGui::Checkbox("Hot reload", &State.hotReloadEnabled);
            ImGui::Separator();
            if (ImGui::MenuItem("Exit", "Alt+F4")) State.quit = true;

            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("View")) {
            if (ImGui::MenuItem("Camera and environment properties...", "Ctrl+1")) showCameraWindow = true;
            if (ImGui::MenuItem("Model properties...", "Ctrl+2")) showMeshWindow = true;
            if (ImGui::MenuItem("View texture...", "Ctrl+3")) showTextureWindow = true;
            if (ImGui::MenuItem("View render options...", "Ctrl+4")) showRenderWindow = true;
            ImGui::EndMenu();
        }
        if (ImGui::MenuItem("Reload (Ctrl+R)")) State.needsReload = true;
        if (ImGui::MenuItem("About...")) aboutPopup = true;

        ImGui::EndMainMenuBar();
    }

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(6.0f, 4.0f));

    if (ImGui::BeginViewportSideBar("##MainStatusBar", ImGui::GetMainViewport(), ImGuiDir_Down, ImGui::GetFrameHeight(), ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoSavedSettings)) {
        ImGui::Text("FPS: %d", GetFPS());

        ImGui::SameLine();
        ImGui::Text("|");
        ImGui::SameLine();

        ImGui::Text("Window size: %i, %i", GetRenderWidth(), GetRenderHeight());

        ImGui::SameLine();
        ImGui::Text("|");
        ImGui::SameLine();

        ImGui::Text("Model type: %s", State.isSlim ? "Slim" : "Classic");

        ImGui::SameLine();
        ImGui::Text("|");
        ImGui::SameLine();

        ImGui::Text("Skin type: %s", State.loadedSkin.isOldType ? "Pre-1.8" : "1.8+");

        ImGui::SameLine();
        ImGui::Text("|");
        ImGui::SameLine();

        ImGui::Text("Skin source: %s", State.loadedSkin.source == SkinSource_Minecraft ? "Minecraft"
            : State.loadedSkin.source == SkinSource_File ? "Local file"
            : State.loadedSkin.source == SkinSource_URL ? "URL" : "Default");

        ImGui::SameLine();
        ImGui::Text("(%s)", GetFileName(State.loadedSkin.path.c_str()));

        if (IsCursorHidden()) {
            ImGui::SameLine();
            ImGui::Text("|");
            ImGui::SameLine();

            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "-- CURSOR LOCKED (ESC to unlock) --");
        }

        ImGui::End();
    }

    ImGui::PopStyleVar();
    ImGui::PopStyleColor();

    // check some hotkeys
    if (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)) {
        if (IsKeyPressed(KEY_O)) {
            auto f = pfd::open_file("Select a skin texture", pfd::path::home(),
                            { "PNG images (*.png)", "*.png" });

            if (!f.result().empty()) {
                std::memset(State.skinPath, 0, sizeof(State.skinPath));
                std::strncpy(State.skinPath, f.result()[0].c_str(), sizeof(State.skinPath) - 1);
                State.skinPath[sizeof(State.skinPath) - 1] = '\0';
                if (!MinecraftSkin::LoadSkinFromPNG(std::string(State.skinPath))) {
                    ShowInvalidTexturePopup();
                }
            }
        }

        if (IsKeyPressed(KEY_M)) minecraftSkinPopup = true;
        if (IsKeyPressed(KEY_U)) urlPopup = true;
        if (IsKeyPressed(KEY_R)) State.needsReload = true;

        if (IsKeyPressed(KEY_ONE)) showCameraWindow = !showCameraWindow;
        if (IsKeyPressed(KEY_TWO)) showMeshWindow = !showMeshWindow;
        if (IsKeyPressed(KEY_THREE)) showTextureWindow = !showTextureWindow;
        if (IsKeyPressed(KEY_FOUR)) showRenderWindow = !showRenderWindow;
    }

    RunMinecraftSkinPopup();
    RunURLSkinPopup();
    RunAboutPopup();
    RunCameraWindow();
    RunTextureWindow();
    RunFileBrowser();
    RunModelPropertiesWindow();
    RunSavePopup();
    RunRenderWindow();

    if (ImGui::BeginPopupModal("Invalid file", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Provided file is invalid. It needs to be a .png file.");
        if (ImGui::Button("OK", ImVec2(64, 0))) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    if (ImGui::BeginPopupModal("Invalid texture", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Texture must be 64x64 or 64x32.");
        if (ImGui::Button("OK", ImVec2(64, 0))) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    if (ImGui::BeginPopupModal("Could not save image", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Image could not be exported. Check the save path.");
        if (ImGui::Button("OK", ImVec2(64, 0))) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    if (ImGui::BeginPopupModal("Invalid path", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Invalid file path, double check the path.");
        if (ImGui::Button("OK", ImVec2(64, 0))) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    if (ImGui::BeginPopupModal("Rendered successfully", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Image was rendered successfully.");
        if (ImGui::Button("OK", ImVec2(64, 0))) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}