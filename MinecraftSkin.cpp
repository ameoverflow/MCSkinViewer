#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "MinecraftSkin.h"
#include "httplib.h"
#include "json.hpp"
#include "base64.hpp"
#include "State.h"
#include "UserInterface.h"

Skin MinecraftSkin::LoadSkinIntoStruct(Image img) {
    Skin skin;

    if (img.width != 64) {
        return {};
    }

    Texture2D returnTexture;
    skin.original = LoadTextureFromImage(img);
    if (img.height == 32) {
        returnTexture = ConvertImage(img);
        skin.isOldType = true;
    } else if (img.height == 64) {
        skin.isOldType = false;
        returnTexture = LoadTextureFromImage(img);
    } else {
        return {};
    }
    skin.texture = returnTexture;
    return skin;
}

bool MinecraftSkin::LoadSkinFromPNG(const std::string& filePath) {
    Image img = LoadImage(filePath.c_str());

    if (img.width != 64 || (img.height != 64 && img.height != 32)) {
        UserInterface::ShowInvalidTexturePopup();
        UnloadImage(img);
        return false;
    }

    Skin skin = LoadSkinIntoStruct(img);
    UnloadImage(img);

    if (skin.texture.id == 0) return false;

    UnloadTexture(State.loadedSkin.texture);

    SetWindowTitle(std::string("Skin viewer | " + std::string(GetFileName(filePath.c_str()))).c_str());
    SetMaterialTexture(&State.classicModel.materials[1], MATERIAL_MAP_DIFFUSE, skin.texture);
    SetMaterialTexture(&State.slimModel.materials[1], MATERIAL_MAP_DIFFUSE, skin.texture);
    State.loadedSkin = skin;

    return true;
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

bool MinecraftSkin::LoadSkinFromMinecraft(const std::string& username) {
    httplib::Client minecraftApi("https://api.minecraftservices.com");

    nlohmann::json buffer;
    std::string uuid, skinBase64, skinUrl;
    std::string skinPath;

    std::string url = "/minecraft/profile/lookup/name/" + username;
    if (httplib::Result result = minecraftApi.Get(url)) {
        if (result->status == 404 || result->body.size() == 0) return {};

        if (!nlohmann::json::accept(result->body)) return {};

        buffer = nlohmann::json::parse(result->body);

        if (!buffer.contains("id") && !buffer["id"].is_string()) return false;
        uuid = buffer["id"].get<std::string>();
    } else {
        return false;
    }

    minecraftApi = httplib::Client("https://sessionserver.mojang.com");

    if (httplib::Result result = minecraftApi.Get("/session/minecraft/profile/" + uuid)) {
        if (result->status == 204 || result->body.size() == 0) return false;

        if (!nlohmann::json::accept(result->body)) return false;

        buffer = nlohmann::json::parse(result->body);

        if (!buffer.contains("properties") || !buffer["properties"].is_array()) return false;
        nlohmann::json properties = buffer["properties"][0];

        if (!properties.contains("value") || !properties["value"].is_string()) return false;
        skinBase64 = properties["value"].get<std::string>();
    } else {
        return false;
    }

    std::string skinJson = base64::from_base64(skinBase64);

    if (!nlohmann::json::accept(skinJson)) return false;

    buffer = nlohmann::json::parse(skinJson);

    if (!buffer.contains("textures") || !buffer["textures"].is_object()) return false;
    nlohmann::json textures = buffer["textures"];

    if (!textures.contains("SKIN") || !textures["SKIN"].is_object()) return false;
    nlohmann::json skinObject = textures["SKIN"];

    if (!skinObject.contains("url") || !skinObject["url"].is_string()) return false;

    bool loadResult = LoadSkinFromURL(skinObject["url"].get<std::string>());
    if (!loadResult) return false;
    SetWindowTitle(std::string("Skin viewer | " + username).c_str());

    return true;
}

bool MinecraftSkin::LoadSkinFromURL(const std::string& url) {
    std::string host, path;

    size_t protocolEnd = url.find("://");
    size_t hostStart = (protocolEnd == std::string::npos) ? 0 : protocolEnd + 3;

    size_t pathStart = url.find("/", hostStart);

    if (pathStart == std::string::npos)
    {
        host = url;
        path = "/";
    }
    else
    {
        host = url.substr(0, pathStart);
        path = url.substr(pathStart);
    }

    httplib::Client skinSource(host);

    if (httplib::Result result = skinSource.Get(path)) {
        if (result->status == 204 || result->body.size() == 0) return false;

        std::string contentType = result->get_header_value("Content-Type");
        std::transform(contentType.begin(), contentType.end(), contentType.begin(), ::tolower);
        if (contentType.find("image/png") == std::string::npos) {
            UserInterface::ShowInvalidFilePopup();
            return false;
        }

        Image skinImage = LoadImageFromMemory(".png", (const unsigned char*)result->body.data(), (int)result->body.size());

        if (skinImage.width != 64 || (skinImage.height != 64 && skinImage.height != 32)) {
            UserInterface::ShowInvalidTexturePopup();
            UnloadImage(skinImage);
            return false;
        }

        Skin skin = LoadSkinIntoStruct(skinImage);
        UnloadImage(skinImage);

        if (skin.texture.id == 0) return false;
        UnloadTexture(State.loadedSkin.texture);

        SetWindowTitle(std::string("Skin viewer | " + url).c_str());
        SetMaterialTexture(&State.classicModel.materials[1], MATERIAL_MAP_DIFFUSE, skin.texture);
        SetMaterialTexture(&State.slimModel.materials[1], MATERIAL_MAP_DIFFUSE, skin.texture);
        State.loadedSkin = skin;
        return true;
    }

    return false;
}
