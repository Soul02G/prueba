#include "game.h"
#include "map.h"
#include "raylib.h"
#include <string.h>
#include <cstdlib>
#include <cmath>
#include <cstring>
#include <cstdio>

// --- 1. PROTOTIPOS ---
static void CollectTileUnderPlayer(GameState* gameState);
void ResetGameState(GameState* gameState);
static void PlacePlayerAtSpawn(GameState* gameState);
static void InsertLeaderboard(GameState* gs);
static int GetWallVariant1(const int map[MAP_ROWS_1][MAP_COLUMNS_1], int row, int col);
static int GetWallVariant2(const int map[MAP_ROWS_2][MAP_COLUMNS_2], int row, int col);
static bool CheckWallCollision(GameState* gameState, float x, float y);

static inline bool TileIsSpike(int t) {
    return t == TILE_SPIKE_UP || t == TILE_SPIKE_DOWN || t == TILE_SPIKE_LEFT || t == TILE_SPIKE_RIGHT;
}

// Bloques alternantes: rojo sólido cuando !toggle, azul sólido cuando toggle
static inline bool TileBlockSolid(int t, bool toggle) {
    return (t == TILE_BLOCK_RED && !toggle) || (t == TILE_BLOCK_BLUE && toggle);
}

static inline bool TileIsTotem(int t) {
    return t >= TILE_TOTEM_UP && t <= TILE_TOTEM_RIGHT;
}

// Dado un tile de pincho, devuelve el offset de la casilla peligrosa (dRow, dCol)
static inline void SpikeHazardOffset(int t, int* dRow, int* dCol) {
    *dRow = 0; *dCol = 0;
    if (t == TILE_SPIKE_UP)    *dRow = -1;
    if (t == TILE_SPIKE_DOWN)  *dRow = 1;
    if (t == TILE_SPIKE_LEFT)  *dCol = -1;
    if (t == TILE_SPIKE_RIGHT) *dCol = 1;
}


static const int playerAnimSequence[PLAYER_ANIM_SEQ_LEN] = { 0, 1, 0, 2, 3, 2, 0, 4 };
static const int playerAnimSpeed = 10;
static const int coinAnimSequence[COIN_ANIM_SEQ_LEN] = { 0, 1, 2, 3 };
static const int coinAnimSpeed = 20;
static const int batAnimSequence[BAT_ANIM_SEQ_LEN] = { 0, 1, 2, 3, 2, 1 };
static const int batAnimSpeed = 10;
static const int monkeyAnimSequence[MONKEY_FRAMES] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
static const char* INITIALS_CHARS = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
static const int INITIALS_CHAR_COUNT = 36;

// --- DATOS DEL NIVEL 1 ---
static const int initialMap[MAP_ROWS_1][MAP_COLUMNS_1] = {
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,12,1,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,3,1,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,3,1,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,3,1,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,3,1,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,1,5,3,3,3,1,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,1,3,0,1,1,1,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,1,3,0,1,1,1,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,1,3,3,3,1,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,1,1,1,3,1,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,1,3,1,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,1,3,1,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,1,1,1,3,1,1,1,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,1,3,3,3,3,3,19,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,1,3,0,3,1,4,1,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,22,0,1,3,0,3,1,3,1,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,1,3,3,5,1,3,1,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,3,1,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,1,3,3,3,3,1,0,1,3,1,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,1,3,1,1,3,1,1,1,3,1,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,1,3,1,1,3,4,3,3,3,1,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,1,1,1,1,3,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0},
    {0,0,0,0,0,1,21,3,3,3,3,3,3,3,3,1,1,5,3,4,3,1,0,0,0,0,0},
    {0,0,0,0,0,1,3,0,0,4,1,1,1,1,3,3,3,3,0,0,3,19,0,0,0,0,0},
    {0,0,0,0,0,1,3,0,0,3,1,0,0,1,1,1,1,1,0,0,3,1,0,0,0,0,0},
    {0,0,0,0,0,1,3,3,3,3,1,0,0,0,1,0,0,0,0,0,3,1,0,0,0,0,0},
    {0,0,0,0,0,1,1,1,1,1,1,0,0,0,1,0,0,0,0,0,3,1,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,16,0,0,2,0,0,3,1,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
};

