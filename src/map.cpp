#include "map.h"
#include <stdio.h>
#include <math.h>

// Posiciones de los 6 niveles en pantalla (estilo grid 3x2)
static const int LEVEL_GRID_COL[MAX_LEVELS] = { 0, 0, 1, 1, 2, 2 };
static const int LEVEL_GRID_ROW[MAX_LEVELS] = { 0, 1, 1, 0, 0, 1 };

// Conexiones secuenciales: 1->2, 2->3, 3->4, 4->5, 5->6
static const int LEVEL_CONNECTIONS[5][2] = {
    {0, 1}, {1, 2}, {2, 3}, {3, 4}, {4, 5}
};
static const int CONNECTION_COUNT = 5;

// Texturas de estrellas
static Texture2D texStarCollected;
static Texture2D texStarEmpty;

void MapLoad(MapState* mapState) {


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
        if (IsKeyPressed(KEY_M))
            mapState->settingsOpen = 0;

        // Navegación vertical entre SFX y VOLUMEN
        if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) mapState->settingsOption = 0;
        if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) mapState->settingsOption = 1;

        if (mapState->settingsOption == 0) { // Opción SFX
            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_LEFT)) {
                mapState->musicEnabled = !mapState->musicEnabled;
            }
        }
        else if (mapState->settingsOption == 1) { // Opción VOLUMEN
            if (IsKeyDown(KEY_LEFT))  mapState->masterVolume -= 0.01f;
            if (IsKeyDown(KEY_RIGHT)) mapState->masterVolume += 0.01f;

            if (mapState->masterVolume < 0.0f) mapState->masterVolume = 0.0f;
            if (mapState->masterVolume > 1.0f) mapState->masterVolume = 1.0f;
        }

        // APLICAR VOLUMEN: Si SFX está OFF, volumen 0. Si no, masterVolume.
        float finalVolume = mapState->musicEnabled ? mapState->masterVolume : 0.0f;
        SetMasterVolume(finalVolume);
        return SCENE_MAP;
    }

    int col = LEVEL_GRID_COL[mapState->selectedLevel];
    int row = LEVEL_GRID_ROW[mapState->selectedLevel];

    if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) {
        for (int i = 0; i < MAX_LEVELS; i++)
            if (LEVEL_GRID_COL[i] == col + 1 && LEVEL_GRID_ROW[i] == row)
            {
                mapState->selectedLevel = i; break;
            }
    }
    if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A)) {
        for (int i = 0; i < MAX_LEVELS; i++)
            if (LEVEL_GRID_COL[i] == col - 1 && LEVEL_GRID_ROW[i] == row)
            {
                mapState->selectedLevel = i; break;
            }
    }
    if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) {
        for (int i = 0; i < MAX_LEVELS; i++)
            if (LEVEL_GRID_COL[i] == col && LEVEL_GRID_ROW[i] == row - 1)
            {
                mapState->selectedLevel = i; break;
            }
    }
    if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) {
        for (int i = 0; i < MAX_LEVELS; i++)
            if (LEVEL_GRID_COL[i] == col && LEVEL_GRID_ROW[i] == row + 1)
            {
                mapState->selectedLevel = i; break;
            }
    }

    if (IsKeyPressed(KEY_TAB) || IsKeyPressed(KEY_P))
        mapState->settingsOpen = 1;

    if (IsKeyPressed(KEY_ESCAPE))
        return SCENE_MENU;

    if (IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_ENTER))
        return SCENE_GAME;

    return SCENE_MAP;
}

