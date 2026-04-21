#include "map.h"
#include "raylib.h"
#include <stdio.h>
#include <math.h>

#define SAVE_FILE_NAME "save.bin"

static const int LEVEL_GRID_COL[MAX_LEVELS] = { 0, 0, 1, 1, 2, 2 };
static const int LEVEL_GRID_ROW[MAX_LEVELS] = { 0, 1, 1, 0, 0, 1 };
static const int LEVEL_CONNECTIONS[5][2] = { {0, 1}, {1, 2}, {2, 3}, {3, 4}, {4, 5} };
static const int CONNECTION_COUNT = 5;

static Texture2D texStarCollected;
static Texture2D texStarEmpty;


static void DrawLevelNode(int levelIndex, int screenX, int screenY, const LevelProgress* progress, int isSelected, float pulse) {
    int nodeW = 110, nodeH = 110;
    int nx = screenX - nodeW / 2, ny = screenY - nodeH / 2;

    bool isLocked = (levelIndex > 1) || (levelIndex > 0 && !progress[levelIndex - 1].completed);

    DrawRectangle(nx + 4, ny + 4, nodeW, nodeH, Color{ 40, 30, 0, 200 });

    Color bgColor = isLocked ? Color{ 100, 90, 40, 255 } : (isSelected ? Color{ 255, 230, 0, 255 } : Color{ 210, 180, 0, 255 });

    DrawRectangle(nx, ny, nodeW, nodeH, bgColor);
    DrawRectangleLines(nx, ny, nodeW, nodeH, Color{ 80, 60, 0, 255 });

    if (isSelected && !isLocked) {
        float alpha = (sinf(pulse) + 1.0f) / 2.0f;
        DrawRectangleLines(nx - 3, ny - 3, nodeW + 6, nodeH + 6, Color{ 255, 255, 0, (unsigned char)(180 * alpha) });
    }

    // Estrellas
    int starSize = 28, starPad = 4;
    int starY = ny + nodeH - starSize - 6;

    // Número centrado verticalmente entre el top del nodo y la zona de estrellas
    int textAreaH = starY - ny;
    int textY = ny + (textAreaH - 36) / 2;
    const char* numText = TextFormat("%d", levelIndex + 1);
    DrawText(numText, nx + (nodeW - MeasureText(numText, 36)) / 2, textY, 36, isLocked ? DARKGRAY : Color{ 30, 20, 0, 255 });

    for (int s = 0; s < 3; s++) {
        int sx = nx + (nodeW - (3 * starSize + 2 * starPad)) / 2 + s * (starSize + starPad);
        Texture2D tex = (s < progress[levelIndex].starsEarned) ? texStarCollected : texStarEmpty;
        if (tex.id > 0) {
            DrawTexturePro(tex, { 0, 0, (float)tex.width, (float)tex.height }, { (float)sx, (float)starY, (float)starSize, (float)starSize }, { 0, 0 }, 0.0f, isLocked ? GRAY : WHITE);
        }
    }
}

static void DrawSettings(const MapState* mapState, int screenWidth, int screenHeight) {
    DrawRectangle(0, 0, screenWidth, screenHeight, ColorAlpha(BLACK, 0.7f));
    int panelW = 400, panelH = 250;
    int px = (screenWidth - panelW) / 2, py = (screenHeight - panelH) / 2;

    DrawRectangle(px, py, panelW, panelH, Color{ 255, 220, 0, 255 });
    DrawRectangleLines(px, py, panelW, panelH, Color{ 80, 60, 0, 255 });
    DrawText("AJUSTES", px + (panelW - MeasureText("AJUSTES", 28)) / 2, py + 20, 28, Color{ 30, 20, 0, 255 });

    int sfxY = py + 85;
    if (mapState->settingsOption == 0) DrawRectangle(px + 40, sfxY - 5, panelW - 80, 45, Color{ 255, 240, 100, 255 });
    DrawText(mapState->musicEnabled ? "SFX: ON" : "SFX: OFF", px + (panelW - MeasureText("SFX: ON", 22)) / 2, sfxY + 6, 22, BLACK);

    int sliderW = 200, sliderX = px + (panelW - sliderW) / 2;
    int sliderY = py + 150 + 25;
    DrawRectangle(sliderX, sliderY, sliderW, 20, Color{ 100, 80, 0, 255 });
    DrawRectangle(sliderX, sliderY, (int)(mapState->masterVolume * sliderW), 20, Color{ 200, 160, 0, 255 });
    DrawRectangle(sliderX + (int)(mapState->masterVolume * (sliderW - 15)), sliderY - 5, 15, 30, WHITE);
}