static const int LEVEL_2_DATA[47][25] = {
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,1,1,1,0,0,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,1,12,1,0,0,1,3,3,3,3,3,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {0,1,3,1,1,1,1,3,0,0,0,3,1,3,3,4,3,1,3,3,3,4,3,3,1},
    {0,1,3,3,4,3,3,3,0,3,4,3,1,3,0,3,3,1,0,0,6,0,0,4,1},
    {0,1,1,1,1,1,1,1,7,3,0,3,0,3,1,3,3,1,4,1,1,0,0,3,1},
    {0,0,0,0,0,0,0,1,1,3,3,3,3,3,1,3,3,3,3,1,3,3,3,3,1},
    {0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,3,3,3,3,1},
    {0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,1,5,3,3,3,3,1,1,1,1},
    {0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,3,1,1,1,0,0,0},
    {0,0,0,0,0,1,3,3,3,3,3,3,3,3,3,1,1,1,3,1,1,1,1,1,1},
    {0,0,0,0,1,1,3,1,1,0,3,0,0,3,3,1,10,3,3,3,3,3,3,9,1},
    {0,0,1,1,1,0,3,3,3,3,3,1,1,3,3,0,3,0,4,0,0,0,0,3,1},
    {0,0,1,6,0,0,3,0,1,1,1,1,1,5,3,3,3,1,8,3,3,3,3,11,1},
    {0,0,1,6,0,0,3,0,1,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {0,0,1,0,0,0,3,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,1,0,0,3,5,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,1,1,1,3,1,1,3,3,3,3,1,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,1,3,3,3,3,3,4,3,1,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,1,1,1,1,3,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0},
    {0,0,1,1,1,1,10,3,3,4,3,0,0,0,9,1,0,0,0,0,0,0,0,0,0},
    {1,1,1,3,3,3,3,3,3,3,1,1,1,0,3,1,0,0,0,0,0,0,0,0,0},
    {1,6,0,3,0,1,0,0,0,3,1,1,10,3,11,1,0,0,0,0,0,0,0,0,0},
    {1,1,1,3,0,0,0,0,0,1,1,1,8,3,9,1,0,0,0,0,0,0,0,0,0},
    {0,0,1,3,3,3,3,4,3,3,1,1,10,3,11,1,0,0,0,0,0,0,0,0,0},
    {0,0,1,1,1,1,3,0,0,3,1,1,8,4,9,1,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,1,3,0,0,3,1,1,1,0,3,1,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,1,8,3,3,11,1,3,3,3,11,1,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,1,1,1,1,1,1,3,0,1,1,1,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,1,3,0,0,1,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,1,1,1,1,1,1,1,3,0,0,1,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,1,0,0,6,0,3,3,3,3,3,1,0,0,1,1,1,1,1,1,1,1},
    {0,0,0,0,1,0,0,0,0,0,1,0,0,0,1,0,0,1,3,3,3,3,3,4,1},
    {0,0,0,0,1,6,0,0,0,0,1,1,1,1,1,1,1,1,4,0,0,0,0,3,1},
    {0,0,0,0,1,1,1,1,0,3,3,3,3,3,3,0,0,0,3,0,0,0,0,4,1},
    {0,0,0,0,0,0,0,1,0,3,3,3,3,1,1,3,3,3,3,0,0,0,0,3,1},
    {0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0,2,0,4,1},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
};


// --- DATOS DEL NIVEL 3 ---
static const int LEVEL_3_DATA[MAP_ROWS_3][MAP_COLUMNS_3] = {
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,14,14,14,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,1,2,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,1,1,0,13,13,13,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
};

// --- DATOS DEL NIVEL 4 ---
static const int LEVEL_4_DATA[MAP_ROWS_4][MAP_COLUMNS_4] = {
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,1,12,1,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,1,3,1,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,1,2,1,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
};

// --- DATOS DEL NIVEL 5 ---
static const int LEVEL_5_DATA[MAP_ROWS_5][MAP_COLUMNS_5] = {
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,1,12,1,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,1,3,1,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,1,2,1,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
};

// --- FUNCIONES DE PAREDES ---
static int TileIsWall1(const int map[MAP_ROWS_1][MAP_COLUMNS_1], int row, int col) {
    if (row < 0 || row >= MAP_ROWS_1 || col < 0 || col >= MAP_COLUMNS_1) return 0;
    return (map[row][col] == TILE_WALL);
}
static int GetWallVariant1(const int map[MAP_ROWS_1][MAP_COLUMNS_1], int row, int col) {
    int A = TileIsWall1(map, row - 1, col), B = TileIsWall1(map, row + 1, col);
    int L = TileIsWall1(map, row, col - 1), R = TileIsWall1(map, row, col + 1);
    int AL = TileIsWall1(map, row - 1, col - 1), AR = TileIsWall1(map, row - 1, col + 1);
    int BL = TileIsWall1(map, row + 1, col - 1), BR = TileIsWall1(map, row + 1, col + 1);
    if (!A && !L) return WALL_CORNER_TL; if (!A && !R) return WALL_CORNER_TR;
    if (!B && !L) return WALL_CORNER_BL; if (!B && !R) return WALL_CORNER_BR;
    if (!A) return WALL_BORDER_TOP; if (!B) return WALL_BORDER_BOTTOM;
    if (!L) return WALL_BORDER_LEFT; if (!R) return WALL_BORDER_RIGHT;
    if ((!AL) + (!AR) + (!BL) + (!BR) >= 1) {
        if (!AL) return WALL_INNER_CORNER_TL; if (!AR) return WALL_INNER_CORNER_TR;
        if (!BL) return WALL_INNER_CORNER_BL; if (!BR) return WALL_INNER_CORNER_BR;
    }
    return WALL_SOLID;
}

static int TileIsWall2(const int map[MAP_ROWS_2][MAP_COLUMNS_2], int row, int col) {
    if (row < 0 || row >= MAP_ROWS_2 || col < 0 || col >= MAP_COLUMNS_2) return 0;
    return (map[row][col] == TILE_WALL);
}
static int GetWallVariant2(const int map[MAP_ROWS_2][MAP_COLUMNS_2], int row, int col) {
    int A = TileIsWall2(map, row - 1, col), B = TileIsWall2(map, row + 1, col);
    int L = TileIsWall2(map, row, col - 1), R = TileIsWall2(map, row, col + 1);
    int AL = TileIsWall2(map, row - 1, col - 1), AR = TileIsWall2(map, row - 1, col + 1);
    int BL = TileIsWall2(map, row + 1, col - 1), BR = TileIsWall2(map, row + 1, col + 1);
    if (!A && !L) return WALL_CORNER_TL; if (!A && !R) return WALL_CORNER_TR;
    if (!B && !L) return WALL_CORNER_BL; if (!B && !R) return WALL_CORNER_BR;
    if (!A) return WALL_BORDER_TOP; if (!B) return WALL_BORDER_BOTTOM;
    if (!L) return WALL_BORDER_LEFT; if (!R) return WALL_BORDER_RIGHT;
    if ((!AL) + (!AR) + (!BL) + (!BR) >= 1) {
        if (!AL) return WALL_INNER_CORNER_TL; if (!AR) return WALL_INNER_CORNER_TR;
        if (!BL) return WALL_INNER_CORNER_BL; if (!BR) return WALL_INNER_CORNER_BR;
    }
    return WALL_SOLID;
}

static int TileIsWall3(const int map[MAP_ROWS_3][MAP_COLUMNS_3], int row, int col) {
    if (row < 0 || row >= MAP_ROWS_3 || col < 0 || col >= MAP_COLUMNS_3) return 0;
    return (map[row][col] == TILE_WALL);
}
static int GetWallVariant3(const int map[MAP_ROWS_3][MAP_COLUMNS_3], int row, int col) {
    int A = TileIsWall3(map, row - 1, col), B = TileIsWall3(map, row + 1, col);
    int L = TileIsWall3(map, row, col - 1), R = TileIsWall3(map, row, col + 1);
    int AL = TileIsWall3(map, row - 1, col - 1), AR = TileIsWall3(map, row - 1, col + 1);
    int BL = TileIsWall3(map, row + 1, col - 1), BR = TileIsWall3(map, row + 1, col + 1);
    if (!A && !L) return WALL_CORNER_TL; if (!A && !R) return WALL_CORNER_TR;
    if (!B && !L) return WALL_CORNER_BL; if (!B && !R) return WALL_CORNER_BR;
    if (!A) return WALL_BORDER_TOP; if (!B) return WALL_BORDER_BOTTOM;
    if (!L) return WALL_BORDER_LEFT; if (!R) return WALL_BORDER_RIGHT;
    if ((!AL) + (!AR) + (!BL) + (!BR) >= 1) {
        if (!AL) return WALL_INNER_CORNER_TL; if (!AR) return WALL_INNER_CORNER_TR;
        if (!BL) return WALL_INNER_CORNER_BL; if (!BR) return WALL_INNER_CORNER_BR;
    }
    return WALL_SOLID;
}

static int TileIsWall4(const int map[MAP_ROWS_4][MAP_COLUMNS_4], int row, int col) {
    if (row < 0 || row >= MAP_ROWS_4 || col < 0 || col >= MAP_COLUMNS_4) return 0;
    return (map[row][col] == TILE_WALL);
}
static int GetWallVariant4(const int map[MAP_ROWS_4][MAP_COLUMNS_4], int row, int col) {
    int A = TileIsWall4(map, row - 1, col), B = TileIsWall4(map, row + 1, col);
    int L = TileIsWall4(map, row, col - 1), R = TileIsWall4(map, row, col + 1);
    int AL = TileIsWall4(map, row - 1, col - 1), AR = TileIsWall4(map, row - 1, col + 1);
    int BL = TileIsWall4(map, row + 1, col - 1), BR = TileIsWall4(map, row + 1, col + 1);
    if (!A && !L) return WALL_CORNER_TL; if (!A && !R) return WALL_CORNER_TR;
    if (!B && !L) return WALL_CORNER_BL; if (!B && !R) return WALL_CORNER_BR;
    if (!A) return WALL_BORDER_TOP; if (!B) return WALL_BORDER_BOTTOM;
    if (!L) return WALL_BORDER_LEFT; if (!R) return WALL_BORDER_RIGHT;
    if ((!AL) + (!AR) + (!BL) + (!BR) >= 1) {
        if (!AL) return WALL_INNER_CORNER_TL; if (!AR) return WALL_INNER_CORNER_TR;
        if (!BL) return WALL_INNER_CORNER_BL; if (!BR) return WALL_INNER_CORNER_BR;
    }
    return WALL_SOLID;
}

static int TileIsWall5(const int map[MAP_ROWS_5][MAP_COLUMNS_5], int row, int col) {
    if (row < 0 || row >= MAP_ROWS_5 || col < 0 || col >= MAP_COLUMNS_5) return 0;
    return (map[row][col] == TILE_WALL);
}
static int GetWallVariant5(const int map[MAP_ROWS_5][MAP_COLUMNS_5], int row, int col) {
    int A = TileIsWall5(map, row - 1, col), B = TileIsWall5(map, row + 1, col);
    int L = TileIsWall5(map, row, col - 1), R = TileIsWall5(map, row, col + 1);
    int AL = TileIsWall5(map, row - 1, col - 1), AR = TileIsWall5(map, row - 1, col + 1);
    int BL = TileIsWall5(map, row + 1, col - 1), BR = TileIsWall5(map, row + 1, col + 1);
    if (!A && !L) return WALL_CORNER_TL; if (!A && !R) return WALL_CORNER_TR;
    if (!B && !L) return WALL_CORNER_BL; if (!B && !R) return WALL_CORNER_BR;
    if (!A) return WALL_BORDER_TOP; if (!B) return WALL_BORDER_BOTTOM;
    if (!L) return WALL_BORDER_LEFT; if (!R) return WALL_BORDER_RIGHT;
    if ((!AL) + (!AR) + (!BL) + (!BR) >= 1) {
        if (!AL) return WALL_INNER_CORNER_TL; if (!AR) return WALL_INNER_CORNER_TR;
        if (!BL) return WALL_INNER_CORNER_BL; if (!BR) return WALL_INNER_CORNER_BR;
    }
    return WALL_SOLID;
}

// --- CHECKWALLCOLLISION ---
static bool CheckWallCollision(GameState* gameState, float x, float y) {
    int col = (int)(x / TILE_SIZE);
    int row = (int)(y / TILE_SIZE);

    if (gameState->currentLevel == 1) {
        if (row < 0 || row >= MAP_ROWS_2 || col < 0 || col >= MAP_COLUMNS_2) return true;
        return (gameState->tileMap_2[row][col] == TILE_WALL);
    }
    else if (gameState->currentLevel == 2) {
        if (row < 0 || row >= MAP_ROWS_3 || col < 0 || col >= MAP_COLUMNS_3) return true;
        return (gameState->tileMap_3[row][col] == TILE_WALL);
    }
    else if (gameState->currentLevel == 3) {
        if (row < 0 || row >= MAP_ROWS_4 || col < 0 || col >= MAP_COLUMNS_4) return true;
        return (gameState->tileMap_4[row][col] == TILE_WALL);
    }
    else if (gameState->currentLevel == 4) {
        if (row < 0 || row >= MAP_ROWS_5 || col < 0 || col >= MAP_COLUMNS_5) return true;
        return (gameState->tileMap_5[row][col] == TILE_WALL);
    }
    else {
        if (row < 0 || row >= MAP_ROWS_1 || col < 0 || col >= MAP_COLUMNS_1) return true;
        return (gameState->tileMap_1[row][col] == TILE_WALL);
    }
}

// --- RECOLECCION ---
static void CollectTileUnderPlayer(GameState* gameState) {
    int col = (int)((gameState->playerX + (TILE_SIZE / 2)) / TILE_SIZE);
    int row = (int)((gameState->playerY + (TILE_SIZE / 2)) / TILE_SIZE);
    int tileValue = 0;

    if (gameState->currentLevel == 0) {
        if (row >= 0 && row < MAP_ROWS_1 && col >= 0 && col < MAP_COLUMNS_1) {
            tileValue = gameState->tileMap_1[row][col];
            if (tileValue == TILE_DOT || tileValue == TILE_COIN || tileValue == TILE_STAR)
                gameState->tileMap_1[row][col] = TILE_EMPTY;
        }
    }
    else if (gameState->currentLevel == 1) {
        if (row >= 0 && row < MAP_ROWS_2 && col >= 0 && col < MAP_COLUMNS_2) {
            tileValue = gameState->tileMap_2[row][col];
            if (tileValue == TILE_DOT || tileValue == TILE_COIN || tileValue == TILE_STAR)
                gameState->tileMap_2[row][col] = TILE_EMPTY;
        }
    }
    else if (gameState->currentLevel == 2) {
        if (row >= 0 && row < MAP_ROWS_3 && col >= 0 && col < MAP_COLUMNS_3) {
            tileValue = gameState->tileMap_3[row][col];
            if (tileValue == TILE_DOT || tileValue == TILE_COIN || tileValue == TILE_STAR)
                gameState->tileMap_3[row][col] = TILE_EMPTY;
        }
    }
    else if (gameState->currentLevel == 3) {
        if (row >= 0 && row < MAP_ROWS_4 && col >= 0 && col < MAP_COLUMNS_4) {
            tileValue = gameState->tileMap_4[row][col];
            if (tileValue == TILE_DOT || tileValue == TILE_COIN || tileValue == TILE_STAR)
                gameState->tileMap_4[row][col] = TILE_EMPTY;
        }
    }
    else {
        if (row >= 0 && row < MAP_ROWS_5 && col >= 0 && col < MAP_COLUMNS_5) {
            tileValue = gameState->tileMap_5[row][col];
            if (tileValue == TILE_DOT || tileValue == TILE_COIN || tileValue == TILE_STAR)
                gameState->tileMap_5[row][col] = TILE_EMPTY;
        }
    }

    switch (tileValue) {
    case TILE_DOT:       gameState->score += 10;  PlaySound(gameState->soundCollectDot);   break;
    case TILE_COIN:
        gameState->coinsCollected += 1;
        printf("Moneda recogida! Llevas: %d\n", gameState->coinsCollected);
        gameState->score += 100;
        PlaySound(gameState->soundCollectCoin);
        break;
    case TILE_STAR:      gameState->starsCollected++; PlaySound(gameState->soundCollectStar); break;
    case TILE_LEVEL_END: gameState->levelCompleted = true; PlaySound(gameState->soundLevelComplete); break;
    }
}

// --- SPAWN DEL JUGADOR ---
static void PlacePlayerAtSpawn(GameState* gameState) {
    if (gameState->currentLevel == 0) {
        for (int row = 0; row < MAP_ROWS_1; row++)
            for (int col = 0; col < MAP_COLUMNS_1; col++)
                if (gameState->tileMap_1[row][col] == TILE_PLAYER_SPAWN) {
                    gameState->playerX = (float)(col * TILE_SIZE);
                    gameState->playerY = (float)(row * TILE_SIZE);
                    gameState->tileMap_1[row][col] = TILE_EMPTY;
                    return;
                }
    }
    else if (gameState->currentLevel == 1) {
        for (int row = 0; row < MAP_ROWS_2; row++)
            for (int col = 0; col < MAP_COLUMNS_2; col++)
                if (gameState->tileMap_2[row][col] == TILE_PLAYER_SPAWN) {
                    gameState->playerX = (float)(col * TILE_SIZE);
                    gameState->playerY = (float)(row * TILE_SIZE);
                    gameState->tileMap_2[row][col] = TILE_EMPTY;
                    return;
                }
    }
    else if (gameState->currentLevel == 2) {
        for (int row = 0; row < MAP_ROWS_3; row++)
            for (int col = 0; col < MAP_COLUMNS_3; col++)
                if (gameState->tileMap_3[row][col] == TILE_PLAYER_SPAWN) {
                    gameState->playerX = (float)(col * TILE_SIZE);
                    gameState->playerY = (float)(row * TILE_SIZE);
                    gameState->tileMap_3[row][col] = TILE_EMPTY;
                    return;
                }
    }
    else if (gameState->currentLevel == 3) {
        for (int row = 0; row < MAP_ROWS_4; row++)
            for (int col = 0; col < MAP_COLUMNS_4; col++)
                if (gameState->tileMap_4[row][col] == TILE_PLAYER_SPAWN) {
                    gameState->playerX = (float)(col * TILE_SIZE);
                    gameState->playerY = (float)(row * TILE_SIZE);
                    gameState->tileMap_4[row][col] = TILE_EMPTY;
                    return;
                }
    }
    else {
        for (int row = 0; row < MAP_ROWS_5; row++)
            for (int col = 0; col < MAP_COLUMNS_5; col++)
                if (gameState->tileMap_5[row][col] == TILE_PLAYER_SPAWN) {
                    gameState->playerX = (float)(col * TILE_SIZE);
                    gameState->playerY = (float)(row * TILE_SIZE);
                    gameState->tileMap_5[row][col] = TILE_EMPTY;
                    return;
                }
    }
}

// --- LEADERBOARD ---
static void InsertLeaderboard(GameState* gs) {
    if (gs->leaderboardCount < LEADERBOARD_SIZE) gs->leaderboardCount++;
    LeaderboardEntry entry;
    strncpy(entry.initials, gs->initials, 4);
    entry.score = gs->score;
    entry.stars = gs->starsCollected;
    entry.time = gs->timer;
    int pos = gs->leaderboardCount - 1;
    gs->leaderboard[pos] = entry;
    for (int i = pos; i > 0; i--) {
        if (gs->leaderboard[i].time < gs->leaderboard[i - 1].time) {
            LeaderboardEntry tmp = gs->leaderboard[i]; gs->leaderboard[i] = gs->leaderboard[i - 1]; gs->leaderboard[i - 1] = tmp;
        }
        else break;
    }
    FILE* f = fopen("leaderboard.txt", "wb");
    fwrite(&gs->leaderboardCount, sizeof(int), 1, f);
    fwrite(gs->leaderboard, sizeof(LeaderboardEntry), gs->leaderboardCount, f);
    fclose(f);
}

// --- RESET ---
void ResetGameState(GameState* gameState) {
    gameState->playerAnimFrame = 0;
    gameState->playerAnimTimer = 0;
    gameState->velocityX = 0;
    gameState->velocityY = 0;
    gameState->playerRotation = 180.0f;
    gameState->score = 0;
    gameState->levelCompleted = 0;
    gameState->blinkTimer = 0.0f;
    gameState->coinsCollected = 0;
    gameState->starsCollected = 0;
    gameState->victoryStarTimer = 0.0f;
    gameState->victoryStarsShown = 0;
    gameState->timer = 0.0f;
    gameState->timerStarted = 0;
    gameState->timerExpired = 0;
    gameState->showingLeaderboard = 0;
    gameState->showingVictoryOptions = 0;
    gameState->enteringInitials = 0;
    gameState->initialIndex = 0;
    gameState->initials[0] = 'A';
    gameState->initials[1] = 'A';
    gameState->initials[2] = 'A';
    gameState->initials[3] = '\0';
    gameState->initialCharIndex[0] = 0;
    gameState->initialCharIndex[1] = 0;
    gameState->initialCharIndex[2] = 0;
    gameState->playerDead = 0;
    gameState->playerDeadScreen = false;
    gameState->menuOpen = false;
    gameState->lastBounceTileCol = -1;
    gameState->lastBounceTileRow = -1;
    gameState->spikeCount = 0;
    gameState->totemCount = 0;
    gameState->arrowCount = 0;
    gameState->monkeyTriggered = false;
    gameState->blockToggle = false;

    if (gameState->currentLevel == 0)
        memcpy(gameState->tileMap_1, initialMap, sizeof(initialMap));
    else if (gameState->currentLevel == 1)
        memcpy(gameState->tileMap_2, LEVEL_2_DATA, sizeof(LEVEL_2_DATA));
    else if (gameState->currentLevel == 2)
        memcpy(gameState->tileMap_3, LEVEL_3_DATA, sizeof(LEVEL_3_DATA));
    else if (gameState->currentLevel == 3)
        memcpy(gameState->tileMap_4, LEVEL_4_DATA, sizeof(LEVEL_4_DATA));
    else
        memcpy(gameState->tileMap_5, LEVEL_5_DATA, sizeof(LEVEL_5_DATA));

    gameState->starsTotal = 0;
    gameState->batCount = 0;
    int rows, cols;
    if (gameState->currentLevel == 0) { rows = MAP_ROWS_1; cols = MAP_COLUMNS_1; }
    else if (gameState->currentLevel == 1) { rows = MAP_ROWS_2; cols = MAP_COLUMNS_2; }
    else if (gameState->currentLevel == 2) { rows = MAP_ROWS_3; cols = MAP_COLUMNS_3; }
    else if (gameState->currentLevel == 3) { rows = MAP_ROWS_4; cols = MAP_COLUMNS_4; }
    else { rows = MAP_ROWS_5; cols = MAP_COLUMNS_5; }

    int monkeySpawnRow = -1;
    int monkeySpawnCol = -1;

    for (int row = 0; row < rows; row++) {
        for (int col = 0; col < cols; col++) {
            int tile;
            if (gameState->currentLevel == 0) tile = gameState->tileMap_1[row][col];
            else if (gameState->currentLevel == 1) tile = gameState->tileMap_2[row][col];
            else if (gameState->currentLevel == 2) tile = gameState->tileMap_3[row][col];
            else if (gameState->currentLevel == 3) tile = gameState->tileMap_4[row][col];
            else                                   tile = gameState->tileMap_5[row][col];

            if (tile == TILE_STAR) {
                gameState->starsTotal++;
            }
            else if (tile == TILE_MONKEY_SPAWN) {
                monkeySpawnRow = row;
                monkeySpawnCol = col;
            }
            else if (tile == TILE_HORIZONTAL_BAT || tile == TILE_VERTICAL_BAT) {
                if (gameState->batCount < MAX_BATS) {
                    Bat& bat = gameState->bats[gameState->batCount];
                    bat.x = (float)(col * TILE_SIZE);
                    bat.y = (float)(row * TILE_SIZE);
                    bat.isHorizontal = (tile == TILE_HORIZONTAL_BAT) ? 1 : 0;
                    bat.stopTimer = 0.0f;
                    bat.deadly = 1;
                    if (bat.isHorizontal) { bat.velocityX = BAT_SPEED; bat.velocityY = 0.0f; }
                    else { bat.velocityX = 0.0f; bat.velocityY = -BAT_SPEED; }
                    gameState->batCount++;
                }
                if (gameState->currentLevel == 0) gameState->tileMap_1[row][col] = TILE_EMPTY;
                else if (gameState->currentLevel == 1) gameState->tileMap_2[row][col] = TILE_EMPTY;
                else if (gameState->currentLevel == 2) gameState->tileMap_3[row][col] = TILE_EMPTY;
                else if (gameState->currentLevel == 3) gameState->tileMap_4[row][col] = TILE_EMPTY;
                else                                   gameState->tileMap_5[row][col] = TILE_EMPTY;
            }
            else if (TileIsSpike(tile) && gameState->spikeCount < MAX_SPIKES) {
                int i = gameState->spikeCount++;
                gameState->spikeCol[i] = col;
                gameState->spikeRow[i] = row;
                gameState->spikeState[i] = 0;
                gameState->spikeTimer[i] = 0.0f;
            }
            else if (TileIsTotem(tile) && gameState->totemCount < MAX_TOTEMS) {
                int ti = gameState->totemCount++;
                gameState->totemCol[ti] = col;
                gameState->totemRow[ti] = row;
                gameState->totemDir[ti] = tile - TILE_TOTEM_UP;
                gameState->totemTimers[ti] = TOTEM_FIRE_INTERVAL;
            }
        }
    }

    gameState->monkey.active = true;
    gameState->monkey.hasDropped = false;
    gameState->monkey.frame = 0;
    gameState->monkey.animTimer = 0.0f;
    gameState->monkey.pauseTimer = 0.0f;
    gameState->monkey.state = MONKEY_IDLE;
    gameState->monkeyDrop.active = false;

    if (monkeySpawnRow != -1 && monkeySpawnCol != -1) {
        gameState->monkey.x = (float)(monkeySpawnCol * TILE_SIZE);
        gameState->monkey.y = (float)(monkeySpawnRow * TILE_SIZE);
        if (gameState->currentLevel == 0) gameState->tileMap_1[monkeySpawnRow][monkeySpawnCol] = TILE_EMPTY;
        else if (gameState->currentLevel == 1) gameState->tileMap_2[monkeySpawnRow][monkeySpawnCol] = TILE_EMPTY;
        else if (gameState->currentLevel == 2) gameState->tileMap_3[monkeySpawnRow][monkeySpawnCol] = TILE_EMPTY;
        else if (gameState->currentLevel == 3) gameState->tileMap_4[monkeySpawnRow][monkeySpawnCol] = TILE_EMPTY;
        else                                   gameState->tileMap_5[monkeySpawnRow][monkeySpawnCol] = TILE_EMPTY;
    }
    else {
        gameState->monkey.x = (float)(6 * TILE_SIZE);
        gameState->monkey.y = (float)(22 * TILE_SIZE);
    }

    gameState->monkeyDrop.x = gameState->monkey.x - TILE_SIZE + (TILE_SIZE * 1.0f);
    gameState->monkeyDrop.y = gameState->monkey.y + TILE_SIZE * 0.5f;
    gameState->monkeyDrop.speed = 350.0f;

    PlacePlayerAtSpawn(gameState);
    for (int i = 0; i < TRAIL_LENGTH; i++)
        gameState->trailPositions[i] = { (float)gameState->playerX, (float)gameState->playerY };
    gameState->cameraX = (float)(gameState->playerX - SCREEN_WIDTH / 2 + TILE_SIZE / 2);
    gameState->cameraY = (float)(gameState->playerY - SCREEN_HEIGHT / 2 + TILE_SIZE / 2);
}

//  GAMELOAD
void GameLoad(GameState* gameState) {
    const char* wallTextureFiles[WALL_VARIANT_COUNT] = {
        "resources\\tile_wall_solid.png","resources\\tile_wall_top.png","resources\\tile_wall_bottom.png",
        "resources\\tile_wall_left.png","resources\\tile_wall_right.png","resources\\tile_wall_tl.png",
        "resources\\tile_wall_tr.png","resources\\tile_wall_bl.png","resources\\tile_wall_br.png",
        "resources\\tile_wall_inner_tl.png","resources\\tile_wall_inner_tr.png",
        "resources\\tile_wall_inner_bl.png","resources\\tile_wall_inner_br.png"
    };
    for (int i = 0; i < WALL_VARIANT_COUNT; i++) {
        gameState->wallTextures[i] = LoadTexture(wallTextureFiles[i]);
        SetTextureFilter(gameState->wallTextures[i], TEXTURE_FILTER_POINT);
    }

    gameState->texRebote8 = LoadTexture("resources\\Rebote-D-I-8.png");
    gameState->texRebote9 = LoadTexture("resources\\Rebote-U-D-9.png");
    gameState->texRebote10 = LoadTexture("resources\\Rebote-U-I-10.png");
    gameState->texRebote11 = LoadTexture("resources\\Rebote-D-D-11.png");
    gameState->dotTexture = LoadTexture("resources\\tile_dot.png");
    gameState->coinTexture = LoadTexture("resources\\coin-frame-0.png");
    gameState->starTexture = LoadTexture("resources\\tile_star.png");
    gameState->levelEndTexture = LoadTexture("resources\\tile_end.png");
    gameState->spikeTexture = LoadTexture("resources\\pinchos.png");
    gameState->texBlockRed  = LoadTexture("resources\\red.png");
    gameState->texBlockBlue = LoadTexture("resources\\blue.png");
    SetTextureFilter(gameState->texBlockRed,  TEXTURE_FILTER_POINT);
    SetTextureFilter(gameState->texBlockBlue, TEXTURE_FILTER_POINT);
    gameState->spikeUnfold = LoadTexture("resources\\despliegue.png");
    gameState->trailHorizontal = LoadTexture("resources\\trail.png");
    gameState->trailVertical = LoadTexture("resources\\trailVert.png");
    gameState->starCollectedTexture = LoadTexture("resources\\star_collected.png");
    gameState->starEmptyTexture = LoadTexture("resources\\star_empty.png");
    gameState->coinFrames[0] = LoadTexture("resources\\coin-frame-0.png");
    gameState->coinFrames[1] = LoadTexture("resources\\coin-frame-1.png");
    gameState->coinFrames[2] = LoadTexture("resources\\coin-frame-2.png");
    gameState->coinFrames[3] = LoadTexture("resources\\coin-frame-3.png");
    gameState->playerFrames[0] = LoadTexture("resources\\totm.png");
    gameState->playerFrames[1] = LoadTexture("resources\\totm_1.png");
    gameState->playerFrames[2] = LoadTexture("resources\\totm_2.png");
    gameState->playerFrames[3] = LoadTexture("resources\\totm_3.png");
    gameState->playerFrames[4] = LoadTexture("resources\\totm_4.png");
    for (int i = 0; i < MONKEY_FRAMES; i++)
        gameState->texMonkeyFrames[i] = LoadTexture(TextFormat("resources\\MonkeyFrames_%d.png", i));
    gameState->texMonkeyDrop = LoadTexture("resources\\coco.png");
    for (int i = 0; i < 4; i++) {
        gameState->batTextures[i] = LoadTexture(TextFormat("resources/bat-frame-%d.png", i + 1));
        SetTextureFilter(gameState->batTextures[i], TEXTURE_FILTER_POINT);
    }
    gameState->texTotem = LoadTexture("resources\\tiraflechas.png");
    gameState->texArrow = LoadTexture("resources\\flecha.png");
    SetTextureFilter(gameState->texTotem, TEXTURE_FILTER_POINT);
    SetTextureFilter(gameState->texArrow, TEXTURE_FILTER_POINT);
    SetTextureFilter(gameState->dotTexture, TEXTURE_FILTER_POINT);
    SetTextureFilter(gameState->starTexture, TEXTURE_FILTER_POINT);
    SetTextureFilter(gameState->levelEndTexture, TEXTURE_FILTER_POINT);
    SetTextureFilter(gameState->spikeTexture, TEXTURE_FILTER_POINT);
    SetTextureFilter(gameState->trailHorizontal, TEXTURE_FILTER_POINT);
    SetTextureFilter(gameState->trailVertical, TEXTURE_FILTER_POINT);
    SetTextureFilter(gameState->starCollectedTexture, TEXTURE_FILTER_POINT);
    SetTextureFilter(gameState->starEmptyTexture, TEXTURE_FILTER_POINT);
    for (int i = 0; i < PLAYER_ANIM_FRAMES; i++) SetTextureFilter(gameState->playerFrames[i], TEXTURE_FILTER_POINT);
    for (int i = 0; i < COIN_ANIM_FRAMES; i++) SetTextureFilter(gameState->coinFrames[i], TEXTURE_FILTER_POINT);

    if (gameState->currentLevel == 1) {
        gameState->currentMapRows = MAP_ROWS_2; gameState->currentMapCols = MAP_COLUMNS_2;
        memcpy(gameState->tileMap_2, LEVEL_2_DATA, sizeof(LEVEL_2_DATA));
    }
    else if (gameState->currentLevel == 2) {
        gameState->currentMapRows = MAP_ROWS_3; gameState->currentMapCols = MAP_COLUMNS_3;
        memcpy(gameState->tileMap_3, LEVEL_3_DATA, sizeof(LEVEL_3_DATA));
    }
    else if (gameState->currentLevel == 3) {
        gameState->currentMapRows = MAP_ROWS_4; gameState->currentMapCols = MAP_COLUMNS_4;
        memcpy(gameState->tileMap_4, LEVEL_4_DATA, sizeof(LEVEL_4_DATA));
    }
    else if (gameState->currentLevel == 4) {
        gameState->currentMapRows = MAP_ROWS_5; gameState->currentMapCols = MAP_COLUMNS_5;
        memcpy(gameState->tileMap_5, LEVEL_5_DATA, sizeof(LEVEL_5_DATA));
    }
    else {
        gameState->currentMapRows = MAP_ROWS_1; gameState->currentMapCols = MAP_COLUMNS_1;
        memcpy(gameState->tileMap_1, initialMap, sizeof(initialMap));
    }

    gameState->soundDash = LoadSound("cosas de la wiki\\dash.wav");
    gameState->soundHitWall = LoadSound("cosas de la wiki\\hitting wall.wav");
    gameState->soundCollectDot = LoadSound("cosas de la wiki\\collecting dots.wav");
    gameState->soundCollectCoin = LoadSound("cosas de la wiki\\collecting coin.wav");
    gameState->soundCollectStar = LoadSound("cosas de la wiki\\collecting star.wav");
    gameState->soundLevelComplete = LoadSound("cosas de la wiki\\level complete.wav");
    gameState->soundLevelStart = LoadSound("cosas de la wiki\\start of the level.wav");

    gameState->leaderboardCount = 0;
    FILE* f = fopen("leaderboard.bin", "rb");
    if (f) {
        fread(&gameState->leaderboardCount, sizeof(int), 1, f);
        fread(gameState->leaderboard, sizeof(LeaderboardEntry), gameState->leaderboardCount, f);
        fclose(f);
    }

    gameState->batFrameCounter = 0;
    gameState->batCurrentFrames = 0;
    gameState->totemCount = 0;
    gameState->arrowCount = 0;

    ResetGameState(gameState);
    PlaySound(gameState->soundLevelStart);
}

void HandleBounceCollision(GameState* gameState) {
    int tileCol = (int)((gameState->playerX + TILE_SIZE / 2) / TILE_SIZE);
    int tileRow = (int)((gameState->playerY + TILE_SIZE / 2) / TILE_SIZE);

    if (tileRow < 0 || tileCol < 0) return;

    int currentTile = (gameState->currentLevel == 0) ? gameState->tileMap_1[tileRow][tileCol] :
        (gameState->currentLevel == 1) ? gameState->tileMap_2[tileRow][tileCol] :
        (gameState->currentLevel == 2) ? gameState->tileMap_3[tileRow][tileCol] :
        (gameState->currentLevel == 3) ? gameState->tileMap_4[tileRow][tileCol] :
        gameState->tileMap_5[tileRow][tileCol];

    if (currentTile >= 8 && currentTile <= 11) {
        int nextVX = 0, nextVY = 0;
        bool activated = false;

        switch (currentTile) {
        case 8:
            if (gameState->velocityX < 0) { nextVY = -PLAYER_MOVE_SPEED; activated = true; }
            else if (gameState->velocityY > 0) { nextVX = PLAYER_MOVE_SPEED; activated = true; }
            break;
        case 9:
            if (gameState->velocityX > 0) { nextVY = PLAYER_MOVE_SPEED; activated = true; }
            else if (gameState->velocityY < 0) { nextVX = -PLAYER_MOVE_SPEED; activated = true; }
            break;
        case 10:
            if (gameState->velocityX < 0) { nextVY = PLAYER_MOVE_SPEED; activated = true; }
            else if (gameState->velocityY < 0) { nextVX = PLAYER_MOVE_SPEED; activated = true; }
            break;
        case 11:
            if (gameState->velocityX > 0) { nextVY = -PLAYER_MOVE_SPEED; activated = true; }
            else if (gameState->velocityY > 0) { nextVX = -PLAYER_MOVE_SPEED; activated = true; }
            break;
        }

        if (activated) {
            gameState->velocityX = nextVX;
            gameState->velocityY = nextVY;
            gameState->playerX = (float)(tileCol * TILE_SIZE);
            gameState->playerY = (float)(tileRow * TILE_SIZE);
            float push = 2.0f;
            if (gameState->velocityX > 0)      gameState->playerX += push;
            else if (gameState->velocityX < 0) gameState->playerX -= push;
            if (gameState->velocityY > 0)      gameState->playerY += push;
            else if (gameState->velocityY < 0) gameState->playerY -= push;
            if (gameState->velocityX > 0)      gameState->playerRotation = 90;
            else if (gameState->velocityX < 0) gameState->playerRotation = 270;
            else if (gameState->velocityY > 0) gameState->playerRotation = 180;
            else if (gameState->velocityY < 0) gameState->playerRotation = 0;
            PlaySound(gameState->soundDash);
        }
    }
}

//  GAMEUPDATE
SceneType GameUpdate(GameState* gameState, MapState* mapState) {

    // --- MENU PAUSA ---
    if (IsKeyPressed(KEY_M) && !gameState->levelCompleted &&
        !gameState->showingVictoryOptions && !gameState->enteringInitials && !gameState->showingLeaderboard
        && !gameState->playerDeadScreen)
        gameState->menuOpen = !gameState->menuOpen;

    if (gameState->menuOpen) {
        if (IsKeyPressed(KEY_UP))   gameState->menuOption = (gameState->menuOption - 1 + 4) % 4;
        if (IsKeyPressed(KEY_DOWN)) gameState->menuOption = (gameState->menuOption + 1) % 4;
        if (gameState->menuOption == 0) { if (IsKeyPressed(KEY_ENTER)) { ResetGameState(gameState); gameState->menuOpen = false; } }
        else if (gameState->menuOption == 1) {
            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_RIGHT)) {
                gameState->musicEnabled = !gameState->musicEnabled; mapState->musicEnabled = gameState->musicEnabled;
            }
        }
        else if (gameState->menuOption == 2) {
            if (IsKeyDown(KEY_LEFT))  gameState->masterVolume -= 0.01f;
            if (IsKeyDown(KEY_RIGHT)) gameState->masterVolume += 0.01f;
            if (gameState->masterVolume < 0.0f) gameState->masterVolume = 0.0f;
            if (gameState->masterVolume > 1.0f) gameState->masterVolume = 1.0f;
            mapState->masterVolume = gameState->masterVolume;
        }
        else if (gameState->menuOption == 3) { if (IsKeyPressed(KEY_ENTER)) return SCENE_MAP; }
        SetMasterVolume(gameState->musicEnabled ? gameState->masterVolume : 0.0f);
        return SCENE_GAME;
    }

    // --- MUERTE ---
    if (gameState->playerDead && !gameState->playerDeadScreen) {
        PlaySound(gameState->soundHitWall);
        gameState->playerDeadScreen = true;
        gameState->velocityX = 0;
        gameState->velocityY = 0;
    }
    if (gameState->playerDeadScreen) {
        if (IsKeyPressed(KEY_SPACE)) {
            gameState->playerDeadScreen = false;
            ResetGameState(gameState);
            PlaySound(gameState->soundLevelStart);
        }
        if (IsKeyPressed(KEY_M)) {
            gameState->playerDeadScreen = false;
            return SCENE_MAP;
        }
        return SCENE_GAME;
    }

    float dt = GetFrameTime();
    gameState->blinkTimer += dt;
    if (gameState->blinkTimer > 1.0f) gameState->blinkTimer = 0.0f;

    // --- INICIALES, VICTORIA Y LEADERBOARD ---
    if (gameState->enteringInitials) {
        int idx = gameState->initialIndex;
        if (IsKeyPressed(KEY_UP)) { gameState->initialCharIndex[idx] = (gameState->initialCharIndex[idx] + 1) % INITIALS_CHAR_COUNT; gameState->initials[idx] = INITIALS_CHARS[gameState->initialCharIndex[idx]]; }
        if (IsKeyPressed(KEY_DOWN)) { gameState->initialCharIndex[idx] = (gameState->initialCharIndex[idx] - 1 + INITIALS_CHAR_COUNT) % INITIALS_CHAR_COUNT; gameState->initials[idx] = INITIALS_CHARS[gameState->initialCharIndex[idx]]; }
        if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_ENTER)) { if (idx < 2) gameState->initialIndex++; else { gameState->enteringInitials = 0; gameState->showingLeaderboard = 1; InsertLeaderboard(gameState); } }
        if (IsKeyPressed(KEY_LEFT) && idx > 0) gameState->initialIndex--;
        if (IsKeyPressed(KEY_BACKSPACE)) { gameState->enteringInitials = 0; gameState->showingLeaderboard = 0; gameState->showingVictoryOptions = 1; }
        return SCENE_GAME;
    }
    if (gameState->showingVictoryOptions) {
        if (IsKeyPressed(KEY_L)) { gameState->showingVictoryOptions = 0; gameState->enteringInitials = 1; }
        if (IsKeyPressed(KEY_M))     return SCENE_MAP;
        if (IsKeyPressed(KEY_SPACE)) { ResetGameState(gameState); PlaySound(gameState->soundLevelStart); }
        return SCENE_GAME;
    }
    if (gameState->showingLeaderboard) {
        if (IsKeyPressed(KEY_M))                              return SCENE_MAP;
        if (IsKeyPressed(KEY_L)) { gameState->showingLeaderboard = 0; gameState->showingVictoryOptions = 1; }
        if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) { ResetGameState(gameState); PlaySound(gameState->soundLevelStart); }
        return SCENE_GAME;
    }

    if (gameState->timerStarted && !gameState->levelCompleted) gameState->timer += dt;

    if (gameState->levelCompleted && !gameState->showingVictoryOptions && !gameState->enteringInitials && !gameState->showingLeaderboard)
        gameState->showingVictoryOptions = 1;
    if (gameState->levelCompleted) return SCENE_GAME;

    // --- INPUT ---
    if (gameState->velocityX == 0 && gameState->velocityY == 0) {
        bool moved = false;
        if      (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) { gameState->velocityX =  PLAYER_MOVE_SPEED; gameState->playerRotation = 90;  gameState->timerStarted = 1; PlaySound(gameState->soundDash); moved = true; }
        else if (IsKeyPressed(KEY_LEFT)  || IsKeyPressed(KEY_A)) { gameState->velocityX = -PLAYER_MOVE_SPEED; gameState->playerRotation = 270; gameState->timerStarted = 1; PlaySound(gameState->soundDash); moved = true; }
        else if (IsKeyPressed(KEY_DOWN)  || IsKeyPressed(KEY_S)) { gameState->velocityY =  PLAYER_MOVE_SPEED; gameState->playerRotation = 180; gameState->timerStarted = 1; PlaySound(gameState->soundDash); moved = true; }
        else if (IsKeyPressed(KEY_UP)    || IsKeyPressed(KEY_W)) { gameState->velocityY = -PLAYER_MOVE_SPEED; gameState->playerRotation = 0;   gameState->timerStarted = 1; PlaySound(gameState->soundDash); moved = true; }
        if (moved) gameState->blockToggle = !gameState->blockToggle;
    }

    // --- MOVIMIENTO X ---
    if (gameState->velocityX != 0) {
        int nextX = gameState->playerX + gameState->velocityX;
        int tileRow = gameState->playerY / TILE_SIZE;
        int nextTileCol = (gameState->velocityX > 0) ? (nextX + TILE_SIZE - 1) / TILE_SIZE : nextX / TILE_SIZE;
        if (gameState->currentLevel == 0) {
            nextTileCol = (nextTileCol < 0) ? 0 : (nextTileCol >= MAP_COLUMNS_1 ? MAP_COLUMNS_1 - 1 : nextTileCol);
            int h = gameState->tileMap_1[tileRow][nextTileCol];
            if (h == TILE_WALL || TileIsSpike(h) || TileIsTotem(h) || TileBlockSolid(h, gameState->blockToggle)) { gameState->playerX = (gameState->velocityX > 0) ? (nextTileCol * TILE_SIZE) - TILE_SIZE : (nextTileCol + 1) * TILE_SIZE; gameState->velocityX = 0; PlaySound(gameState->soundHitWall); }
            else if (nextX < 0 || nextX + TILE_SIZE > MAP_COLUMNS_1 * TILE_SIZE) { gameState->velocityX = 0; PlaySound(gameState->soundHitWall); }
            else gameState->playerX = nextX;
        }
        else if (gameState->currentLevel == 1) {
            nextTileCol = (nextTileCol < 0) ? 0 : (nextTileCol >= MAP_COLUMNS_2 ? MAP_COLUMNS_2 - 1 : nextTileCol);
            int h = gameState->tileMap_2[tileRow][nextTileCol];
            if (h == TILE_WALL || TileIsSpike(h) || TileIsTotem(h) || TileBlockSolid(h, gameState->blockToggle)) { gameState->playerX = (gameState->velocityX > 0) ? (nextTileCol * TILE_SIZE) - TILE_SIZE : (nextTileCol + 1) * TILE_SIZE; gameState->velocityX = 0; PlaySound(gameState->soundHitWall); }
            else if (nextX < 0 || nextX + TILE_SIZE > MAP_COLUMNS_2 * TILE_SIZE) { gameState->velocityX = 0; PlaySound(gameState->soundHitWall); }
            else gameState->playerX = nextX;
        }
        else if (gameState->currentLevel == 2) {
            nextTileCol = (nextTileCol < 0) ? 0 : (nextTileCol >= MAP_COLUMNS_3 ? MAP_COLUMNS_3 - 1 : nextTileCol);
            int h = gameState->tileMap_3[tileRow][nextTileCol];
            if (h == TILE_WALL || TileIsSpike(h) || TileIsTotem(h) || TileBlockSolid(h, gameState->blockToggle)) { gameState->playerX = (gameState->velocityX > 0) ? (nextTileCol * TILE_SIZE) - TILE_SIZE : (nextTileCol + 1) * TILE_SIZE; gameState->velocityX = 0; PlaySound(gameState->soundHitWall); }
            else if (nextX < 0 || nextX + TILE_SIZE > MAP_COLUMNS_3 * TILE_SIZE) { gameState->velocityX = 0; PlaySound(gameState->soundHitWall); }
            else gameState->playerX = nextX;
        }
        else if (gameState->currentLevel == 3) {
            nextTileCol = (nextTileCol < 0) ? 0 : (nextTileCol >= MAP_COLUMNS_4 ? MAP_COLUMNS_4 - 1 : nextTileCol);
            int h = gameState->tileMap_4[tileRow][nextTileCol];
            if (h == TILE_WALL || TileIsSpike(h) || TileIsTotem(h) || TileBlockSolid(h, gameState->blockToggle)) { gameState->playerX = (gameState->velocityX > 0) ? (nextTileCol * TILE_SIZE) - TILE_SIZE : (nextTileCol + 1) * TILE_SIZE; gameState->velocityX = 0; PlaySound(gameState->soundHitWall); }
            else if (nextX < 0 || nextX + TILE_SIZE > MAP_COLUMNS_4 * TILE_SIZE) { gameState->velocityX = 0; PlaySound(gameState->soundHitWall); }
            else gameState->playerX = nextX;
        }
        else {
            nextTileCol = (nextTileCol < 0) ? 0 : (nextTileCol >= MAP_COLUMNS_5 ? MAP_COLUMNS_5 - 1 : nextTileCol);
            int h = gameState->tileMap_5[tileRow][nextTileCol];
            if (h == TILE_WALL || TileIsSpike(h) || TileIsTotem(h) || TileBlockSolid(h, gameState->blockToggle)) { gameState->playerX = (gameState->velocityX > 0) ? (nextTileCol * TILE_SIZE) - TILE_SIZE : (nextTileCol + 1) * TILE_SIZE; gameState->velocityX = 0; PlaySound(gameState->soundHitWall); }
            else if (nextX < 0 || nextX + TILE_SIZE > MAP_COLUMNS_5 * TILE_SIZE) { gameState->velocityX = 0; PlaySound(gameState->soundHitWall); }
            else gameState->playerX = nextX;
        }
    }

    // --- MOVIMIENTO Y ---
    if (gameState->velocityY != 0) {
        int nextY = gameState->playerY + gameState->velocityY;
        int tileCol = gameState->playerX / TILE_SIZE;
        int nextTileRow = (gameState->velocityY > 0) ? (nextY + TILE_SIZE - 1) / TILE_SIZE : nextY / TILE_SIZE;
        if (gameState->currentLevel == 0) {
            nextTileRow = (nextTileRow < 0) ? 0 : (nextTileRow >= MAP_ROWS_1 ? MAP_ROWS_1 - 1 : nextTileRow);
            int h = gameState->tileMap_1[nextTileRow][tileCol];
            if (h == TILE_WALL || TileIsSpike(h) || TileIsTotem(h) || TileBlockSolid(h, gameState->blockToggle)) { gameState->playerY = (gameState->velocityY > 0) ? (nextTileRow * TILE_SIZE) - TILE_SIZE : (nextTileRow + 1) * TILE_SIZE; gameState->velocityY = 0; PlaySound(gameState->soundHitWall); }
            else if (nextY < 0 || nextY + TILE_SIZE > MAP_ROWS_1 * TILE_SIZE) { gameState->velocityY = 0; PlaySound(gameState->soundHitWall); }
            else gameState->playerY = nextY;
        }
        else if (gameState->currentLevel == 1) {
            nextTileRow = (nextTileRow < 0) ? 0 : (nextTileRow >= MAP_ROWS_2 ? MAP_ROWS_2 - 1 : nextTileRow);
            int h = gameState->tileMap_2[nextTileRow][tileCol];
            if (h == TILE_WALL || TileIsSpike(h) || TileIsTotem(h) || TileBlockSolid(h, gameState->blockToggle)) { gameState->playerY = (gameState->velocityY > 0) ? (nextTileRow * TILE_SIZE) - TILE_SIZE : (nextTileRow + 1) * TILE_SIZE; gameState->velocityY = 0; PlaySound(gameState->soundHitWall); }
            else if (nextY < 0 || nextY + TILE_SIZE > MAP_ROWS_2 * TILE_SIZE) { gameState->velocityY = 0; PlaySound(gameState->soundHitWall); }
            else gameState->playerY = nextY;
        }
        else if (gameState->currentLevel == 2) {
            nextTileRow = (nextTileRow < 0) ? 0 : (nextTileRow >= MAP_ROWS_3 ? MAP_ROWS_3 - 1 : nextTileRow);
            int h = gameState->tileMap_3[nextTileRow][tileCol];
            if (h == TILE_WALL || TileIsSpike(h) || TileIsTotem(h) || TileBlockSolid(h, gameState->blockToggle)) { gameState->playerY = (gameState->velocityY > 0) ? (nextTileRow * TILE_SIZE) - TILE_SIZE : (nextTileRow + 1) * TILE_SIZE; gameState->velocityY = 0; PlaySound(gameState->soundHitWall); }
            else if (nextY < 0 || nextY + TILE_SIZE > MAP_ROWS_3 * TILE_SIZE) { gameState->velocityY = 0; PlaySound(gameState->soundHitWall); }
            else gameState->playerY = nextY;
        }
        else if (gameState->currentLevel == 3) {
            nextTileRow = (nextTileRow < 0) ? 0 : (nextTileRow >= MAP_ROWS_4 ? MAP_ROWS_4 - 1 : nextTileRow);
            int h = gameState->tileMap_4[nextTileRow][tileCol];
            if (h == TILE_WALL || TileIsSpike(h) || TileIsTotem(h) || TileBlockSolid(h, gameState->blockToggle)) { gameState->playerY = (gameState->velocityY > 0) ? (nextTileRow * TILE_SIZE) - TILE_SIZE : (nextTileRow + 1) * TILE_SIZE; gameState->velocityY = 0; PlaySound(gameState->soundHitWall); }
            else if (nextY < 0 || nextY + TILE_SIZE > MAP_ROWS_4 * TILE_SIZE) { gameState->velocityY = 0; PlaySound(gameState->soundHitWall); }
            else gameState->playerY = nextY;
        }
        else {
            nextTileRow = (nextTileRow < 0) ? 0 : (nextTileRow >= MAP_ROWS_5 ? MAP_ROWS_5 - 1 : nextTileRow);
            int h = gameState->tileMap_5[nextTileRow][tileCol];
            if (h == TILE_WALL || TileIsSpike(h) || TileIsTotem(h) || TileBlockSolid(h, gameState->blockToggle)) { gameState->playerY = (gameState->velocityY > 0) ? (nextTileRow * TILE_SIZE) - TILE_SIZE : (nextTileRow + 1) * TILE_SIZE; gameState->velocityY = 0; PlaySound(gameState->soundHitWall); }
            else if (nextY < 0 || nextY + TILE_SIZE > MAP_ROWS_5 * TILE_SIZE) { gameState->velocityY = 0; PlaySound(gameState->soundHitWall); }
            else gameState->playerY = nextY;
        }
    }

    if (gameState->velocityX != 0 || gameState->velocityY != 0)
        HandleBounceCollision(gameState);

    // --- MURCIELAGOS ---
    for (int i = 0; i < gameState->batCount; i++) {
        Bat& bat = gameState->bats[i];
        if (bat.stopTimer > 0.0f) { bat.stopTimer -= dt; continue; }
        float nextX = bat.x + bat.velocityX;
        float nextY = bat.y + bat.velocityY;
        float checkX = (bat.velocityX > 0) ? (nextX + TILE_SIZE - 1) : (bat.velocityX < 0 ? nextX : bat.x + 5);
        float checkY = (bat.velocityY > 0) ? (nextY + TILE_SIZE - 1) : (bat.velocityY < 0 ? nextY : bat.y + 5);
        if (CheckWallCollision(gameState, checkX, checkY)) {
            bat.stopTimer = BAT_STOP_TIME;
            bat.velocityX *= -1;
            bat.velocityY *= -1;
        }
        else { bat.x = nextX; bat.y = nextY; }
    }

    // --- COLISIONES CON ENEMIGOS Y PINCHOS ---
    if (!gameState->playerDead && !gameState->levelCompleted) {
        float pL = (float)gameState->playerX + 4, pR = pL + TILE_SIZE - 8;
        float pT = (float)gameState->playerY + 4, pB = pT + TILE_SIZE - 8;

        for (int i = 0; i < gameState->batCount; i++) {
            Bat& bat = gameState->bats[i];
            if (pL < bat.x + TILE_SIZE - 4 && pR > bat.x + 4 && pT < bat.y + TILE_SIZE - 4 && pB > bat.y + 4) {
                gameState->playerDead = 1; break;
            }
        }

        if (!gameState->playerDead) {
            int pCol = (int)((gameState->playerX + TILE_SIZE / 2) / TILE_SIZE);
            int pRow = (int)((gameState->playerY + TILE_SIZE / 2) / TILE_SIZE);
            for (int i = 0; i < gameState->spikeCount; i++) {
                int sc = gameState->spikeCol[i], sr = gameState->spikeRow[i];
                int t;
                if (gameState->currentLevel == 0) t = gameState->tileMap_1[sr][sc];
                else if (gameState->currentLevel == 1) t = gameState->tileMap_2[sr][sc];
                else if (gameState->currentLevel == 2) t = gameState->tileMap_3[sr][sc];
                else if (gameState->currentLevel == 3) t = gameState->tileMap_4[sr][sc];
                else                                   t = gameState->tileMap_5[sr][sc];
                int dRow, dCol;
                SpikeHazardOffset(t, &dRow, &dCol);
                bool playerOnHazard = (pCol == sc + dCol && pRow == sr + dRow);
                switch (gameState->spikeState[i]) {
                case 0: if (playerOnHazard) { gameState->spikeState[i] = 1; gameState->spikeTimer[i] = 0.0f; } break;
                case 1: gameState->spikeTimer[i] += dt; if (gameState->spikeTimer[i] >= 0.2f) { gameState->spikeState[i] = 2; gameState->spikeTimer[i] = 0.0f; } break;
                case 2: gameState->spikeTimer[i] += dt; if (gameState->spikeTimer[i] >= 1.0f) { gameState->spikeState[i] = 3; gameState->spikeTimer[i] = 0.0f; } break;
                case 3:
                    if (playerOnHazard) gameState->playerDead = 1;
                    gameState->spikeTimer[i] += dt;
                    if (gameState->spikeTimer[i] >= 0.5f) { gameState->spikeState[i] = 0; gameState->spikeTimer[i] = 0.0f; }
                    break;
                }
            }
        }
    }

    // --- TOTEMS ---
    for (int i = 0; i < gameState->totemCount; i++) {
        gameState->totemTimers[i] -= dt;
        if (gameState->totemTimers[i] <= 0.0f) {
            gameState->totemTimers[i] = TOTEM_FIRE_INTERVAL;
            if (gameState->arrowCount < MAX_ARROWS) {
                Arrow& a = gameState->arrows[gameState->arrowCount++];
                int spawnCol = gameState->totemCol[i], spawnRow = gameState->totemRow[i];
                switch (gameState->totemDir[i]) {
                case 0: spawnRow -= 1; a.vx = 0;            a.vy = -ARROW_SPEED; a.rotation = 90.0f; break;
                case 1: spawnRow += 1; a.vx = 0;            a.vy = ARROW_SPEED; a.rotation = 270.0f; break;
                case 2: spawnCol -= 1; a.vx = -ARROW_SPEED; a.vy = 0;            a.rotation = 0.0f; break;
                case 3: spawnCol += 1; a.vx = ARROW_SPEED; a.vy = 0;            a.rotation = 180.0f; break;
                }
                a.x = (float)(spawnCol * TILE_SIZE);
                a.y = (float)(spawnRow * TILE_SIZE);
                a.active = true;
            }
        }
    }

    // --- FLECHAS ---
    {
        int mapRows, mapCols;
        if (gameState->currentLevel == 0) { mapRows = MAP_ROWS_1; mapCols = MAP_COLUMNS_1; }
        else if (gameState->currentLevel == 1) { mapRows = MAP_ROWS_2; mapCols = MAP_COLUMNS_2; }
        else if (gameState->currentLevel == 2) { mapRows = MAP_ROWS_3; mapCols = MAP_COLUMNS_3; }
        else if (gameState->currentLevel == 3) { mapRows = MAP_ROWS_4; mapCols = MAP_COLUMNS_4; }
        else { mapRows = MAP_ROWS_5; mapCols = MAP_COLUMNS_5; }
        float pL = (float)gameState->playerX + 4, pR = pL + TILE_SIZE - 8;
        float pT = (float)gameState->playerY + 4, pB = pT + TILE_SIZE - 8;
        int alive = 0;
        for (int i = 0; i < gameState->arrowCount; i++) {
            Arrow& a = gameState->arrows[i];
            if (!a.active) continue;
            a.x += a.vx; a.y += a.vy;
            int ac = (int)((a.x + TILE_SIZE / 2) / TILE_SIZE);
            int ar = (int)((a.y + TILE_SIZE / 2) / TILE_SIZE);
            bool hitWall = (ac < 0 || ac >= mapCols || ar < 0 || ar >= mapRows);
            if (!hitWall) {
                int t;
                if (gameState->currentLevel == 0) t = gameState->tileMap_1[ar][ac];
                else if (gameState->currentLevel == 1) t = gameState->tileMap_2[ar][ac];
                else if (gameState->currentLevel == 2) t = gameState->tileMap_3[ar][ac];
                else if (gameState->currentLevel == 3) t = gameState->tileMap_4[ar][ac];
                else                                   t = gameState->tileMap_5[ar][ac];
                hitWall = (t == TILE_WALL) || TileIsTotem(t);
            }
            if (hitWall) { a.active = false; continue; }
            if (!gameState->playerDead) {
                float aL = a.x + 4, aR = a.x + TILE_SIZE - 4;
                float aT = a.y + 4, aB = a.y + TILE_SIZE - 4;
                if (aL < pR && aR > pL && aT < pB && aB > pT) { gameState->playerDead = 1; a.active = false; continue; }
            }
            gameState->arrows[alive++] = a;
        }
        gameState->arrowCount = alive;
    }

    // --- SISTEMA DEL MONO ---
    {
        int rows = gameState->currentMapRows;
        int cols = gameState->currentMapCols;

        if (gameState->monkey.x == 0.0f && gameState->monkey.y == 0.0f) {
            int spawnRow = -1, spawnCol = -1;
            for (int r = 0; r < rows && spawnRow == -1; r++) {
                for (int c = 0; c < cols; c++) {
                    int t;
                    if (gameState->currentLevel == 0) t = gameState->tileMap_1[r][c];
                    else if (gameState->currentLevel == 1) t = gameState->tileMap_2[r][c];
                    else if (gameState->currentLevel == 2) t = gameState->tileMap_3[r][c];
                    else if (gameState->currentLevel == 3) t = gameState->tileMap_4[r][c];
                    else                                   t = gameState->tileMap_5[r][c];
                    if (t == TILE_MONKEY_SPAWN) { spawnRow = r; spawnCol = c; break; }
                }
            }
            if (spawnRow != -1) {
                gameState->monkey.active = true;
                gameState->monkey.state = MONKEY_IDLE;
                gameState->monkey.hasDropped = false;
                gameState->monkey.x = (float)(spawnCol * TILE_SIZE);
                gameState->monkey.y = (float)(spawnRow * TILE_SIZE);
                gameState->monkey.frame = 0;
                gameState->monkey.animTimer = 0.0f;
                gameState->monkey.pauseTimer = 0.0f;
            }
        }

        if (gameState->monkey.active) {
            // Paso 1: detectar trigger
            if (!gameState->monkeyTriggered && !gameState->monkeyDrop.active) {
                int pColLeft = (int)(gameState->playerX / TILE_SIZE);
                int pColRight = (int)((gameState->playerX + TILE_SIZE - 1) / TILE_SIZE);
                int pRowTop = (int)(gameState->playerY / TILE_SIZE);
                int pRowBot = (int)((gameState->playerY + TILE_SIZE - 1) / TILE_SIZE);
                for (int r = pRowTop; r <= pRowBot && !gameState->monkeyTriggered; r++) {
                    for (int c = pColLeft; c <= pColRight && !gameState->monkeyTriggered; c++) {
                        if (r >= 0 && r < rows && c >= 0 && c < cols) {
                            int tile;
                            if (gameState->currentLevel == 0) tile = gameState->tileMap_1[r][c];
                            else if (gameState->currentLevel == 1) tile = gameState->tileMap_2[r][c];
                            else if (gameState->currentLevel == 2) tile = gameState->tileMap_3[r][c];
                            else if (gameState->currentLevel == 3) tile = gameState->tileMap_4[r][c];
                            else                                   tile = gameState->tileMap_5[r][c];
                            if (tile == TILE_MONKEY_TRIGGER || tile == 21)
                                gameState->monkeyTriggered = true;
                        }
                    }
                }
            }

            // Paso 2: animar el mono
            gameState->monkey.animTimer += dt;
            if (gameState->monkey.animTimer >= 0.12f) {
                gameState->monkey.animTimer = 0.0f;
                gameState->monkey.frame = (gameState->monkey.frame + 1) % MONKEY_FRAMES;
                // Paso 3: lanzar coco en frame 8 si trigger activo
                if (gameState->monkey.frame == 8 && !gameState->monkeyDrop.active && gameState->monkeyTriggered) {
                    gameState->monkeyDrop.active = true;
                    gameState->monkeyDrop.x = gameState->monkey.x - TILE_SIZE + (TILE_SIZE * 1.0f);
                    gameState->monkeyDrop.y = gameState->monkey.y + TILE_SIZE * 0.5f;
                    gameState->monkeyDrop.speed = 350.0f;
                    gameState->monkeyTriggered = false;
                }
            }
        }
    }

    // Coco pegado al mono cuando no cae
    if (!gameState->monkeyDrop.active) {
        gameState->monkeyDrop.x = gameState->monkey.x - TILE_SIZE + (TILE_SIZE * 1.0f);
        gameState->monkeyDrop.y = gameState->monkey.y + TILE_SIZE * 0.5f;
    }

    // Caida del coco
    if (gameState->monkeyDrop.active) {
        gameState->monkeyDrop.y += gameState->monkeyDrop.speed * dt;
        float pL = (float)gameState->playerX + 4, pR = pL + TILE_SIZE - 8;
        float pT = (float)gameState->playerY + 4, pB = pT + TILE_SIZE - 8;
        float cL = gameState->monkeyDrop.x + 2, cR = cL + TILE_SIZE * 1.5f - 4;
        float cT = gameState->monkeyDrop.y + 2, cB = cT + TILE_SIZE * 1.5f - 4;
        if (!gameState->playerDead && cL < pR && cR > pL && cT < pB && cB > pT) {
            gameState->playerDead = 1;
            gameState->monkeyDrop.active = false;
        }
        if (gameState->monkeyDrop.y > (float)(gameState->currentMapRows * TILE_SIZE))
            gameState->monkeyDrop.active = false;
    }

    // --- RECOLECCION DE OBJETOS ---
    CollectTileUnderPlayer(gameState);

    // --- ANIMACIONES Y TRAIL ---
    int playerIsMoving = (gameState->velocityX != 0 || gameState->velocityY != 0);
    if (playerIsMoving) {
        gameState->playerAnimFrame = 0;
        gameState->playerAnimTimer = 0;
        for (int i = TRAIL_LENGTH - 1; i > 0; i--) gameState->trailPositions[i] = gameState->trailPositions[i - 1];
        gameState->trailPositions[0] = { (float)gameState->playerX, (float)gameState->playerY };
    }
    else {
        if (++gameState->playerAnimTimer >= playerAnimSpeed) { gameState->playerAnimTimer = 0; gameState->playerAnimFrame = (gameState->playerAnimFrame + 1) % PLAYER_ANIM_SEQ_LEN; }
        for (int i = 0; i < TRAIL_LENGTH; i++) gameState->trailPositions[i] = { (float)gameState->playerX, (float)gameState->playerY };
    }
    if (++gameState->coinAnimTimer >= coinAnimSpeed) { gameState->coinAnimTimer = 0; gameState->coinAnimFrame = (gameState->coinAnimFrame + 1) % COIN_ANIM_SEQ_LEN; }
    if (++gameState->batFrameCounter >= batAnimSpeed) { gameState->batFrameCounter = 0; gameState->batCurrentFrames = (gameState->batCurrentFrames + 1) % BAT_ANIM_SEQ_LEN; }

    return SCENE_GAME;
}

