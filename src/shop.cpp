#include "map.h"
#include "shop.h"
#include "raylib.h"
#include <stdio.h>

static Texture2D texPlayer;
static Texture2D texArrow;
static Texture2D texMonkey[11];
static Texture2D texCoco;
static Texture2D texTotem;
static Texture2D texBat;

static Texture2D texPlayerSkin;
static Texture2D texBatSkin;
static Texture2D texCocoSkin;
static Texture2D texFlechaSkin;
static Texture2D texTotemSkin;
static Texture2D texTotemSkinS;
static Texture2D texMonkeySkin[11];

static const int TOTAL_ITEMS = 6;
static int  selectedItem = 0;

static const char* itemNames[TOTAL_ITEMS] = {
    "SKIN JUGADOR",
    "MURCIELAGO",
    "FLECHA",
    "COCO",
    "TIRAFLECHAS",
    "MONO"
};

static const int skinCost[TOTAL_ITEMS] = { 15, 10, 8, 8, 12, 12 };

static int skinOption[TOTAL_ITEMS] = { 0, 0, 0, 0, 0, 0 };

static int  shopMonkeyFrame = 0;
static int  shopMonkeyTimer = 0;
static const int SHOP_MONKEY_SPEED = 8;


void ShopLoad(MapState* mapState) {
    texPlayer = LoadTexture("resources/totm.png");
    texBat = LoadTexture("resources/bat-frame-1.png");
    texArrow = LoadTexture("resources/flecha.png");
    texCoco = LoadTexture("resources/coco.png");
    texTotem = LoadTexture("resources/tiraflechas.png");
    for (int i = 0; i < 11; i++)
        texMonkey[i] = LoadTexture(TextFormat("resources/MonkeyFrames_%d.png", i));

    texPlayerSkin = LoadTexture("resources/totmS.png");
    texBatSkin = LoadTexture("resources/batS-frame-1.png");
    texCocoSkin = LoadTexture("resources/cocoS.png");
    texFlechaSkin = LoadTexture("resources/flechaS.png");
    texTotemSkin = LoadTexture("resources/tiraflechasS.png");
    texTotemSkinS = LoadTexture("resources/tiraflechasSS.png");

    // Si un frame de skin no existe (fallback 1x1), duplicar el frame anterior
    for (int i = 0; i < 11; i++) {
        texMonkeySkin[i] = LoadTexture(TextFormat("resources/MonkeyFramesS_%d.png", i));
        if (texMonkeySkin[i].width <= 1 && i > 0)
            texMonkeySkin[i] = texMonkeySkin[i - 1];
    }

    selectedItem = 0;

    skinOption[0] = mapState->playerSkinIndex;
    skinOption[1] = mapState->batSkinIndex;
    skinOption[2] = mapState->flechaSkinIndex;
    skinOption[3] = mapState->cocoSkinIndex;
    skinOption[4] = mapState->tiraflechasSkinIndex;
    skinOption[5] = mapState->monkeySkinIndex;

    shopMonkeyFrame = 1;
    shopMonkeyTimer = 0;
}


static int MaxSkinOption(int item) {
    if (item == 4) return 2;
    return 1;
}

static const char* SkinOptionLabel(int item, int opt) {
    if (item == 4) {
        if (opt == 0) return "VISTA: SKIN BASE";
        if (opt == 1) return "VISTA: SKIN S";
        return "VISTA: SKIN SS";
    }
    return (opt == 0) ? "VISTA: SKIN BASE" : "VISTA: SKIN ESPECIAL";
}


static bool IsSkinOptionBought(int item, int opt, const MapState* ms) {
    if (opt == 0) return true;
    switch (item) {
    case 0: return ms->playerSkinBought;
    case 1: return ms->batSkinBought;
    case 2: return ms->flechaSkinBought;
    case 3: return ms->cocoSkinBought;
    case 4: return (opt == 1) ? ms->tiraflechasSkinBought : ms->tiraflechasSSkinBought;
    case 5: return ms->monkeySkinBought;
    }
    return false;
}


static int SkinOptionCost(int item, int /*opt*/) {
    return skinCost[item];
}