static void DrawLevelNode(int levelIndex, int screenX, int screenY,
    const LevelProgress* progress, int isSelected, float pulse)
{
    int nodeW = 110;
    int nodeH = 110;  // un poco mas alto para caber las estrellas
    int nx = screenX - nodeW / 2;
    int ny = screenY - nodeH / 2;

    // Sombra
    DrawRectangle(nx + 4, ny + 4, nodeW, nodeH, Color{ 40, 30, 0, 200 });

    // Fondo del nodo
    Color bgColor = isSelected
        ? Color{ 255, 230, 0, 255 }
    : Color{ 210, 180, 0, 255 };
    DrawRectangle(nx, ny, nodeW, nodeH, bgColor);

    // Bordes
    DrawRectangleLines(nx, ny, nodeW, nodeH, Color{ 80, 60, 0, 255 });
    DrawRectangleLines(nx + 2, ny + 2, nodeW - 4, nodeH - 4, Color{ 255, 255, 100, 200 });

    // Pulso de selección
    if (isSelected) {
        float alpha = (sinf(pulse) + 1.0f) / 2.0f;
        DrawRectangleLines(nx - 3, ny - 3, nodeW + 6, nodeH + 6,
            Color{ 255, 255, 0, (unsigned char)(180 * alpha) });
    }

    // Número del nivel
    const char* numText = TextFormat("%d", levelIndex + 1);
    int fontSize = 36;
    int textW = MeasureText(numText, fontSize);
    DrawText(numText, nx + (nodeW - textW) / 2, ny + 8, fontSize, Color{ 30, 20, 0, 255 });

    // ── ESTRELLAS ──
    int starSize = 28;   // tamaño de cada estrella en pantalla
    int starPad = 4;
    int starsRowW = 3 * starSize + 2 * starPad;
    int starStartX = nx + (nodeW - starsRowW) / 2;
    int starY = ny + nodeH - starSize - 6;

    for (int s = 0; s < 3; s++)
    {
        int sx = starStartX + s * (starSize + starPad);

        Texture2D tex = (s < progress->starsEarned) ? texStarCollected : texStarEmpty;

        if (tex.id > 0)
        {
            // Textura cargada correctamente: dibujar escalada
            Rectangle src = { 0, 0, (float)tex.width, (float)tex.height };
            Rectangle dest = { (float)sx, (float)starY, (float)starSize, (float)starSize };
            DrawTexturePro(tex, src, dest, { 0, 0 }, 0.0f, WHITE);
        }
        else
        {
            // Fallback: círculo de color si la textura no cargó
            Color fallback = (s < progress->starsEarned)
                ? Color{ 255, 220, 0, 255 }
            : Color{ 80, 60, 0, 200 };
            DrawCircle(sx + starSize / 2, starY + starSize / 2, starSize / 2, fallback);
        }
    }
}

static void DrawSettings(const MapState* mapState, int screenWidth, int screenHeight) {
    DrawRectangle(0, 0, screenWidth, screenHeight, ColorAlpha(BLACK, 0.7f));

    int panelW = 400;
    int panelH = 250;
    int px = (screenWidth - panelW) / 2;
    int py = (screenHeight - panelH) / 2;

    // Fondo del panel
    DrawRectangle(px, py, panelW, panelH, Color{ 255, 220, 0, 255 });
    DrawRectangleLines(px, py, panelW, panelH, Color{ 80, 60, 0, 255 });
    DrawRectangleLines(px + 3, py + 3, panelW - 6, panelH - 6, Color{ 255, 255, 120, 255 });

    const char* title = "AJUSTES";
    DrawText(title, px + (panelW - MeasureText(title, 28)) / 2, py + 20, 28, Color{ 30, 20, 0, 255 });
    DrawLine(px + 40, py + 60, px + panelW - 40, py + 60, Color{ 100, 80, 0, 150 });

    // --- OPCIÓN SFX ---
    int sfxY = py + 85;
    Color sfxColor = (mapState->settingsOption == 0) ? BLACK : Color{ 120, 100, 0, 255 };
    if (mapState->settingsOption == 0) {
        DrawRectangle(px + 40, sfxY - 5, panelW - 80, 45, Color{ 255, 240, 100, 255 });
        DrawRectangleLines(px + 40, sfxY - 5, panelW - 80, 45, Color{ 80, 60, 0, 255 });
    }

    const char* sfxStatus = mapState->musicEnabled ? "SFX: ON" : "SFX: OFF";
    DrawText(sfxStatus, px + (panelW - MeasureText(sfxStatus, 22)) / 2, sfxY + 6, 22, sfxColor);

    // --- OPCIÓN VOLUMEN ---
    int volY = py + 150;
    Color volColor = (mapState->settingsOption == 1) ? BLACK : Color{ 120, 100, 0, 255 };

    // Etiqueta "VOLUMEN"
    DrawText("VOLUMEN", px + (panelW - MeasureText("VOLUMEN", 18)) / 2, volY - 5, 18, volColor);

    // Barra de fondo del slider
    int sliderW = 200;
    int sliderH = 20;
    int sliderX = px + (panelW - sliderW) / 2;
    int sliderY = volY + 25;

    // 1. Fondo oscuro de la barra
    DrawRectangle(sliderX, sliderY, sliderW, sliderH, Color{ 100, 80, 0, 255 });

    // 2. NUEVO: Relleno de la barra (Muestra el progreso del volumen)
    // Usamos un color dorado para que destaque sobre el fondo oscuro
    DrawRectangle(sliderX, sliderY, (int)(mapState->masterVolume * sliderW), sliderH, Color{ 200, 160, 0, 255 });

    // 3. Slider (Manejador)
    int handleW = 15;
    int handleX = sliderX + (int)(mapState->masterVolume * (sliderW - handleW));

    // Si la opción de volumen está seleccionada, el slider brilla más
    Color handleColor = (mapState->settingsOption == 1) ? Color{ 255, 255, 200, 255 } : Color{ 200, 180, 100, 255 };
    DrawRectangle(handleX, sliderY - 5, handleW, sliderH + 10, handleColor);
    DrawRectangleLines(handleX, sliderY - 5, handleW, sliderH + 10, BLACK);

    // Texto del porcentaje
    const char* volPct = TextFormat("%d%%", (int)(mapState->masterVolume * 100));
    Color pctColor = (mapState->settingsOption == 1) ? BLACK : Color{ 120, 100, 0, 255 };
    DrawText(volPct, sliderX + sliderW + 15, sliderY - 2, 16, pctColor);

    // Hint inferior
    const char* hint = "ARRIBA/ABAJO para navegar";
    DrawText(hint, px + (panelW - MeasureText(hint, 13)) / 2, py + panelH - 25, 13, Color{ 100, 80, 0, 200 });
}

