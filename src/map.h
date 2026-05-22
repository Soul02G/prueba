#pragma once
#include "raylib.h"
#include "scenes.h"

#define MAX_LEVELS        6
#define MAX_STARS         3
#define TOTAL_COINS_KEY   "totalCoins"

typedef struct {
    int   starsEarned;   // 0-3 estrellas conseguidas
    int   completed;     // 1 si completado
} LevelProgress;

typedef struct MapState {
    LevelProgress levels[MAX_LEVELS];
    int           totalCoins;       // monedas totales acumuladas
    int           selectedLevel;    // nivel seleccionado en el mapa (0-based)
    int           settingsOpen;     // 1 = panel ajustes abierto
    int           settingsOption;   // 0 para SFX, 1 para VOLUMEN
    bool          musicEnabled;     // ON/OFF
    float         masterVolume;     // 0.0 a 1.0

    // Animación de selección
    float         selectPulse;
} MapState;

// Prototipos de funciones
void      MapLoad(MapState* mapState);
SceneType MapUpdate(MapState* mapState);
void      MapDraw(const MapState* mapState, int screenWidth, int screenHeight);
void      MapUnload(MapState* mapState);
void      MapAddCoins(MapState* mapState, int amount);
void      MapRegisterLevelComplete(MapState* mapState, int levelIndex, int starsEarned, int coinsEarned);