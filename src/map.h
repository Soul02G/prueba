#pragma once
#include "raylib.h"
#include "scenes.h"

#define MAX_LEVELS        6
#define MAX_STARS         3
#define TOTAL_COINS_KEY   "totalCoins"

typedef struct {
    int   starsEarned;
    int   completed;
} LevelProgress;

typedef struct MapState {
    LevelProgress levels[MAX_LEVELS];
    int           totalCoins;
    int           selectedLevel;
    int           settingsOpen;
    int           settingsOption;
    bool          musicEnabled;
    float         masterVolume;
    float         selectPulse;

    // --- Skins jugador y murcielago (existentes) ---
    bool          playerSkinBought;
    int           playerSkinIndex;
    bool          batSkinBought;
    int           batSkinIndex;

    // --- Skin coco ---
    bool          cocoSkinBought;
    int           cocoSkinIndex;      // 0 = base, 1 = cocoS

    // --- Skin flecha ---
    bool          flechaSkinBought;
    int           flechaSkinIndex;    // 0 = base, 1 = flechaS

    // --- Skin tiraflechas (2 skins alternativas) ---
    bool          tiraflechasSkinBought;   // tiraflechasS.png
    bool          tiraflechasSSkinBought;  // tiraflechasSS.png
    int           tiraflechasSkinIndex;    // 0 = base, 1 = tiraflechasS, 2 = tiraflechasSS

    // --- Skin mono (animada) ---
    bool          monkeySkinBought;
    int           monkeySkinIndex;    // 0 = base, 1 = MonkeyFramesS_N

} MapState;

void      MapLoad(MapState* mapState);
SceneType MapUpdate(MapState* mapState);
void      MapDraw(const MapState* mapState, int screenWidth, int screenHeight);
void      MapUnload(MapState* mapState);
void      MapAddCoins(MapState* mapState, int amount);
void      MapRegisterLevelComplete(MapState* mapState, int levelIndex, int starsEarned, int coinsEarned);