SceneType ShopUpdate(MapState* mapState) {

    shopMonkeyTimer++;
    if (shopMonkeyTimer >= SHOP_MONKEY_SPEED) {
        shopMonkeyTimer = 0;
        shopMonkeyFrame = shopMonkeyFrame + 1;
        if (shopMonkeyFrame >= 11) shopMonkeyFrame = 1;
    }

    if (mapState->settingsOpen) {
        if (IsKeyPressed(KEY_M)) mapState->settingsOpen = 0;
        if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) mapState->settingsOption = 0;
        if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) mapState->settingsOption = 1;

        if (mapState->settingsOption == 0) {
            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) ||
                IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_LEFT))
                mapState->musicEnabled = !mapState->musicEnabled;
        }
        else {
            if (IsKeyDown(KEY_LEFT))  mapState->masterVolume -= 0.01f;
            if (IsKeyDown(KEY_RIGHT)) mapState->masterVolume += 0.01f;
            if (mapState->masterVolume < 0.0f) mapState->masterVolume = 0.0f;
            if (mapState->masterVolume > 1.0f) mapState->masterVolume = 1.0f;
        }
        SetMasterVolume(mapState->musicEnabled ? mapState->masterVolume : 0.0f);
        return SCENE_SHOP;
    }

    if (IsKeyPressed(KEY_M))   mapState->settingsOpen = 1;
    if (IsKeyPressed(KEY_TAB)) return SCENE_MAP;

    if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) {
        selectedItem = (selectedItem + 1) % TOTAL_ITEMS;
        skinOption[0] = mapState->playerSkinIndex;
        skinOption[1] = mapState->batSkinIndex;
        skinOption[2] = mapState->flechaSkinIndex;
        skinOption[3] = mapState->cocoSkinIndex;
        skinOption[4] = mapState->tiraflechasSkinIndex;
        skinOption[5] = mapState->monkeySkinIndex;
    }
    if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A)) {
        selectedItem = (selectedItem - 1 + TOTAL_ITEMS) % TOTAL_ITEMS;
        skinOption[0] = mapState->playerSkinIndex;
        skinOption[1] = mapState->batSkinIndex;
        skinOption[2] = mapState->flechaSkinIndex;
        skinOption[3] = mapState->cocoSkinIndex;
        skinOption[4] = mapState->tiraflechasSkinIndex;
        skinOption[5] = mapState->monkeySkinIndex;
    }

    int maxOpt = MaxSkinOption(selectedItem);

    if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) {
        int newOpt = skinOption[selectedItem] + 1;
        if (newOpt > maxOpt) newOpt = maxOpt;
        skinOption[selectedItem] = newOpt;

        if (IsSkinOptionBought(selectedItem, newOpt, mapState)) {
            switch (selectedItem) {
            case 0: mapState->playerSkinIndex = newOpt; break;
            case 1: mapState->batSkinIndex = newOpt; break;
            case 2: mapState->flechaSkinIndex = newOpt; break;
            case 3: mapState->cocoSkinIndex = newOpt; break;
            case 4: mapState->tiraflechasSkinIndex = newOpt; break;
            case 5: mapState->monkeySkinIndex = newOpt; break;
            }
        }
    }
    if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) {
        int newOpt = skinOption[selectedItem] - 1;
        if (newOpt < 0) newOpt = 0;
        skinOption[selectedItem] = newOpt;

        if (IsSkinOptionBought(selectedItem, newOpt, mapState)) {
            switch (selectedItem) {
            case 0: mapState->playerSkinIndex = newOpt; break;
            case 1: mapState->batSkinIndex = newOpt; break;
            case 2: mapState->flechaSkinIndex = newOpt; break;
            case 3: mapState->cocoSkinIndex = newOpt; break;
            case 4: mapState->tiraflechasSkinIndex = newOpt; break;
            case 5: mapState->monkeySkinIndex = newOpt; break;
            }
        }
    }

    if (IsKeyPressed(KEY_SPACE)) {
        int opt = skinOption[selectedItem];
        int cost = SkinOptionCost(selectedItem, opt);
        if (opt > 0 && !IsSkinOptionBought(selectedItem, opt, mapState)) {
            if (mapState->totalCoins >= cost) {
                mapState->totalCoins -= cost;
                switch (selectedItem) {
                case 0: mapState->playerSkinBought = true;  mapState->playerSkinIndex = 1; break;
                case 1: mapState->batSkinBought = true;  mapState->batSkinIndex = 1; break;
                case 2: mapState->flechaSkinBought = true;  mapState->flechaSkinIndex = 1; break;
                case 3: mapState->cocoSkinBought = true;  mapState->cocoSkinIndex = 1; break;
                case 4:
                    if (opt == 1) { mapState->tiraflechasSkinBought = true; mapState->tiraflechasSkinIndex = 1; }
                    else { mapState->tiraflechasSSkinBought = true; mapState->tiraflechasSkinIndex = 2; }
                    break;
                case 5: mapState->monkeySkinBought = true;  mapState->monkeySkinIndex = 1; break;
                }
            }
        }
    }

    return SCENE_SHOP;
}

