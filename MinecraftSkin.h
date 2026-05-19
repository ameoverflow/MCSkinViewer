#ifndef SKINVIEWER_MINECRAFTSKIN_H
#define SKINVIEWER_MINECRAFTSKIN_H
#include <raylib.h>
#include <string>

struct Skin {
    bool isOldType;
    Texture2D texture;
};

namespace MinecraftSkin {
    Skin LoadSkinTexture(const std::string& path);
    Texture2D ConvertImage(Image img);
    Skin LoadSkinFromMinecraft(const std::string& username);
}

#endif //SKINVIEWER_MINECRAFTSKIN_H