//  GAMEDRAW - Helpers
static void DrawPanel(int panelW, int panelH) {
    int px = (SCREEN_WIDTH - panelW) / 2, py = (SCREEN_HEIGHT - panelH) / 2;
    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, ColorAlpha(BLACK, 0.82f));
    DrawRectangle(px, py, panelW, panelH, Color{ 255,220,0,255 });
    DrawRectangleLines(px, py, panelW, panelH, Color{ 80,60,0,255 });
    DrawRectangleLines(px + 3, py + 3, panelW - 6, panelH - 6, Color{ 255,255,120,255 });
}
static void DrawPanelTitle(const char* text, int panelW, int panelH, int fontSize) {
    int px = (SCREEN_WIDTH - panelW) / 2, py = (SCREEN_HEIGHT - panelH) / 2;
    DrawText(text, px + (panelW - MeasureText(text, fontSize)) / 2, py + 16, fontSize, Color{ 30,20,0,255 });
    DrawLine(px + 30, py + 16 + fontSize + 6, px + panelW - 30, py + 16 + fontSize + 6, Color{ 100,80,0,160 });
}

static void DrawMenuPanel(GameState* gs) {
    const int W = 340, H = 310; DrawPanel(W, H);
    int px = (SCREEN_WIDTH - W) / 2, py = (SCREEN_HEIGHT - H) / 2;
    DrawPanelTitle("PAUSA", W, H, 26);
    const char* lbs[4] = { "REINICIAR","","VOLUMEN","SALIR AL MAPA" };
    for (int i = 0; i < 4; i++) {
        int oy = py + 70 + i * 54; bool sel = (gs->menuOption == i);
        if (sel) { DrawRectangle(px + 16, oy - 4, W - 32, 40, Color{ 255,245,120,255 }); DrawRectangleLines(px + 16, oy - 4, W - 32, 40, Color{ 80,60,0,255 }); }
        const char* lb = lbs[i]; if (i == 1) lb = gs->musicEnabled ? "SFX: ON" : "SFX: OFF";
        DrawText(lb, px + (W - MeasureText(lb, 20)) / 2, oy + 6, 20, Color{ 30,20,0,255 });
        if (i == 2) { int sx = px + 30, sy = oy + 32, sw = W - 60; DrawRectangle(sx, sy, sw, 8, Color{ 120,95,0,255 }); DrawRectangle(sx, sy, (int)(gs->masterVolume * sw), 8, Color{ 255,200,0,255 }); int hx = sx + (int)(gs->masterVolume * (sw - 10)); DrawRectangle(hx, sy - 4, 10, 16, sel ? WHITE : Color{ 220,190,80,255 }); DrawRectangleLines(hx, sy - 4, 10, 16, Color{ 80,60,0,255 }); DrawText(TextFormat("%d%%", (int)(gs->masterVolume * 100)), sx + sw + 8, sy - 2, 14, Color{ 60,45,0,255 }); }
    }
    const char* h = "ARRIBA/ABAJO: navegar    ENTER: confirmar";
    DrawText(h, px + (W - MeasureText(h, 11)) / 2, py + H - 18, 11, Color{ 100,80,0,200 });
}