void MapDraw(const MapState* mapState, int screenWidth, int screenHeight) {
    ClearBackground(BLACK);

    // ── HEADER ──
    DrawRectangle(0, 0, screenWidth, 50, Color{ 30, 25, 0, 255 });

    DrawCircle(screenWidth - 130, 25, 14, Color{ 255, 220, 0, 255 });
    DrawCircleLines(screenWidth - 130, 25, 14, Color{ 80, 60, 0, 255 });
    DrawText(TextFormat("%d", mapState->totalCoins), screenWidth - 110, 17, 18, Color{ 255, 220, 0, 255 });

    DrawRectangle(screenWidth - 44, 8, 36, 36, Color{ 255, 220, 0, 255 });
    DrawRectangleLines(screenWidth - 44, 8, 36, 36, Color{ 80, 60, 0, 255 });
    DrawText("*", screenWidth - 34, 13, 24, Color{ 30, 20, 0, 255 });

    // ── TÍTULO MAPA ──
    const char* mapTitle = "~~~ MAPA ~~~";
    int mapTitleW = MeasureText(mapTitle, 22);
    DrawText(mapTitle, (screenWidth - mapTitleW) / 2, 65, 22, Color{ 255, 220, 0, 255 });

    const char* modeText = "MODO BASADO EN FASES";
    int modeW = MeasureText(modeText, 14);
    DrawText(modeText, (screenWidth - modeW) / 2, 94, 14, Color{ 180, 0, 220, 255 });

    // ── GRID DE NIVELES ──
    int gridStartY = 130;
    int gridEndY = screenHeight - 80;
    int gridH = gridEndY - gridStartY;
    int gridW = screenWidth;

    int nodeX[MAX_LEVELS];
    int nodeY[MAX_LEVELS];
    int cols = 3, rows = 2;

    for (int i = 0; i < MAX_LEVELS; i++) {
        int c = LEVEL_GRID_COL[i];
        int r = LEVEL_GRID_ROW[i];
        nodeX[i] = gridW / (cols + 1) * (c + 1);
        nodeY[i] = gridStartY + gridH / (rows + 1) * (r + 1);
    }

    // Conexiones
    for (int c = 0; c < CONNECTION_COUNT; c++) {
        int a = LEVEL_CONNECTIONS[c][0];
        int b = LEVEL_CONNECTIONS[c][1];
        DrawLineEx(
            { (float)nodeX[a], (float)nodeY[a] },
            { (float)nodeX[b], (float)nodeY[b] },
            4.0f, Color{ 255, 220, 0, 180 });
    }

    // Nodos
    for (int i = 0; i < MAX_LEVELS; i++) {
        DrawLevelNode(i, nodeX[i], nodeY[i],
            &mapState->levels[i],
            i == mapState->selectedLevel,
            mapState->selectPulse);
    }

    // ── INSTRUCCIONES ──
    DrawRectangle(0, screenHeight - 70, screenWidth, 70, Color{ 30, 25, 0, 255 });
    const char* navHint = "FLECHAS: navegar    ESPACIO: jugar    M: ajustes    ESC: salir";
    int navW = MeasureText(navHint, 12);
    DrawText(navHint, (screenWidth - navW) / 2, screenHeight - 42, 12, Color{ 200, 170, 0, 255 });

    const char* selText = TextFormat("NIVEL %d seleccionado", mapState->selectedLevel + 1);
    int selW = MeasureText(selText, 14);
    DrawText(selText, (screenWidth - selW) / 2, screenHeight - 22, 14, Color{ 255, 220, 0, 255 });

    if (mapState->settingsOpen)
        DrawSettings(mapState, screenWidth, screenHeight);
}

void MapUnload(MapState* mapState) {
    UnloadTexture(texStarCollected);
    UnloadTexture(texStarEmpty);
}

void MapRegisterLevelComplete(MapState* mapState, int levelIndex, int starsEarned, int coinsEarned) {
    if (levelIndex < 0 || levelIndex >= MAX_LEVELS) return;

    mapState->levels[levelIndex].completed = 1;
    if (starsEarned > mapState->levels[levelIndex].starsEarned)
        mapState->levels[levelIndex].starsEarned = starsEarned;

    mapState->totalCoins += coinsEarned;
}