void SaveGameProgress(const MapState* mapState) {
    SaveFileData(SAVE_FILE_NAME, (void*)mapState, sizeof(MapState));
}

void LoadGameProgress(MapState* mapState) {
    if (FileExists(SAVE_FILE_NAME)) {
        int bytesRead = 0;
        unsigned char* data = LoadFileData(SAVE_FILE_NAME, &bytesRead);

        if (data != NULL && bytesRead == sizeof(MapState)) {
            *mapState = *(MapState*)data;
            UnloadFileData(data);
            return;
        }
    }

    for (int i = 0; i < MAX_LEVELS; i++) {
        mapState->levels[i].starsEarned = 0;
        mapState->levels[i].completed = 0;
    }
    mapState->totalCoins = 0;
}


void MapLoad(MapState* mapState) {
    LoadGameProgress(mapState);

    mapState->selectedLevel = 0;
    mapState->selectPulse = 0.0f;
    mapState->settingsOpen = 0;
    mapState->settingsOption = 0;
    mapState->musicEnabled = true;
    mapState->masterVolume = 0.5f;

    SetMasterVolume(mapState->masterVolume);
    texStarCollected = LoadTexture("resources/star_collected.png");
    texStarEmpty = LoadTexture("resources/star_empty.png");
}

SceneType MapUpdate(MapState* mapState) {
    mapState->selectPulse += GetFrameTime() * 3.0f;

    if (mapState->settingsOpen) {
        if (IsKeyPressed(KEY_M)) mapState->settingsOpen = 0;
        if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) mapState->settingsOption = 0;
        if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) mapState->settingsOption = 1;

        if (mapState->settingsOption == 0) {
            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_LEFT)) {
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
        return SCENE_MAP;
    }

    int col = LEVEL_GRID_COL[mapState->selectedLevel];
    int row = LEVEL_GRID_ROW[mapState->selectedLevel];
    int nextLevel = -1;

    if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) {
        for (int i = 0; i < MAX_LEVELS; i++)
            if (LEVEL_GRID_COL[i] == col + 1 && LEVEL_GRID_ROW[i] == row) { nextLevel = i; break; }
    }
    if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A)) {
        for (int i = 0; i < MAX_LEVELS; i++)
            if (LEVEL_GRID_COL[i] == col - 1 && LEVEL_GRID_ROW[i] == row) { nextLevel = i; break; }
    }
    if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) {
        for (int i = 0; i < MAX_LEVELS; i++)
            if (LEVEL_GRID_COL[i] == col && LEVEL_GRID_ROW[i] == row - 1) { nextLevel = i; break; }
    }
    if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) {
        for (int i = 0; i < MAX_LEVELS; i++)
            if (LEVEL_GRID_COL[i] == col && LEVEL_GRID_ROW[i] == row + 1) { nextLevel = i; break; }
    }

    if (nextLevel != -1 && nextLevel < MAX_LEVELS) {
        if (nextLevel == 0 || mapState->levels[nextLevel - 1].completed) {
            mapState->selectedLevel = nextLevel;
        }
    }

    if (IsKeyPressed(KEY_M)) mapState->settingsOpen = 1;
    if (IsKeyPressed(KEY_ESCAPE)) return SCENE_MENU;
    if (IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_ENTER)) {
        if (mapState->selectedLevel <= 1) return SCENE_GAME;
    }

    return SCENE_MAP;
}