static void DrawVictoryPanel(GameState* gs) {
    const int W = 400, H = 320; DrawPanel(W, H);
    int px = (SCREEN_WIDTH - W) / 2, py = (SCREEN_HEIGHT - H) / 2;
    DrawPanelTitle("NIVEL COMPLETADO!", W, H, 24);
    int ssz = 44, gap = 10, tot = gs->starsTotal > 0 ? gs->starsTotal : 3;
    int sx = px + (W - (tot * ssz + (tot - 1) * gap)) / 2;
    for (int i = 0; i < tot; i++) { Texture2D& st = (i < gs->starsCollected) ? gs->starCollectedTexture : gs->starEmptyTexture; DrawTextureEx(st, { (float)(sx + i * (ssz + gap)),(float)(py + 68) }, 0, (float)ssz / (st.width > 0 ? st.width : ssz), WHITE); }
    int sc = (int)gs->timer, cc = (int)((gs->timer - sc) * 100);
    const char* ss = TextFormat("PUNTUACION: %d", gs->score); const char* ts = TextFormat("TIEMPO:     %d.%02d", sc, cc);
    DrawText(ss, px + (W - MeasureText(ss, 19)) / 2, py + 130, 19, Color{ 30,20,0,255 });
    DrawText(ts, px + (W - MeasureText(ts, 19)) / 2, py + 156, 19, Color{ 30,20,0,255 });
    DrawLine(px + 30, py + 186, px + W - 30, py + 186, Color{ 100,80,0,150 });
    struct { const char* k; const char* d; }opts[] = { "[L]","Guardar puntuacion","[ESPACIO]","Reintentar","[M]","Volver al mapa" };
    for (int i = 0; i < 3; i++) { int oy = py + 196 + i * 34; DrawText(opts[i].k, px + 40, oy, 17, Color{ 80,60,0,255 }); DrawText(opts[i].d, px + 40 + MeasureText(opts[i].k, 17) + 10, oy, 17, Color{ 30,20,0,255 }); }
}

