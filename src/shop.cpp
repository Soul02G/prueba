#include "map.h"
#include "shop.h"
#include "raylib.h"
#include <stdio.h>

// --- Texturas base (sprite por defecto) ---
static Texture2D texPlayer;
static Texture2D texArrow;
static Texture2D texMonkey;
static Texture2D texCoco;
static Texture2D texTotem;
static Texture2D texBat;

// --- Texturas de las skins alternativas ---
static Texture2D texPlayerSkin; // totmS.png

// --- Estado de la tienda ---
static int selectedItem = 0;
static const int TOTAL_ITEMS = 6;
static const char* itemNames[6] = { "SKIN JUGADOR", "FLECHA", "MONO", "COCO", "TIRAFLECHAS", "MURCIELAGO." };

// skinOption controla qué estamos VIENDO en pantalla (0 = base, 1 = especial)
static int skinOption[TOTAL_ITEMS] = { 0, 0, 0, 0, 0, 0 };

// Coste de cada skin alternativa (0 = no tiene skin comprable)
static const int skinCost[TOTAL_ITEMS] = { 15, 0, 0, 0, 0, 0 };

// ---------------------------------------------------------------

void ShopLoad(MapState* mapState) {
    texPlayer = LoadTexture("resources/totm.png");
    texArrow = LoadTexture("resources/flecha.png");
    texMonkey = LoadTexture("resources/MonkeyFrames_0.png");
    texCoco = LoadTexture("resources/coco.png");
    texTotem = LoadTexture("resources/tiraflechas.png");
    texBat = LoadTexture("resources/bat-frame-1.png");
    texPlayerSkin = LoadTexture("resources/totmS.png");
    selectedItem = 0;

    // Inicializar la visualización con lo que el jugador ya tenga equipado
    skinOption[0] = mapState->playerSkinIndex;
}

SceneType ShopUpdate(MapState* mapState) {
    if (mapState->settingsOpen) {
        if (IsKeyPressed(KEY_M)) mapState->settingsOpen = 0;
        if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) mapState->settingsOption = 0;
        if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) mapState->settingsOption = 1;

        if (mapState->settingsOption == 0) {
            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) ||
                IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_LEFT)) {
                mapState->musicEnabled = !mapState->musicEnabled;
            }
        }
        else if (mapState->settingsOption == 1) {
            if (IsKeyDown(KEY_LEFT))  mapState->masterVolume -= 0.01f;
            if (IsKeyDown(KEY_RIGHT)) mapState->masterVolume += 0.01f;
            if (mapState->masterVolume < 0.0f) mapState->masterVolume = 0.0f;
            if (mapState->masterVolume > 1.0f) mapState->masterVolume = 1.0f;
        }
        SetMasterVolume(mapState->musicEnabled ? mapState->masterVolume : 0.0f);
        return SCENE_SHOP;
    }

    if (IsKeyPressed(KEY_M)) mapState->settingsOpen = 1;
    if (IsKeyPressed(KEY_TAB)) return SCENE_MAP;

    // Navegar entre articulos con izquierda/derecha
    if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) {
        selectedItem++;
        if (selectedItem >= TOTAL_ITEMS) selectedItem = 0;
        // Al cambiar de ítem, mostramos por defecto lo que esté configurado en el mapa
        if (selectedItem == 0) skinOption[0] = mapState->playerSkinIndex;
        else skinOption[selectedItem] = 0;
    }
    if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A)) {
        selectedItem--;
        if (selectedItem < 0) selectedItem = TOTAL_ITEMS - 1;
        if (selectedItem == 0) skinOption[0] = mapState->playerSkinIndex;
        else skinOption[selectedItem] = 0;
    }

    // --- Logica de Previsualización, Compra y Equipamiento ---
    if (skinCost[selectedItem] > 0) {
        int cost = skinCost[selectedItem];

        // 1. PERMITIR PREVISUALIZAR (Cambiar de skin en pantalla con ARRIBA/ABAJO libremente)
        if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) {
            skinOption[selectedItem] = 1; // Muestra la skin especial
            // Si ya está comprada, la equipa inmediatamente en el juego
            if (mapState->playerSkinBought) {
                mapState->playerSkinIndex = 1;
            }
        }
        if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) {
            skinOption[selectedItem] = 0; // Muestra la skin base
            if (mapState->playerSkinBought) {
                mapState->playerSkinIndex = 0;
            }
        }

        // 2. COMPRAR CON ESPACIO (Solo si estás previsualizando la skin 1 y no la tienes)
        if (IsKeyPressed(KEY_SPACE)) {
            if (skinOption[selectedItem] == 1 && !mapState->playerSkinBought) {
                if (mapState->totalCoins >= cost) {
                    mapState->totalCoins -= cost;
                    mapState->playerSkinBought = true;
                    mapState->playerSkinIndex = 1; // Queda equipada tras comprar
                    printf("Skin comprada! Monedas restantes: %d\n", mapState->totalCoins);
                }
            }
        }
    }

    return SCENE_SHOP;
}

