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

struct MapState {
    LevelProgress levels[MAX_LEVELS];
    int           totalCoins;       // monedas totales acumuladas
    int           selectedLevel;    // nivel seleccionado en el mapa (0-based)
    int           settingsOpen;     // 1 = panel ajustes abierto
    int           settingsOption;   // 0 para SFX, 1 para VOLUMEN
    bool          musicEnabled;     // ON/OFF
    float         masterVolume;     // 0.0 a 1.0

    // Animación de selección
    float           selectPulse;
};

// Prototipos de funciones
void      MapLoad(struct MapState* mapState);
SceneType MapUpdate(struct MapState* mapState);
void      MapDraw(const struct MapState* mapState, int screenWidth, int screenHeight);
void      MapUnload(struct MapState* mapState);
void      MapAddCoins(MapState* mapState, int amount);

// Registra progreso: Asegúrate de que en map.cpp la función reciba estos 4 parámetros
void      MapRegisterLevelComplete(struct MapState* mapState, int levelIndex, int starsEarned, int coinsEarned);