static void DrawInitialsPanel(GameState* gs) {
    const int W = 380, H = 270; DrawPanel(W, H);
    int px = (SCREEN_WIDTH - W) / 2, py = (SCREEN_HEIGHT - H) / 2;
    DrawPanelTitle("INTRODUCE TUS INICIALES", W, H, 19);
    int bW = 62, bH = 72, bx = px + (W - (3 * bW + 2 * 16)) / 2, by = py + 72;
    for (int i = 0; i < 3; i++) {
        int lx = bx + i * (bW + 16); bool act = (gs->initialIndex == i);
        DrawRectangle(lx, by, bW, bH, act ? Color{ 255,255,120,255 } : Color{ 210,175,0,255 });
        DrawRectangleLines(lx, by, bW, bH, Color{ 80,60,0,255 });
        if (act) DrawRectangleLines(lx - 2, by - 2, bW + 4, bH + 4, Color{ 255,255,0,255 });
        char l[2] = { gs->initials[i],'\0' };
        DrawText(l, lx + (bW - MeasureText(l, 42)) / 2, by + 12, 42, Color{ 30,20,0,255 });
        if (act) DrawText("v", lx + (bW - MeasureText("v", 16)) / 2, by + bH + 4, 16, Color{ 30,20,0,200 });
    }
    const char* h1 = "ARRIBA/ABAJO: letra    DERECHA/ENTER: siguiente";
    const char* h2 = "IZQUIERDA: anterior    BACKSPACE: cancelar";
    DrawText(h1, px + (W - MeasureText(h1, 12)) / 2, py + 175, 12, Color{ 80,60,0,220 });
    DrawText(h2, px + (W - MeasureText(h2, 12)) / 2, py + 193, 12, Color{ 80,60,0,220 });
}

