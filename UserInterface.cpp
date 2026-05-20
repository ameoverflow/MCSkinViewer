//
// Created by void on 20/05/2026.
//

#include "UserInterface.h"

#include <cstring>
#include <raylib.h>
#include "imgui.h"
#include "State.h"
#include "rlimgui/rlImGui.h"
#include "portable-file-dialogs.h"

bool invalidFilePopup, invalidTexturePopup;
bool aboutPopup, minecraftSkinPopup, urlPopup, pngPopup;
bool showCameraWindow, showTextureWindow;

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

            if (f.result().size() > 0) {
                std::memset(State.skinPath, 0, sizeof(State.skinPath));
                std::strncpy(State.skinPath, f.result()[0].c_str(), sizeof(State.skinPath) - 1);
                State.skinPath[sizeof(State.skinPath) - 1] = '\0';
            }
        }

        if (ImGui::Button("Open", ImVec2(64, 0)) || inputSubmitted) {
            if (!MinecraftSkin::LoadSkinFromPNG(std::string(State.skinPath))) {
                UserInterface::ShowInvalidTexturePopup();
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
        ImGui::Text("(c) 2026 ameOverflow, licensed under MIT License.\nNot an official Minecraft project, not associated with Mojang or Microsoft.");
        if (ImGui::Button("OK", ImVec2(64, 0))) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

void RunTextureWindow() {
    if (showTextureWindow) {
        ImGui::Begin("Currently loaded skin", &showTextureWindow);

        ImVec2 availableSpace = ImGui::GetContentRegionAvail();

        Texture2D displaySkin;

        if (State.loadedSkin.original.id == 0) {
            displaySkin = State.isSlim ? State.alex.original : State.steve.original;
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
        ImGui::Begin("Camera properties", &showCameraWindow, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize);

        ImGui::Text("Camera coordinates: ");
        ImGui::Text("%f %f %f", State.camera.position.x, State.camera.position.y, State.camera.position.z);

        ImGui::Text("Camera target: ");
        ImGui::Text("%f %f %f", State.camera.target.x, State.camera.target.y, State.camera.target.z);

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

    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Open from PNG...")) pngPopup = true;
            if (ImGui::MenuItem("Open from URL...")) urlPopup = true;
            if (ImGui::MenuItem("Open from Minecraft...")) minecraftSkinPopup = true;
            ImGui::Separator();
            if (ImGui::MenuItem("Exit", "Alt+F4")) State.quit = true;

            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("View")) {
            if (ImGui::MenuItem("Camera properties...")) showCameraWindow = true;
            if (ImGui::MenuItem("View currently loaded skin...")) showTextureWindow = true;
            ImGui::Separator();
            if (ImGui::MenuItem("Classic skin type (4px arms)")) State.isSlim = false;
            if (ImGui::MenuItem("Slim skin type (3px arms)")) State.isSlim = true;
            ImGui::EndMenu();
        }
        if (ImGui::MenuItem("About...")) aboutPopup = true;

        std::string statusText = "Model: ";
        statusText += State.isSlim ? "Slim" : "Classic";
        statusText += ", Skin type: ";
        statusText += State.loadedSkin.isOldType ? "Pre-1.8" : "1.8+";

        float padding = 20.0f;
        float statusPosX;
        if (IsCursorHidden()) {
           statusPosX = ImGui::GetWindowWidth() - ImGui::CalcTextSize(statusText.c_str()).x - ImGui::CalcTextSize(" -- CURSOR LOCKED (ESC to unlock) --").x - padding;
        } else {
            statusPosX = ImGui::GetWindowWidth() - ImGui::CalcTextSize(statusText.c_str()).x - padding;
        }

        ImGui::SameLine(statusPosX);
        ImGui::TextDisabled("%s", statusText.c_str());

        if (IsCursorHidden()) {
            float cursorLockedPosX = ImGui::GetWindowWidth() - ImGui::CalcTextSize(" -- CURSOR LOCKED (ESC to unlock) --").x - padding;
            ImGui::SameLine(cursorLockedPosX);
            ImGui::TextColored(ImVec4(1, 0, 0, 1), " -- CURSOR LOCKED (ESC to unlock) --");
        }

        ImGui::EndMainMenuBar();
    }

    RunMinecraftSkinPopup();
    RunURLSkinPopup();
    RunAboutPopup();
    RunCameraWindow();
    RunTextureWindow();
    RunFileBrowser();

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
}