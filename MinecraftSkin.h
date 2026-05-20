#ifndef SKINVIEWER_MINECRAFTSKIN_H
#define SKINVIEWER_MINECRAFTSKIN_H
#include <raylib.h>
#include <string>

enum SkinSource {
    SkinSource_Unknown,
    SkinSource_Minecraft,
    SkinSource_File,
    SkinSource_URL
};

struct Skin {
    SkinSource source;
    std::string path;
    bool isOldType;
    Texture2D texture;
    Texture2D original;
};

namespace MinecraftSkin {
    Skin LoadSkinIntoStruct(Image img);
    bool LoadSkinFromPNG(const std::string& path);
    Texture2D ConvertImage(Image img);
    bool LoadSkinFromMinecraft(const std::string& username);
    bool LoadSkinFromURL(const std::string& url);
}

#endif //SKINVIEWER_MINECRAFTSKIN_H