static void DrawLeaderboardPanel(GameState* gs) {
    const int W = 440, H = 440; DrawPanel(W, H);
    int px = (SCREEN_WIDTH - W) / 2, py = (SCREEN_HEIGHT - H) / 2;
    DrawPanelTitle("LEADERBOARD", W, H, 26);
    int hy = py + 56;
    DrawText("POS", px + 18, hy, 13, Color{ 60,45,0,255 }); DrawText("NOMBRE", px + 60, hy, 13, Color{ 60,45,0,255 });
    DrawText("TIEMPO", px + 170, hy, 13, Color{ 60,45,0,255 }); DrawText("SCORE", px + 265, hy, 13, Color{ 60,45,0,255 }); DrawText("ESTRS", px + 365, hy, 13, Color{ 60,45,0,255 });
    DrawLine(px + 14, hy + 17, px + W - 14, hy + 17, Color{ 100,80,0,180 });
    if (gs->leaderboardCount == 0) { const char* e = "Todavia no hay entradas"; DrawText(e, px + (W - MeasureText(e, 16)) / 2, py + 200, 16, Color{ 80,60,0,200 }); }
    for (int i = 0; i < gs->leaderboardCount; i++) {
        LeaderboardEntry& e = gs->leaderboard[i]; int ry = hy + 26 + i * 30;
        DrawRectangle(px + 14, ry - 2, W - 28, 26, (i % 2 == 0) ? Color{ 255,235,80,60 } : Color{ 0,0,0,0 });
        int s2 = (int)e.time, c2 = (int)((e.time - s2) * 100); Color rc = (i == 0) ? Color{ 30,20,0,255 } : Color{ 60,45,0,220 };
        DrawText(TextFormat("#%d", i + 1), px + 18, ry, 14, rc); DrawText(e.initials, px + 60, ry, 14, rc);
        DrawText(TextFormat("%d.%02d", s2, c2), px + 170, ry, 14, rc); DrawText(TextFormat("%d", e.score), px + 265, ry, 14, rc);
        for (int s = 0; s < 3; s++) DrawText("*", px + 365 + s * 18, ry, 14, (s < e.stars) ? Color{ 255,200,0,255 } : Color{ 120,95,0,180 });
    }
    const char* h = "[L] Opciones    [M] Mapa    [ESPACIO] Reintentar";
    DrawText(h, px + (W - MeasureText(h, 11)) / 2, py + H - 18, 11, Color{ 80,60,0,200 });
}

