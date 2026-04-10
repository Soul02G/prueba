#pragma once
#include "raylib.h"
#include "scenes.h"

#define MAX_LEVELS       6
#define MAX_STARS        3
#define TOTAL_COINS_KEY  "totalCoins"

typedef struct {
    int   starsEarned;   // 0-3 estrellas conseguidas
    int   completed;     // 1 si completado
} LevelProgress;

typedef struct {
    LevelProgress levels[MAX_LEVELS];
    int           totalCoins;       // monedas totales acumuladas
    int           selectedLevel;    // nivel seleccionado en el mapa (0-based)
    int           settingsOpen;     // 1 = panel ajustes abierto

    // Animación de selección
    float         selectPulse;
} MapState;

void  MapLoad(MapState* mapState);
SceneType MapUpdate(MapState* mapState);
void  MapDraw(const MapState* mapState, int screenWidth, int screenHeight);
void  MapUnload(MapState* mapState);

// Llamado desde game cuando se completa un nivel
void  MapRegisterLevelComplete(MapState* mapState, int levelIndex, int starsEarned, int coinsEarned);