// ---------------------------------------------------------------

static void DrawSettingsOverlay(const MapState* mapState, int screenWidth, int screenHeight) {
    DrawRectangle(0, 0, screenWidth, screenHeight, ColorAlpha(BLACK, 0.7f));
    int panelW = 400, panelH = 280;
    int px = (screenWidth - panelW) / 2, py = (screenHeight - panelH) / 2;

    DrawRectangle(px, py, panelW, panelH, Color{ 255, 220, 0, 255 });
    DrawRectangleLines(px, py, panelW, panelH, Color{ 80, 60, 0, 255 });
    DrawText("AJUSTES", px + (panelW - MeasureText("AJUSTES", 28)) / 2, py + 20, 28, Color{ 30, 20, 0, 255 });

    int sfxY = py + 75;
    if (mapState->settingsOption == 0) DrawRectangle(px + 40, sfxY - 5, panelW - 80, 40, Color{ 255, 240, 100, 255 });
    DrawText(mapState->musicEnabled ? "SFX: ON" : "SFX: OFF",
        px + (panelW - MeasureText("SFX: ON", 22)) / 2, sfxY + 6, 22, BLACK);

    int volLabelY = py + 135;
    if (mapState->settingsOption == 1) DrawRectangle(px + 40, volLabelY - 5, panelW - 80, 100, Color{ 255, 240, 100, 255 });

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

// ---------------------------------------------------------------

void ShopDraw(const MapState* mapState, int screenWidth, int screenHeight) {
    ClearBackground(BLACK);

    // --- Barra superior ---
    DrawRectangle(0, 0, screenWidth, 50, Color{ 30, 25, 0, 255 });
    DrawCircle(screenWidth - 130, 25, 14, Color{ 255, 220, 0, 255 });
    DrawCircleLines(screenWidth - 130, 25, 14, Color{ 80, 60, 0, 255 });
    DrawText(TextFormat("%d", mapState->totalCoins), screenWidth - 110, 17, 18, YELLOW);

    DrawRectangle(screenWidth - 44, 8, 36, 36, Color{ 255, 220, 0, 255 });
    DrawRectangleLines(screenWidth - 44, 8, 36, 36, Color{ 80, 60, 0, 255 });
    DrawText("*", screenWidth - 34, 13, 24, Color{ 30, 20, 0, 255 });

    DrawText("TIENDA", (screenWidth - MeasureText("TIENDA", 32)) / 2, 65, 32, YELLOW);

    // --- Caja central ---
    int boxW = 200, boxH = 200;
    int bx = (screenWidth - boxW) / 2;
    int by = (screenHeight - boxH) / 2 - 60;

    DrawRectangle(bx, by, boxW, boxH, Color{ 30, 30, 30, 255 });
    DrawRectangleLines(bx, by, boxW, boxH, YELLOW);

    // Elegir textura a mostrar (Basado en skinOption que cambia dinámicamente)
    Texture2D currentTex = { 0 };
    if (selectedItem == 0) {
        currentTex = (skinOption[0] == 1) ? texPlayerSkin : texPlayer;
    }
    else {
        switch (selectedItem) {
        case 1: currentTex = texArrow;  break;
        case 2: currentTex = texMonkey; break;
        case 3: currentTex = texCoco;   break;
        case 4: currentTex = texTotem;  break;
        case 5: currentTex = texBat;    break;
        }
    }

    if (currentTex.id > 0) {
        int padding = 16;
        DrawTexturePro(currentTex,
            { 0.0f, 0.0f, (float)currentTex.width, (float)currentTex.height },
            { (float)(bx + padding), (float)(by + padding),
              (float)(boxW - padding * 2), (float)(boxH - padding * 2) },
            { 0.0f, 0.0f }, 0.0f, WHITE);
    }

    // --- Nombre del articulo ---
    const char* itemName = itemNames[selectedItem];
    DrawText(itemName, (screenWidth - MeasureText(itemName, 24)) / 2, by + boxH + 20, 24, WHITE);

    // --- Indicadores de skin ---
    if (skinCost[selectedItem] > 0) {
        int cost = skinCost[selectedItem];

        Color upColor = (skinOption[selectedItem] == 1) ? YELLOW : Color{ 150, 150, 150, 255 };
        Color downColor = (skinOption[selectedItem] == 0) ? YELLOW : Color{ 150, 150, 150, 255 };
        DrawText("^", bx + boxW / 2 - 8, by - 36, 28, upColor);

        int flechaAbajoY = by + boxH + 68;
        DrawText("v", bx + boxW / 2 - 8, flechaAbajoY, 28, downColor);

        // Estado visual actual del contenedor
        const char* skinLabel = (skinOption[selectedItem] == 0) ? "VISTA: SKIN BASE" : "VISTA: SKIN ESPECIAL";
        int skinLabelY = flechaAbajoY + 32;
        DrawText(skinLabel, (screenWidth - MeasureText(skinLabel, 16)) / 2, skinLabelY, 16, YELLOW);

        // Mensaje dinámico de compra o equipamiento
        int priceLabelY = skinLabelY + 28;
        if (!mapState->playerSkinBought) {
            if (skinOption[selectedItem] == 1) {
                // Si está viendo la skin bloqueada
                const char* priceLabel = TextFormat("PRECIO: %d monedas [ESPACIO] comprar", cost);
                Color priceColor = (mapState->totalCoins >= cost) ? Color{ 255, 220, 0, 255 } : Color{ 200, 80, 80, 255 };
                DrawText(priceLabel, (screenWidth - MeasureText(priceLabel, 14)) / 2, priceLabelY, 14, priceColor);

                if (mapState->totalCoins < cost) {
                    const char* noFunds = "Monedas insuficientes";
                    DrawText(noFunds, (screenWidth - MeasureText(noFunds, 13)) / 2, priceLabelY + 22, 13, Color{ 200, 80, 80, 255 });
                }
            }
            else {
                // Si está viendo su skin base normal pero el artículo es vendible
                const char* previewHint = "[ARRIBA] para previsualizar skin especial";
                DrawText(previewHint, (screenWidth - MeasureText(previewHint, 14)) / 2, priceLabelY, 14, LIGHTGRAY);
            }
        }
        else {
            // Ya es dueño: Indica si está equipada o no
            bool isActive = (skinOption[selectedItem] == mapState->playerSkinIndex);
            const char* owned = isActive ? "EQUIPADA [ARRIBA/ABAJO] para cambiar" : "COMPRADA [ARRIBA/ABAJO] para equipar";
            Color ownedColor = isActive ? Color{ 100, 255, 100, 255 } : Color{ 150, 220, 150, 255 };
            DrawText(owned, (screenWidth - MeasureText(owned, 14)) / 2, priceLabelY, 14, ownedColor);
        }
    }

    // --- Flechas laterales de navegacion ---
    DrawText("<", bx - 40, by + (boxH - 32) / 2, 32, WHITE);
    DrawText(">", bx + boxW + 25, by + (boxH - 32) / 2, 32, WHITE);

    // --- Hint inferior ---
    const char* shopHint = "FLECHAS: cambiar articulo    M: ajustes    TAB: volver al mapa";
    DrawText(shopHint, (screenWidth - MeasureText(shopHint, 12)) / 2, screenHeight - 32, 12, Color{ 200, 170, 0, 255 });

    if (mapState->settingsOpen) DrawSettingsOverlay(mapState, screenWidth, screenHeight);
}

void ShopUnload(void) {
    UnloadTexture(texPlayer);
    UnloadTexture(texPlayerSkin);
    UnloadTexture(texArrow);
    UnloadTexture(texMonkey);
    UnloadTexture(texCoco);
    UnloadTexture(texTotem);
    UnloadTexture(texBat);
}