static void DrawDeathPanel(GameState* gs) {
    const int W = 340, H = 260; DrawPanel(W, H);
    int px = (SCREEN_WIDTH - W) / 2, py = (SCREEN_HEIGHT - H) / 2;
    DrawPanelTitle("HAS MUERTO", W, H, 26);
    const char* sub = "Mejor suerte la proxima vez...";
    DrawText(sub, px + (W - MeasureText(sub, 14)) / 2, py + 58, 14, Color{ 60, 45, 0, 220 });
    DrawLine(px + 30, py + 80, px + W - 30, py + 80, Color{ 100, 80, 0, 150 });
    struct { const char* k; const char* d; } opts[] = { { "[ESPACIO]", "Reintentar" }, { "[M]", "Volver al mapa" } };
    for (int i = 0; i < 2; i++) {
        int oy = py + 100 + i * 50;
        DrawRectangle(px + 16, oy - 6, W - 32, 40, Color{ 255, 245, 120, 80 });
        DrawRectangleLines(px + 16, oy - 6, W - 32, 40, Color{ 80, 60, 0, 120 });
        DrawText(opts[i].k, px + 50, oy + 6, 18, Color{ 80, 60, 0, 255 });
        DrawText(opts[i].d, px + 50 + MeasureText(opts[i].k, 18) + 12, oy + 6, 18, Color{ 30, 20, 0, 255 });
    }
    const char* h = "ESPACIO: reintentar    M: mapa";
    DrawText(h, px + (W - MeasureText(h, 11)) / 2, py + H - 18, 11, Color{ 100, 80, 0, 200 });
}

