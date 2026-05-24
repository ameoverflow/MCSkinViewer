//
// Created by void on 24/05/2026.
//

#ifndef SKINVIEWER_RENDERER_H
#define SKINVIEWER_RENDERER_H
#include <string>

namespace Renderer {
    void RenderSkin(bool renderGrid = false);
    bool RenderToFile(const std::string& filename, int width, int height);
}

#endif //SKINVIEWER_RENDERER_H