static void DrawSettingsOverlay(const MapState* mapState, int screenWidth, int screenHeight) {
    DrawRectangle(0, 0, screenWidth, screenHeight, ColorAlpha(BLACK, 0.7f));
    int panelW = 400, panelH = 280;
    int px = (screenWidth - panelW) / 2, py = (screenHeight - panelH) / 2;

    DrawRectangle(px, py, panelW, panelH, Color{ 255, 220, 0, 255 });
    DrawRectangleLines(px, py, panelW, panelH, Color{ 80, 60, 0, 255 });
    DrawText("AJUSTES", px + (panelW - MeasureText("AJUSTES", 28)) / 2, py + 20, 28, Color{ 30, 20, 0, 255 });

    int sfxY = py + 75;
    if (mapState->settingsOption == 0)
        DrawRectangle(px + 40, sfxY - 5, panelW - 80, 40, Color{ 255, 240, 100, 255 });
    DrawText(mapState->musicEnabled ? "SFX: ON" : "SFX: OFF",
        px + (panelW - MeasureText("SFX: ON", 22)) / 2, sfxY + 6, 22, BLACK);

    int volLabelY = py + 135;
    if (mapState->settingsOption == 1)
        DrawRectangle(px + 40, volLabelY - 5, panelW - 80, 100, Color{ 255, 240, 100, 255 });
    DrawText("VOLUMEN", px + (panelW - MeasureText("VOLUMEN", 20)) / 2, volLabelY, 20, Color{ 30, 20, 0, 255 });
    const char* volPct = TextFormat("%d%%", (int)(mapState->masterVolume * 100));
    DrawText(volPct, px + (panelW - MeasureText(volPct, 20)) / 2, volLabelY + 26, 20, Color{ 30, 20, 0, 255 });

    int sliderW = 280, sliderX = px + (panelW - sliderW) / 2;
    int sliderY = volLabelY + 58;
    DrawRectangle(sliderX, sliderY, sliderW, 14, Color{ 100, 80, 0, 255 });
    DrawRectangle(sliderX, sliderY, (int)(mapState->masterVolume * sliderW), 14, Color{ 200, 160, 0, 255 });
    int handleX = sliderX + (int)(mapState->masterVolume * (sliderW - 14));
    DrawRectangle(handleX, sliderY - 5, 14, 24, WHITE);
    DrawRectangleLines(handleX, sliderY - 5, 14, 24, Color{ 80, 60, 0, 255 });
}


