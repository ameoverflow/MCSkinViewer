#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "MinecraftSkin.h"
#include "httplib.h"
#include "json.hpp"
#include "base64.hpp"

Texture2D MinecraftSkin::LoadSkinTexture(const std::string& filePath) {
    Image img = LoadImage(filePath.c_str());

    if (img.width != 64) {
        UnloadImage(img);
        return {};
    }

    Texture2D returnTexture;
    if (img.height == 32) {
        returnTexture = ConvertImage(img);
        UnloadImage(img);
    } else if (img.height == 64) {
        returnTexture = LoadTextureFromImage(img);
        UnloadImage(img);
    }

    return returnTexture;
}

Texture2D MinecraftSkin::ConvertImage(Image img) {
    Image converted = GenImageColor(64, 64, BLANK);

    ImageDraw(&converted, img, {0, 0, 64, 32}, {0, 0, 64, 32}, WHITE);

    // fun for your entire family, right arm and leg are flipped
    // extract the 16x12 right arm chunk
    Image leftArmChunk = ImageFromImage(img, { 40, 20, 16, 12});
    // flip it horizontally so the shoulders/hands face the right direction
    ImageFlipHorizontal(&leftArmChunk);
    // stamp it into the modern left arm coordinates
    ImageDraw(&converted, leftArmChunk, {0, 0, 16, 12}, {32, 52, 16, 12}, WHITE);
    UnloadImage(leftArmChunk); // clean up temp chunk

    Image leftArmTop = ImageFromImage(img, {44, 16, 4, 4});
    ImageFlipHorizontal(&leftArmTop);
    ImageDraw(&converted, leftArmTop, {0, 0, 4, 4}, {36, 48, 4, 4}, WHITE);
    UnloadImage(leftArmTop);

    Image leftArmBottom = ImageFromImage(img, {48, 16, 4, 4});
    ImageFlipHorizontal(&leftArmBottom);
    ImageDraw(&converted, leftArmBottom, {0, 0, 4, 4}, {40, 48, 4, 4}, WHITE);
    UnloadImage(leftArmBottom);

    Image leftLegChunk = ImageFromImage(img, { 0, 20, 16, 12});
    ImageFlipHorizontal(&leftLegChunk);
    ImageDraw(&converted, leftLegChunk, {0, 0, 16, 16}, {16, 52, 16, 12}, WHITE);
    UnloadImage(leftLegChunk);

    Image leftLegTop = ImageFromImage(img, {4, 16, 4, 4});
    ImageFlipHorizontal(&leftLegTop);
    ImageDraw(&converted, leftLegTop, {0, 0, 4, 4}, {20, 48, 4, 4}, WHITE);
    UnloadImage(leftLegTop);

    Image leftLegBottom = ImageFromImage(img, {8, 16, 4, 4});
    ImageFlipHorizontal(&leftLegBottom);
    ImageDraw(&converted, leftLegBottom, {0, 0, 4, 4}, {24, 48, 4, 4}, WHITE);
    UnloadImage(leftLegBottom);

    return LoadTextureFromImage(converted);
}

Texture2D MinecraftSkin::LoadSkinFromMinecraft(const std::string& username) {
    httplib::Client minecraftApi("https://api.minecraftservices.com");

    nlohmann::json buffer;
    std::string uuid, skin, skinUrl;
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
        skin = properties["value"].get<std::string>();
    } else {
        return {};
    }

    std::string skinJson = base64::from_base64(skin);

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
            UnloadImage(skinImage);
        } else if (skinImage.height == 64) {
            returnTexture = LoadTextureFromImage(skinImage);
            UnloadImage(skinImage);
        }

        return returnTexture;
    }
    return {};
}
