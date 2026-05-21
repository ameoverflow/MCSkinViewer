//
// Created by void on 20/05/2026.
//

#ifndef SKINVIEWER_STATE_H
#define SKINVIEWER_STATE_H
#include <raylib.h>

#include "MinecraftSkin.h"
#include "FileWatch.hpp"

struct AppState {
    bool isSlim = false;
    char username[17] = "";
    char url[256] = "";
    char skinPath[256] = "";
    Skin loadedSkin;
    Camera camera;
    Model classicModel, slimModel;
    Skin steveSkin, alexSkin;
    bool quit;
    float backgroundColor[3] = {0.1f, 0.1f, 0.1f};
    float cameraFov = 45.0f;
    bool enableSkinGrid;
    bool enableGrid = true;
    std::unique_ptr<filewatch::FileWatch<std::string>> watch = nullptr;
    std::atomic<bool> needsReload = false;
    bool hotReloadEnabled = true;
    float reloadCooldownTimer = 0.0f;

    bool enabledMeshes[12] = {true, true, true, true, true, true, true, true, true, true, true, true};
};

inline AppState State;

#endif //SKINVIEWER_STATE_H
