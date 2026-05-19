#ifndef SKINVIEWER_MINECRAFTSKIN_H
#define SKINVIEWER_MINECRAFTSKIN_H
#include <raylib.h>
#include <string>

namespace MinecraftSkin {
    Texture2D LoadSkinTexture(const std::string& path);
    Texture2D ConvertImage(Image img);
    Texture2D LoadSkinFromMinecraft(const std::string& username);
}

#endif //SKINVIEWER_MINECRAFTSKIN_H
