//
// Created by void on 24/05/2026.
//

#include "Renderer.h"

#include <raylib.h>
#include <raymath.h>

#include "State.h"

RenderTexture renderOutput;

void Renderer::RenderSkin(bool renderGrid) {
    ClearBackground({State.backgroundColor[0] * 255.0f, State.backgroundColor[1] * 255.0f, State.backgroundColor[2] * 255.0f, 255});
    BeginMode3D(State.camera);

    // dinnerbone easter egg
    Matrix transform = MatrixScale(1, 1, 1);
    Vector3 position = { 0.0f, 0.0f, 0.0f };
    if (State.loadedSkin.source == SkinSource_Minecraft && State.loadedSkin.path == "Dinnerbone") {
        BoundingBox box = GetModelBoundingBox(State.isSlim ? State.slimModel : State.classicModel);
        float height = box.max.y - box.min.y;
        transform = MatrixRotateX(PI);
        transform = MatrixRotateZ(PI);
        position.y = height;
    } else {
        position.y = 0;
    }

    // draw the skin
    transform = MatrixMultiply(transform, MatrixTranslate(position.x, position.y, position.z));
    for (int i = 0; i < 12; i++) {
        if (!State.enabledMeshes[i]) continue;
        int matIndex = (State.isSlim ? State.slimModel : State.classicModel).meshMaterial[i];
        DrawMesh((State.isSlim ? State.slimModel : State.classicModel).meshes[i],  (State.isSlim ? State.slimModel : State.classicModel).materials[matIndex], transform);
    }

    if (renderGrid) DrawGrid(10, 1.0f);

    EndMode3D();
}

bool Renderer::RenderToFile(const std::string &filename, int width, int height) {
    renderOutput = LoadRenderTexture(width, height);
    BeginTextureMode(renderOutput);
    RenderSkin(false);
    EndTextureMode();

    Image outputImage = LoadImageFromTexture(renderOutput.texture);
    ImageFlipVertical(&outputImage);
    UnloadRenderTexture(renderOutput);

    bool returnVal = ExportImage(outputImage, filename.c_str());
    UnloadImage(outputImage);
    return returnVal;
}