//  GAMEDRAW - Principal
void GameDraw(GameState* gameState) {
    ClearBackground(BLACK);

    int mapPixelWidth = gameState->currentMapCols * TILE_SIZE;
    int mapPixelHeight = gameState->currentMapRows * TILE_SIZE;
    float targetCX = (float)(gameState->playerX - SCREEN_WIDTH / 2 + TILE_SIZE / 2);
    float targetCY = (float)(gameState->playerY - SCREEN_HEIGHT / 2 + TILE_SIZE / 2);
    const float cs = 0.15f;
    gameState->cameraX += (targetCX - gameState->cameraX) * cs;
    gameState->cameraY += (targetCY - gameState->cameraY) * cs;

    if (gameState->cameraX < 0) gameState->cameraX = 0;
    if (gameState->cameraX > mapPixelWidth - SCREEN_WIDTH)  gameState->cameraX = (float)(mapPixelWidth - SCREEN_WIDTH);
    if (mapPixelWidth < SCREEN_WIDTH)  gameState->cameraX = -(float)(SCREEN_WIDTH - mapPixelWidth) / 2.0f;
    if (gameState->cameraY < 0) gameState->cameraY = 0;
    if (gameState->cameraY > mapPixelHeight - SCREEN_HEIGHT) gameState->cameraY = (float)(mapPixelHeight - SCREEN_HEIGHT);
    if (mapPixelHeight < SCREEN_HEIGHT) gameState->cameraY = -(float)(SCREEN_HEIGHT - mapPixelHeight) / 2.0f;

    int cameraX = (int)gameState->cameraX, cameraY = (int)gameState->cameraY;

    Color collectibleColor = (gameState->blinkTimer < 0.5f) ? Color{ 255,220,0,255 } : Color{ 180,0,220,255 };

    // --- MAPA ---
    for (int row = 0; row < gameState->currentMapRows; row++) {
        for (int col = 0; col < gameState->currentMapCols; col++) {
            int tt;
            if (gameState->currentLevel == 1) tt = gameState->tileMap_2[row][col];
            else if (gameState->currentLevel == 2) tt = gameState->tileMap_3[row][col];
            else if (gameState->currentLevel == 3) tt = gameState->tileMap_4[row][col];
            else if (gameState->currentLevel == 4) tt = gameState->tileMap_5[row][col];
            else                                   tt = gameState->tileMap_1[row][col];
            if (tt == TILE_EMPTY) continue;

            int sx = col * TILE_SIZE - cameraX, sy = row * TILE_SIZE - cameraY;
            if (sx < -TILE_SIZE || sx > SCREEN_WIDTH + TILE_SIZE || sy < -TILE_SIZE || sy > SCREEN_HEIGHT + TILE_SIZE) continue;

            Rectangle src = { 0, 0, (float)TILE_SIZE, (float)TILE_SIZE };
            Rectangle dst = { (float)sx, (float)sy, (float)TILE_SIZE, (float)TILE_SIZE };
            Vector2 orig = { 0, 0 };

            switch (tt) {
            case TILE_WALL: {
                int wv;
                if (gameState->currentLevel == 1) wv = GetWallVariant2(gameState->tileMap_2, row, col);
                else if (gameState->currentLevel == 2) wv = GetWallVariant3(gameState->tileMap_3, row, col);
                else if (gameState->currentLevel == 3) wv = GetWallVariant4(gameState->tileMap_4, row, col);
                else if (gameState->currentLevel == 4) wv = GetWallVariant5(gameState->tileMap_5, row, col);
                else                                   wv = GetWallVariant1(gameState->tileMap_1, row, col);
                DrawTexturePro(gameState->wallTextures[wv], src, dst, orig, 0, WHITE);
                break;
            }
            case TILE_DOT:
                DrawTexturePro(gameState->dotTexture, { 0,0,(float)gameState->dotTexture.width,(float)gameState->dotTexture.height }, dst, orig, 0, collectibleColor);
                break;
            case TILE_COIN: {
                Texture2D& ct = gameState->coinFrames[coinAnimSequence[gameState->coinAnimFrame]];
                DrawTexturePro(ct, { 0,0,(float)ct.width,(float)ct.height }, dst, orig, 0, WHITE);
                break;
            }
            case TILE_STAR:
                DrawTexturePro(gameState->starTexture, { 0,0,(float)gameState->starTexture.width,(float)gameState->starTexture.height }, dst, orig, 0, collectibleColor);
                break;
            case 8:
                DrawTexturePro(gameState->texRebote8, { 0,0,(float)gameState->texRebote8.width, (float)gameState->texRebote8.height }, dst, orig, 0, WHITE); break;
            case 9:
                DrawTexturePro(gameState->texRebote9, { 0,0,(float)gameState->texRebote9.width, (float)gameState->texRebote9.height }, dst, orig, 0, WHITE); break;
            case 10:
                DrawTexturePro(gameState->texRebote10, { 0,0,(float)gameState->texRebote10.width,(float)gameState->texRebote10.height }, dst, orig, 0, WHITE); break;
            case 11:
                DrawTexturePro(gameState->texRebote11, { 0,0,(float)gameState->texRebote11.width,(float)gameState->texRebote11.height }, dst, orig, 0, WHITE); break;
                // FIX (doc2): usar TILE_LEVEL_END en vez del literal 12
            case TILE_LEVEL_END:
                DrawTexturePro(gameState->levelEndTexture, { 0,0,(float)gameState->levelEndTexture.width,(float)gameState->levelEndTexture.height }, dst, orig, 0, WHITE);
                break;

            case TILE_BLOCK_RED: {
                bool solid = !gameState->blockToggle;
                Color c = solid ? WHITE : Color{ 255, 255, 255, 80 };
                DrawTexturePro(gameState->texBlockRed, { 0,0,(float)gameState->texBlockRed.width,(float)gameState->texBlockRed.height }, dst, orig, 0, c);
                break;
            }
            case TILE_BLOCK_BLUE: {
                bool solid = gameState->blockToggle;
                Color c = solid ? WHITE : Color{ 255, 255, 255, 80 };
                DrawTexturePro(gameState->texBlockBlue, { 0,0,(float)gameState->texBlockBlue.width,(float)gameState->texBlockBlue.height }, dst, orig, 0, c);
                break;
            }
            case TILE_MONKEY_TRIGGER:
                DrawTexture(gameState->dotTexture, sx, sy, WHITE);
                break;
            case TILE_MONKEY_SPAWN:
                break;
            case TILE_TOTEM_UP:
            case TILE_TOTEM_DOWN:
            case TILE_TOTEM_LEFT:
            case TILE_TOTEM_RIGHT: {
                float rot = 0.0f;
                if (tt == TILE_TOTEM_UP)    rot = 90.0f;
                if (tt == TILE_TOTEM_DOWN)  rot = 270.0f;
                if (tt == TILE_TOTEM_LEFT)  rot = 0.0f;
                if (tt == TILE_TOTEM_RIGHT) rot = 180.0f;
                Vector2 center = { (float)sx + TILE_SIZE / 2.0f, (float)sy + TILE_SIZE / 2.0f };
                DrawTexturePro(gameState->texTotem,
                    { 0, 0, (float)gameState->texTotem.width, (float)gameState->texTotem.height },
                    { center.x, center.y, (float)TILE_SIZE, (float)TILE_SIZE },
                    { TILE_SIZE / 2.0f, TILE_SIZE / 2.0f }, rot, WHITE);
                break;
            }
            case TILE_SPIKE_UP:
            case TILE_SPIKE_DOWN:
            case TILE_SPIKE_LEFT:
            case TILE_SPIKE_RIGHT: {
                int spikeIdx = -1;
                for (int i = 0; i < gameState->spikeCount; i++)
                    if (gameState->spikeCol[i] == col && gameState->spikeRow[i] == row) { spikeIdx = i; break; }
                int state = (spikeIdx >= 0) ? gameState->spikeState[spikeIdx] : 0;
                float stimer = (spikeIdx >= 0) ? gameState->spikeTimer[spikeIdx] : 0.0f;
                float rot = 0.0f;
                if (tt == TILE_SPIKE_DOWN)  rot = 180.0f;
                if (tt == TILE_SPIKE_LEFT)  rot = 270.0f;
                if (tt == TILE_SPIKE_RIGHT) rot = 90.0f;
                Color spkColor = WHITE;
                if (state == 0) spkColor = Color{ 100, 100, 100, 180 };
                else if (state == 1) { float b = sinf(stimer / 0.2f * 3.14159f * 6); spkColor = Color{ 255, (unsigned char)(200 + 55 * b), 0, 255 }; }
                else if (state == 3) spkColor = Color{ 255, 60, 60, 255 };
                Vector2 center = { (float)sx + TILE_SIZE / 2.0f, (float)sy + TILE_SIZE / 2.0f };
                DrawTexturePro(gameState->spikeTexture,
                    { 0, 0, (float)gameState->spikeTexture.width, (float)gameState->spikeTexture.height },
                    { center.x, center.y, (float)TILE_SIZE, (float)TILE_SIZE },
                    { TILE_SIZE / 2.0f, TILE_SIZE / 2.0f }, rot, spkColor);
                if (state >= 1 && spikeIdx >= 0) {
                    int dRow, dCol;
                    SpikeHazardOffset(tt, &dRow, &dCol);
                    int hx = (col + dCol) * TILE_SIZE - cameraX;
                    int hy = (row + dRow) * TILE_SIZE - cameraY;

                    if (state == 3) {
                        float b = fabsf(sinf(stimer * 3.14159f * 8));
                        Vector2 hCenter = { (float)hx + TILE_SIZE / 2.0f, (float)hy + TILE_SIZE / 2.0f };
                        Rectangle unfSrc = { 0, 0, (float)gameState->spikeUnfold.width, (float)gameState->spikeUnfold.height };
                        Rectangle unfDst = { hCenter.x, hCenter.y, (float)TILE_SIZE, (float)TILE_SIZE };
                        DrawTexturePro(gameState->spikeUnfold, unfSrc, unfDst,
                            { TILE_SIZE / 2.0f, TILE_SIZE / 2.0f }, rot,
                            Color{ 255, 255, 255, (unsigned char)(200 + 55 * b) });
                    }
                }
                break;
            }
            }
        }
    }

    // --- TRAIL ---
    if (gameState->velocityX != 0 || gameState->velocityY != 0) {
        Texture2D at = (gameState->velocityY != 0) ? gameState->trailVertical : gameState->trailHorizontal;
        for (int i = TRAIL_LENGTH - 1; i >= 0; i--) {
            float op = (float)(TRAIL_LENGTH - i) / (float)TRAIL_LENGTH;
            Color tc = { 255,255,255,(unsigned char)(255 * op) };
            Rectangle td = { gameState->trailPositions[i].x - cameraX, gameState->trailPositions[i].y - cameraY, (float)TILE_SIZE, (float)TILE_SIZE };
            DrawTexturePro(at, { 0,0,(float)at.width,(float)at.height }, td, { 0,0 }, 0.0f, tc);
        }
    }

    // --- MURCIELAGOS ---
    for (int i = 0; i < gameState->batCount; i++) {
        Bat& bat = gameState->bats[i];
        int bsx = (int)bat.x - cameraX, bsy = (int)bat.y - cameraY;
        if (bsx < -TILE_SIZE || bsx > SCREEN_WIDTH + TILE_SIZE || bsy < -TILE_SIZE || bsy > SCREEN_HEIGHT + TILE_SIZE) continue;
        Texture2D& bt = gameState->batTextures[batAnimSequence[gameState->batCurrentFrames]];
        DrawTexturePro(bt, { 0,0,(float)bt.width,(float)bt.height }, { (float)bsx,(float)bsy,(float)TILE_SIZE,(float)TILE_SIZE }, { 0,0 }, 0.0f, WHITE);
    }

    // --- FLECHAS ---
    for (int i = 0; i < gameState->arrowCount; i++) {
        Arrow& a = gameState->arrows[i];
        if (!a.active) continue;
        int ax = (int)a.x - cameraX, ay = (int)a.y - cameraY;
        if (ax < -TILE_SIZE || ax > SCREEN_WIDTH + TILE_SIZE || ay < -TILE_SIZE || ay > SCREEN_HEIGHT + TILE_SIZE) continue;
        Vector2 center = { (float)ax + TILE_SIZE / 2.0f, (float)ay + TILE_SIZE / 2.0f };
        DrawTexturePro(gameState->texArrow,
            { 0, 0, (float)gameState->texArrow.width, (float)gameState->texArrow.height },
            { center.x, center.y, (float)TILE_SIZE, (float)TILE_SIZE },
            { TILE_SIZE / 2.0f, TILE_SIZE / 2.0f }, a.rotation, WHITE);
    }

    // --- MONO ---
    if (gameState->monkey.active) {
        int frameIdx = gameState->monkey.frame % MONKEY_FRAMES;
        Texture2D& tex = gameState->texMonkeyFrames[frameIdx];
        DrawTexturePro(tex,
            { 0.0f, 0.0f, (float)tex.width, (float)tex.height },
            { (float)(gameState->monkey.x - cameraX - TILE_SIZE),
              (float)(gameState->monkey.y - cameraY - TILE_SIZE),
              (float)TILE_SIZE * 3.0f, (float)TILE_SIZE * 3.0f },
            { 0.0f, 0.0f }, 0.0f, WHITE);
    }

    // --- COCO ---
    {
        Texture2D& texCoco = gameState->texMonkeyDrop;
        float cocoScale = 0.8f;
        float cocoW = TILE_SIZE * 1.5f * cocoScale;
        float cocoH = TILE_SIZE * 1.5f * cocoScale;
        Rectangle srcCoco = { 0.0f, 0.0f, (float)texCoco.width, (float)texCoco.height };

        if (gameState->monkeyDrop.active) {
            Rectangle dst2 = { (float)(gameState->monkeyDrop.x - cameraX), (float)(gameState->monkeyDrop.y - cameraY), cocoW, cocoH };
            if (texCoco.id > 0) DrawTexturePro(texCoco, srcCoco, dst2, { 0,0 }, 0, WHITE);
            else DrawCircle((int)(dst2.x + cocoW / 2), (int)(dst2.y + cocoH / 2), TILE_SIZE * 0.75f * cocoScale, BROWN);
        }
        else if (gameState->monkey.active && !gameState->monkey.hasDropped) {
            float cx = gameState->monkey.x + (TILE_SIZE * 0.75f) - TILE_SIZE + (TILE_SIZE * 0.15f);
            float cy = gameState->monkey.y + (TILE_SIZE * 1.0f) - TILE_SIZE + (TILE_SIZE * 0.15f);
            Rectangle dst2 = { (float)(cx - cameraX), (float)(cy - cameraY), cocoW, cocoH };
            if (texCoco.id > 0) DrawTexturePro(texCoco, srcCoco, dst2, { 0,0 }, 0, WHITE);
            else DrawCircle((int)(dst2.x + cocoW / 2), (int)(dst2.y + cocoH / 2), TILE_SIZE * 0.75f * cocoScale, BROWN);
        }
    }

    // --- JUGADOR ---
    Texture2D cpf = gameState->playerFrames[playerAnimSequence[gameState->playerAnimFrame]];
    Rectangle pd = { (float)(gameState->playerX - cameraX) + TILE_SIZE / 2.0f, (float)(gameState->playerY - cameraY) + TILE_SIZE / 2.0f, (float)TILE_SIZE, (float)TILE_SIZE };
    DrawTexturePro(cpf, { 0,0,(float)cpf.width,(float)cpf.height }, pd, { TILE_SIZE / 2.0f, TILE_SIZE / 2.0f }, gameState->playerRotation + 180, WHITE);

    // --- HUD ---
    DrawRectangle(0, 0, SCREEN_WIDTH, 60, BLACK);
    DrawText(TextFormat("SCORE: %d", gameState->score), 10, 10, 20, WHITE);
    int sc = (int)gameState->timer, cc = (int)((gameState->timer - sc) * 100);
    DrawText(TextFormat("%d.%02d", sc, cc), 10, 32, 24, gameState->timerStarted ? WHITE : GRAY);

    int ssz = 32, stx = (SCREEN_WIDTH - (gameState->starsTotal * 48)) / 2;
    for (int i = 0; i < gameState->starsTotal; i++) {
        Texture2D& st = (i < gameState->starsCollected) ? gameState->starCollectedTexture : gameState->starEmptyTexture;
        DrawTextureEx(st, { (float)(stx + i * 48),10.0f }, 0, (float)ssz / (st.width > 0 ? st.width : ssz), WHITE);
    }

    if (!gameState->levelCompleted && !gameState->showingVictoryOptions && !gameState->enteringInitials && !gameState->showingLeaderboard && !gameState->playerDeadScreen) {
        DrawRectangle(SCREEN_WIDTH - 50, 15, 35, 35, YELLOW);
        DrawRectangle(SCREEN_WIDTH - 43, 21, 8, 23, BLACK);
        DrawRectangle(SCREEN_WIDTH - 28, 21, 8, 23, BLACK);
    }

    if (gameState->menuOpen)              DrawMenuPanel(gameState);
    if (gameState->showingVictoryOptions) DrawVictoryPanel(gameState);
    if (gameState->enteringInitials)      DrawInitialsPanel(gameState);
    if (gameState->showingLeaderboard)    DrawLeaderboardPanel(gameState);
    if (gameState->playerDeadScreen)      DrawDeathPanel(gameState);
}

//  GAMEUNLOAD
void GameUnload(GameState* gameState) {
    for (int i = 0; i < WALL_VARIANT_COUNT; i++) UnloadTexture(gameState->wallTextures[i]);
    UnloadTexture(gameState->dotTexture); UnloadTexture(gameState->coinTexture);
    UnloadTexture(gameState->starTexture); UnloadTexture(gameState->levelEndTexture);
    UnloadTexture(gameState->spikeTexture);
    UnloadTexture(gameState->texBlockRed);
    UnloadTexture(gameState->texBlockBlue);
    UnloadTexture(gameState->trailHorizontal); UnloadTexture(gameState->trailVertical);
    for (int i = 0; i < PLAYER_ANIM_FRAMES; i++) UnloadTexture(gameState->playerFrames[i]);
    UnloadTexture(gameState->starCollectedTexture); UnloadTexture(gameState->starEmptyTexture);
    for (int i = 0; i < 4; i++) UnloadTexture(gameState->batTextures[i]);
    UnloadTexture(gameState->texTotem);
    UnloadTexture(gameState->texArrow);
    for (int i = 0; i < MONKEY_FRAMES; i++) UnloadTexture(gameState->texMonkeyFrames[i]);
    UnloadTexture(gameState->texMonkeyDrop);
    UnloadSound(gameState->soundDash); UnloadSound(gameState->soundHitWall);
    UnloadSound(gameState->soundCollectDot); UnloadSound(gameState->soundCollectCoin);
    UnloadSound(gameState->soundCollectStar); UnloadSound(gameState->soundLevelComplete);
    UnloadSound(gameState->soundLevelStart);
}