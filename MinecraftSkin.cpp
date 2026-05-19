#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "MinecraftSkin.h"
#include "httplib.h"
#include "json.hpp"
#include "base64.hpp"

Skin MinecraftSkin::LoadSkinTexture(const std::string& filePath) {
    Image img = LoadImage(filePath.c_str());

    Skin skin;

    if (img.width != 64) {
        UnloadImage(img);
        return {};
    }

    Texture2D returnTexture;
    if (img.height == 32) {
        returnTexture = ConvertImage(img);
        skin.isOldType = true;
        UnloadImage(img);
    } else if (img.height == 64) {
        skin.isOldType = false;
        returnTexture = LoadTextureFromImage(img);
        UnloadImage(img);
    }
    skin.texture = returnTexture;

    return skin;
}

void FlipSkinPart(Image originalImage, Image targetImage, Rectangle sourceRect, Rectangle targetRect) {
    Image chunk = ImageFromImage(originalImage, sourceRect);
    ImageFlipHorizontal(&chunk);
    ImageDraw(&targetImage, chunk, {0, 0, sourceRect.width, sourceRect.height}, targetRect, WHITE);
    UnloadImage(chunk);
}

Texture2D MinecraftSkin::ConvertImage(Image img) {
    Image converted = GenImageColor(64, 64, BLANK);

    ImageDraw(&converted, img, {0, 0, 64, 32}, {0, 0, 64, 32}, WHITE);

    // right arm
    FlipSkinPart(img, converted, {40, 20, 4, 12}, {40, 52, 4, 12});
    FlipSkinPart(img, converted, {44, 20, 4, 12}, {36, 52, 4, 12});
    FlipSkinPart(img, converted, {48, 20, 4, 12}, {32, 52, 4, 12});
    FlipSkinPart(img, converted,  {52, 20, 4, 12}, {44, 52, 4, 12});

    FlipSkinPart(img, converted, {0, 20, 4, 12}, {24, 52, 4, 12});
    FlipSkinPart(img, converted, {4, 20, 4, 12}, {20, 52, 4, 12});
    FlipSkinPart(img, converted, {8, 20, 4, 12}, {16, 52, 4, 12});
    FlipSkinPart(img, converted,  {12, 20, 4, 12}, {28, 52, 4, 12});

    FlipSkinPart(img, converted, {44, 16, 4, 4}, {36, 48, 4, 4});
    FlipSkinPart(img, converted, {48, 16, 4, 4}, {40, 48, 4, 4});
    FlipSkinPart(img, converted, {4, 16, 4, 4}, {20, 48, 4, 4});
    FlipSkinPart(img, converted, {8, 16, 4, 4}, {24, 48, 4, 4});

    return LoadTextureFromImage(converted);
}

Skin MinecraftSkin::LoadSkinFromMinecraft(const std::string& username) {
    Skin skin;
    httplib::Client minecraftApi("https://api.minecraftservices.com");

    nlohmann::json buffer;
    std::string uuid, skinBase64, skinUrl;
    std::string skinPath;

    std::string url = "/minecraft/profile/lookup/name/" + username;
    if (httplib::Result result = minecraftApi.Get(url)) {
        if (result->status == 404 || result->body.size() == 0) return {};

        if (!nlohmann::json::accept(result->body)) return {};

        buffer = nlohmann::json::parse(result->body);

        if (!buffer.contains("id") && !buffer["id"].is_string()) return {};
        uuid = buffer["id"].get<std::string>();
    } else {
        return {};
    }

    minecraftApi = httplib::Client("https://sessionserver.mojang.com");

    if (httplib::Result result = minecraftApi.Get("/session/minecraft/profile/" + uuid)) {
        if (result->status == 204 || result->body.size() == 0) return {};

        if (!nlohmann::json::accept(result->body)) return {};

        buffer = nlohmann::json::parse(result->body);

        if (!buffer.contains("properties") || !buffer["properties"].is_array()) return {};
        nlohmann::json properties = buffer["properties"][0];

        if (!properties.contains("value") || !properties["value"].is_string()) return {};
        skinBase64 = properties["value"].get<std::string>();
    } else {
        return {};
    }

    std::string skinJson = base64::from_base64(skinBase64);

    if (!nlohmann::json::accept(skinJson)) return {};

    buffer = nlohmann::json::parse(skinJson);

    if (!buffer.contains("textures") || !buffer["textures"].is_object()) return {};
    nlohmann::json textures = buffer["textures"];

    if (!textures.contains("SKIN") || !textures["SKIN"].is_object()) return {};
    nlohmann::json skinObject = textures["SKIN"];

    if (!skinObject.contains("url") || !skinObject["url"].is_string()) return {};
    skinUrl = skinObject["url"].get<std::string>();

    size_t protocolEnd = skinUrl.find("://");
    size_t hostStart = (protocolEnd == std::string::npos) ? 0 : protocolEnd + 3;

    size_t pathStart = skinUrl.find("/", hostStart);

    if (pathStart == std::string::npos)
    {
        skinPath = "/";
    }
    else
    {
        skinPath = skinUrl.substr(pathStart);
    }

    httplib::Client minecraftTextures("http://textures.minecraft.net");

    if (httplib::Result result = minecraftTextures.Get(skinPath)) {
        if (result->status == 204 || result->body.size() == 0) return {};

        Image skinImage = LoadImageFromMemory(".png", (const unsigned char*)result->body.data(), (int)result->body.size());

        if (skinImage.width != 64) {
            UnloadImage(skinImage);
            return {};
        }

        Texture2D returnTexture;
        if (skinImage.height == 32) {
            returnTexture = ConvertImage(skinImage);
            skin.isOldType = true;
            UnloadImage(skinImage);
        } else if (skinImage.height == 64) {
            skin.isOldType = false;
            returnTexture = LoadTextureFromImage(skinImage);
            UnloadImage(skinImage);
        }
        skin.texture = returnTexture;

        return skin;
    }
    return {};
}