void ShopDraw(const MapState* mapState, int screenWidth, int screenHeight) {
    ClearBackground(BLACK);

    DrawRectangle(0, 0, screenWidth, 50, Color{ 30, 25, 0, 255 });
    DrawCircle(screenWidth - 130, 25, 14, Color{ 255, 220, 0, 255 });
    DrawCircleLines(screenWidth - 130, 25, 14, Color{ 80, 60, 0, 255 });
    DrawText(TextFormat("%d", mapState->totalCoins), screenWidth - 110, 17, 18, YELLOW);
    DrawRectangle(screenWidth - 44, 8, 36, 36, Color{ 255, 220, 0, 255 });
    DrawRectangleLines(screenWidth - 44, 8, 36, 36, Color{ 80, 60, 0, 255 });
    DrawText("*", screenWidth - 34, 13, 24, Color{ 30, 20, 0, 255 });

    DrawText("TIENDA", (screenWidth - MeasureText("TIENDA", 32)) / 2, 65, 32, YELLOW);

    int boxW = 200, boxH = 200;
    int bx = (screenWidth - boxW) / 2;
    int by = (screenHeight - boxH) / 2 - 60;

    DrawRectangle(bx, by, boxW, boxH, Color{ 30, 30, 30, 255 });
    DrawRectangleLines(bx, by, boxW, boxH, YELLOW);

    int opt = skinOption[selectedItem];

    if (selectedItem == 5) {
        // MONO: animado — usar skin o base, ambos con fallback garantizado en carga
        Texture2D* frames = (opt == 1) ? texMonkeySkin : texMonkey;
        Texture2D& cur = frames[shopMonkeyFrame];
        if (cur.id > 0 && cur.width > 1) {
            int pad = 16;
            DrawTexturePro(cur,
                { 0.0f, 0.0f, (float)cur.width, (float)cur.height },
                { (float)(bx + pad), (float)(by + pad),
                  (float)(boxW - pad * 2), (float)(boxH - pad * 2) },
                { 0.0f, 0.0f }, 0.0f, WHITE);
        }
    }
    else {
        Texture2D currentTex = { 0 };
        switch (selectedItem) {
        case 0: currentTex = (opt == 1) ? texPlayerSkin : texPlayer;  break;
        case 1: currentTex = (opt == 1) ? texBatSkin : texBat;     break;
        case 2: currentTex = (opt == 1) ? texFlechaSkin : texArrow;   break;
        case 3: currentTex = (opt == 1) ? texCocoSkin : texCoco;    break;
        case 4:
            if (opt == 1) currentTex = texTotemSkin;
            else if (opt == 2) currentTex = texTotemSkinS;
            else               currentTex = texTotem;
            break;
        }
        if (currentTex.id > 0) {
            int pad = 16;
            DrawTexturePro(currentTex,
                { 0.0f, 0.0f, (float)currentTex.width, (float)currentTex.height },
                { (float)(bx + pad), (float)(by + pad),
                  (float)(boxW - pad * 2), (float)(boxH - pad * 2) },
                { 0.0f, 0.0f }, 0.0f, WHITE);
        }
    }

    const char* itemName = itemNames[selectedItem];
    DrawText(itemName, (screenWidth - MeasureText(itemName, 24)) / 2, by + boxH + 20, 24, WHITE);

    int maxOpt = MaxSkinOption(selectedItem);
    Color upColor = (opt < maxOpt) ? YELLOW : Color{ 80, 80, 80, 255 };
    Color downColor = (opt > 0) ? YELLOW : Color{ 80, 80, 80, 255 };
    DrawText("^", bx + boxW / 2 - 8, by - 36, 28, upColor);

    int flechaAbajoY = by + boxH + 68;
    DrawText("v", bx + boxW / 2 - 8, flechaAbajoY, 28, downColor);

    const char* skinLabel = SkinOptionLabel(selectedItem, opt);
    int skinLabelY = flechaAbajoY + 32;
    DrawText(skinLabel, (screenWidth - MeasureText(skinLabel, 16)) / 2, skinLabelY, 16, YELLOW);

    int infoY = skinLabelY + 28;

    if (opt == 0) {
        const char* baseMsg = "SKIN BASE (siempre disponible)";
        DrawText(baseMsg, (screenWidth - MeasureText(baseMsg, 14)) / 2, infoY, 14, LIGHTGRAY);
    }
    else {
        bool bought = IsSkinOptionBought(selectedItem, opt, mapState);
        int  cost = SkinOptionCost(selectedItem, opt);

        if (!bought) {
            const char* priceLabel = TextFormat("PRECIO: %d monedas  [ESPACIO] comprar", cost);
            Color priceColor = (mapState->totalCoins >= cost)
                ? Color{ 255, 220, 0, 255 }
            : Color{ 200, 80, 80, 255 };
            DrawText(priceLabel, (screenWidth - MeasureText(priceLabel, 14)) / 2, infoY, 14, priceColor);
            if (mapState->totalCoins < cost) {
                const char* noFunds = "Monedas insuficientes";
                DrawText(noFunds, (screenWidth - MeasureText(noFunds, 13)) / 2, infoY + 22, 13, Color{ 200, 80, 80, 255 });
            }
        }
        else {
            bool equipped = false;
            switch (selectedItem) {
            case 0: equipped = (mapState->playerSkinIndex == opt); break;
            case 1: equipped = (mapState->batSkinIndex == opt); break;
            case 2: equipped = (mapState->flechaSkinIndex == opt); break;
            case 3: equipped = (mapState->cocoSkinIndex == opt); break;
            case 4: equipped = (mapState->tiraflechasSkinIndex == opt); break;
            case 5: equipped = (mapState->monkeySkinIndex == opt); break;
            }
            const char* owned = equipped
                ? "EQUIPADA  [ARRIBA/ABAJO] para cambiar"
                : "COMPRADA  [ARRIBA/ABAJO] para equipar";
            Color ownedColor = equipped
                ? Color{ 100, 255, 100, 255 }
            : Color{ 150, 220, 150, 255 };
            DrawText(owned, (screenWidth - MeasureText(owned, 14)) / 2, infoY, 14, ownedColor);
        }
    }

    DrawText("<", bx - 40, by + (boxH - 32) / 2, 32, WHITE);
    DrawText(">", bx + boxW + 25, by + (boxH - 32) / 2, 32, WHITE);

    const char* shopHint = "FLECHAS IZQUIERDA/DERECHA: articulo    ARRIBA/ABAJO: skin    ESPACIO: comprar    M: ajustes    TAB: mapa";
    DrawText(shopHint, (screenWidth - MeasureText(shopHint, 11)) / 2, screenHeight - 32, 11, Color{ 200, 170, 0, 255 });

    if (mapState->settingsOpen)
        DrawSettingsOverlay(mapState, screenWidth, screenHeight);
}

void ShopUnload(void) {
    UnloadTexture(texPlayer);
    UnloadTexture(texPlayerSkin);
    UnloadTexture(texBat);
    UnloadTexture(texBatSkin);
    UnloadTexture(texArrow);
    UnloadTexture(texFlechaSkin);
    UnloadTexture(texCoco);
    UnloadTexture(texCocoSkin);
    UnloadTexture(texTotem);
    UnloadTexture(texTotemSkin);
    UnloadTexture(texTotemSkinS);
    for (int i = 0; i < 11; i++) {
        UnloadTexture(texMonkey[i]);
        UnloadTexture(texMonkeySkin[i]);
    }
}