void MapRegisterLevelComplete(MapState* mapState, int levelIndex, int starsEarned, int coinsEarned) {
    if (levelIndex < 0 || levelIndex >= MAX_LEVELS) return;

    mapState->levels[levelIndex].completed = 1;
    if (starsEarned > mapState->levels[levelIndex].starsEarned)
        mapState->levels[levelIndex].starsEarned = starsEarned;

    SaveGameProgress(mapState);
}

void MapAddCoins(MapState* mapState, int amount) {
    if (mapState == NULL || amount <= 0) return;
    mapState->totalCoins += amount;
    TraceLog(LOG_INFO, "[PERSISTENCIA] Monedas recibidas: %d. Total: %d", amount, mapState->totalCoins);
    SaveGameProgress(mapState);
}

void MapDraw(const MapState* mapState, int screenWidth, int screenHeight) {
    if (mapState == NULL) return;
    ClearBackground(BLACK);

    // HEADER
    DrawRectangle(0, 0, screenWidth, 50, Color{ 30, 25, 0, 255 });
    DrawCircle(screenWidth - 130, 25, 14, Color{ 255, 220, 0, 255 });
    DrawCircleLines(screenWidth - 130, 25, 14, Color{ 80, 60, 0, 255 });
    DrawText(TextFormat("%d", mapState->totalCoins), screenWidth - 110, 17, 18, YELLOW);

    DrawRectangle(screenWidth - 44, 8, 36, 36, Color{ 255, 220, 0, 255 });
    DrawRectangleLines(screenWidth - 44, 8, 36, 36, Color{ 80, 60, 0, 255 });
    DrawText("*", screenWidth - 34, 13, 24, Color{ 30, 20, 0, 255 });

    DrawText("~~~ MAPA ~~~", (screenWidth - MeasureText("~~~ MAPA ~~~", 22)) / 2, 65, 22, YELLOW);

    // GRID - Centrado uniforme
    int gridStartY = 130;
    int gridEndY = screenHeight - 80;
    int nodeX[MAX_LEVELS], nodeY[MAX_LEVELS];

    int gridCols = 3; // columnas 0, 1, 2
    int gridRows = 2; // filas 0, 1
    int cellW = screenWidth / (gridCols + 1);
    int cellH = (gridEndY - gridStartY) / gridRows;

    for (int i = 0; i < MAX_LEVELS; i++) {
        nodeX[i] = cellW * (LEVEL_GRID_COL[i] + 1);
        nodeY[i] = gridStartY + cellH * LEVEL_GRID_ROW[i] + cellH / 2;
    }

    // Conexiones
    for (int c = 0; c < CONNECTION_COUNT; c++) {
        DrawLineEx({ (float)nodeX[LEVEL_CONNECTIONS[c][0]], (float)nodeY[LEVEL_CONNECTIONS[c][0]] },
            { (float)nodeX[LEVEL_CONNECTIONS[c][1]], (float)nodeY[LEVEL_CONNECTIONS[c][1]] },
            4.0f, Color{ 255, 220, 0, 150 });
    }

    // Nodos
    for (int i = 0; i < MAX_LEVELS; i++) {
        DrawLevelNode(i, nodeX[i], nodeY[i], mapState->levels, (i == mapState->selectedLevel), mapState->selectPulse);
    }

    const char* navHint = "FLECHAS: navegar    ESPACIO: jugar    M: ajustes";
    DrawText(navHint, (screenWidth - MeasureText(navHint, 12)) / 2, screenHeight - 42, 12, Color{ 200, 170, 0, 255 });

    if (mapState->settingsOpen) DrawSettings(mapState, screenWidth, screenHeight);
}

void MapUnload(MapState* mapState) {
    UnloadTexture(texStarCollected);
    UnloadTexture(texStarEmpty);
}