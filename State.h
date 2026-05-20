//
// Created by void on 20/05/2026.
//

#ifndef SKINVIEWER_STATE_H
#define SKINVIEWER_STATE_H
#include <raylib.h>

#include "MinecraftSkin.h"

struct AppState {
    bool isSlim = false;
    char username[17] = "";
    char url[256] = "";
    char skinPath[256] = "";
    Skin loadedSkin;
    Camera camera;
    Model classicModel, slimModel;
    Skin steve, alex;
    bool quit;

    bool enabledMeshes[12] = {true, true, true, true, true, true, true, true, true, true, true, true};
};

inline AppState State;

#endif //SKINVIEWER_STATE_H
