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
static void ResetGameState(GameState* gameState);
static void PlacePlayerAtSpawn(GameState* gameState);
static void InsertLeaderboard(GameState* gs);
static int GetWallVariant1(const int map[MAP_ROWS_1][MAP_COLUMNS_1], int row, int col);
static int GetWallVariant2(const int map[MAP_ROWS_2][MAP_COLUMNS_2], int row, int col);
static bool CheckWallCollision(GameState* gameState, float x, float y);

// --- 2. DATOS DE ANIMACION ---
static const int playerAnimSequence[PLAYER_ANIM_SEQ_LEN] = { 0, 1, 0, 2, 3, 2, 0, 4 };
static const int playerAnimSpeed = 10;
static const int coinAnimSequence[COIN_ANIM_SEQ_LEN] = { 0, 1, 2, 3 };
static const int coinAnimSpeed = 20;
static const int batAnimSequence[BAT_ANIM_SEQ_LEN] = { 0, 1, 2, 3, 2, 1 };
static const int batAnimSpeed = 10;
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
    {0,0,0,0,0,0,0,0,0,0,0,1,3,3,3,3,3,1,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,1,3,0,3,1,4,1,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,1,3,0,3,1,3,1,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,1,3,3,5,1,3,1,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,3,1,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,1,3,3,3,3,1,0,1,3,1,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,1,3,1,1,3,1,1,1,3,1,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,1,3,1,1,3,4,3,3,3,1,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,1,1,1,1,3,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0},
    {0,0,0,0,0,1,3,3,3,3,3,3,3,3,3,1,1,5,3,4,3,1,0,0,0,0,0},
    {0,0,0,0,0,1,3,6,0,4,1,1,1,1,3,3,3,3,0,0,3,1,0,0,0,0,0},
    {0,0,0,0,0,1,3,0,0,3,1,0,0,1,1,1,1,1,0,0,3,1,0,0,0,0,0},
    {0,0,0,0,0,1,3,3,3,3,1,0,0,0,1,0,0,0,0,0,3,1,0,0,0,0,0},
    {0,0,0,0,0,1,1,1,1,1,1,0,0,0,1,0,0,0,0,0,3,1,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,2,0,0,3,1,0,0,0,0,0},
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
    {0,0,0,0,0,0,0,1,0,3,3,3,3,1,3,3,3,3,3,0,0,0,0,3,1},
    {0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0,2,0,4,1},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
};

// --- FUNCIONES DE PAREDES NIVEL 1 ---
static int TileIsWall1(const int map[MAP_ROWS_1][MAP_COLUMNS_1], int row, int col) {
    if (row < 0 || row >= MAP_ROWS_1 || col < 0 || col >= MAP_COLUMNS_1) return 0;
    return (map[row][col] == TILE_WALL);
}

static int GetWallVariant1(const int map[MAP_ROWS_1][MAP_COLUMNS_1], int row, int col) {
    int U = TileIsWall1(map, row - 1, col);
    int D = TileIsWall1(map, row + 1, col);
    int L = TileIsWall1(map, row, col - 1);
    int R = TileIsWall1(map, row, col + 1);
    int UL = TileIsWall1(map, row - 1, col - 1);
    int UR = TileIsWall1(map, row - 1, col + 1);
    int DL = TileIsWall1(map, row + 1, col - 1);
    int DR = TileIsWall1(map, row + 1, col + 1);

    // Esquinas exteriores
    if (!U && !L) return WALL_CORNER_TL;
    if (!U && !R) return WALL_CORNER_TR;
    if (!D && !L) return WALL_CORNER_BL;
    if (!D && !R) return WALL_CORNER_BR;

    // Bordes
    if (!U) return WALL_BORDER_TOP;
    if (!D) return WALL_BORDER_BOTTOM;
    if (!L) return WALL_BORDER_LEFT;
    if (!R) return WALL_BORDER_RIGHT;

    // Esquinas interiores (rodeada por los 4 cardinales, falta una diagonal)
    if (!UL) return WALL_INNER_CORNER_TL;
    if (!UR) return WALL_INNER_CORNER_TR;
    if (!DL) return WALL_INNER_CORNER_BL;
    if (!DR) return WALL_INNER_CORNER_BR;

    return WALL_SOLID;
}

// --- FUNCIONES DE PAREDES NIVEL 2 ---
static int TileIsWall2(const int map[MAP_ROWS_2][MAP_COLUMNS_2], int row, int col) {
    if (row < 0 || row >= MAP_ROWS_2 || col < 0 || col >= MAP_COLUMNS_2) return 0;
    return (map[row][col] == TILE_WALL);
}

static int GetWallVariant2(const int map[MAP_ROWS_2][MAP_COLUMNS_2], int row, int col) {
    int hasWallAbove = TileIsWall2(map, row - 1, col);
    int hasWallBelow = TileIsWall2(map, row + 1, col);
    int hasWallLeft = TileIsWall2(map, row, col - 1);
    int hasWallRight = TileIsWall2(map, row, col + 1);
    int hasWallAboveLeft = TileIsWall2(map, row - 1, col - 1);
    int hasWallAboveRight = TileIsWall2(map, row - 1, col + 1);
    int hasWallBelowLeft = TileIsWall2(map, row + 1, col - 1);
    int hasWallBelowRight = TileIsWall2(map, row + 1, col + 1);

    if (!hasWallAbove && !hasWallLeft)  return WALL_CORNER_TL;
    if (!hasWallAbove && !hasWallRight) return WALL_CORNER_TR;
    if (!hasWallBelow && !hasWallLeft)  return WALL_CORNER_BL;
    if (!hasWallBelow && !hasWallRight) return WALL_CORNER_BR;
    if (!hasWallAbove) return WALL_BORDER_TOP;
    if (!hasWallBelow) return WALL_BORDER_BOTTOM;
    if (!hasWallLeft)  return WALL_BORDER_LEFT;
    if (!hasWallRight) return WALL_BORDER_RIGHT;
    int md = (!hasWallAboveLeft) + (!hasWallAboveRight) + (!hasWallBelowLeft) + (!hasWallBelowRight);
    if (md >= 1) {
        if (!hasWallAboveLeft)  return WALL_INNER_CORNER_TL;
        if (!hasWallAboveRight) return WALL_INNER_CORNER_TR;
        if (!hasWallBelowLeft)  return WALL_INNER_CORNER_BL;
        if (!hasWallBelowRight) return WALL_INNER_CORNER_BR;
    }
    return WALL_SOLID;
}

// --- CHECKWALLCOLLISION (a nivel de archivo, NO dentro de GameUpdate) ---
static bool CheckWallCollision(GameState* gameState, float x, float y) {
    int col = (int)(x / TILE_SIZE);
    int row = (int)(y / TILE_SIZE);

    // Nivel 2 (currentLevel es 1)
    if (gameState->currentLevel == 1) {
        if (row < 0 || row >= MAP_ROWS_2 || col < 0 || col >= MAP_COLUMNS_2) return true;
        return (gameState->tileMap_2[row][col] == TILE_WALL);
    }
    // Nivel 1 (currentLevel es 0)
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
    else {
        if (row >= 0 && row < MAP_ROWS_2 && col >= 0 && col < MAP_COLUMNS_2) {
            tileValue = gameState->tileMap_2[row][col];
            if (tileValue == TILE_DOT || tileValue == TILE_COIN || tileValue == TILE_STAR)
                gameState->tileMap_2[row][col] = TILE_EMPTY;
        }
    }

    switch (tileValue) {
    case TILE_DOT:       gameState->score += 10;  PlaySound(gameState->soundCollectDot);   break;
    case TILE_COIN:      gameState->score += 100; PlaySound(gameState->soundCollectCoin);  break;
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
    else {
        for (int row = 0; row < MAP_ROWS_2; row++)
            for (int col = 0; col < MAP_COLUMNS_2; col++)
                if (gameState->tileMap_2[row][col] == TILE_PLAYER_SPAWN) {
                    gameState->playerX = (float)(col * TILE_SIZE);
                    gameState->playerY = (float)(row * TILE_SIZE);
                    gameState->tileMap_2[row][col] = TILE_EMPTY;
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
    const char* text = u8"Hola mundo — UTF‑8";
    fwrite(&gs->leaderboardCount, sizeof(int), 1, f); fwrite(gs->leaderboard, sizeof(LeaderboardEntry), gs->leaderboardCount, f);
    fclose(f);
}

// --- RESET ---
static void ResetGameState(GameState* gameState) {
    gameState->playerAnimFrame = 0;
    gameState->playerAnimTimer = 0;
    gameState->velocityX = 0;
    gameState->velocityY = 0;
    gameState->playerRotation = 0.0f;
    gameState->score = 0;
    gameState->levelCompleted = 0;
    gameState->blinkTimer = 0.0f;
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
    gameState->menuOpen = false;
    gameState->lastBounceTileCol = -1;
    gameState->lastBounceTileRow = -1;

    // Restaurar mapa del nivel activo
    if (gameState->currentLevel == 0)
        memcpy(gameState->tileMap_1, initialMap, sizeof(initialMap));
    else
        memcpy(gameState->tileMap_2, LEVEL_2_DATA, sizeof(LEVEL_2_DATA));

    // Contar estrellas y spawnear murcielagos
    gameState->starsTotal = 0;
    gameState->batCount = 0;
    int rows = (gameState->currentLevel == 0) ? MAP_ROWS_1 : MAP_ROWS_2;
    int cols = (gameState->currentLevel == 0) ? MAP_COLUMNS_1 : MAP_COLUMNS_2;

    for (int row = 0; row < rows; row++) {
        for (int col = 0; col < cols; col++) {
            int tile = (gameState->currentLevel == 0) ? gameState->tileMap_1[row][col] : gameState->tileMap_2[row][col];

            if (tile == TILE_STAR) {
                gameState->starsTotal++;
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
                else                              gameState->tileMap_2[row][col] = TILE_EMPTY;
            }
        }
    }

    PlacePlayerAtSpawn(gameState);
    for (int i = 0; i < TRAIL_LENGTH; i++)
        gameState->trailPositions[i] = { (float)gameState->playerX, (float)gameState->playerY };
    gameState->cameraX = (float)(gameState->playerX - SCREEN_WIDTH / 2 + TILE_SIZE / 2);
    gameState->cameraY = (float)(gameState->playerY - SCREEN_HEIGHT / 2 + TILE_SIZE / 2);
}

// ============================================================
//  GAMELOAD
// ============================================================
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
    for (int i = 0; i < 4; i++) {
        gameState->batTextures[i] = LoadTexture(TextFormat("resources/bat-frame-%d.png", i + 1));
        SetTextureFilter(gameState->batTextures[i], TEXTURE_FILTER_POINT);
    }

    SetTextureFilter(gameState->dotTexture, TEXTURE_FILTER_POINT);
    SetTextureFilter(gameState->starTexture, TEXTURE_FILTER_POINT);
    SetTextureFilter(gameState->levelEndTexture, TEXTURE_FILTER_POINT);
    SetTextureFilter(gameState->trailHorizontal, TEXTURE_FILTER_POINT);
    SetTextureFilter(gameState->trailVertical, TEXTURE_FILTER_POINT);
    SetTextureFilter(gameState->starCollectedTexture, TEXTURE_FILTER_POINT);
    SetTextureFilter(gameState->starEmptyTexture, TEXTURE_FILTER_POINT);
    for (int i = 0; i < PLAYER_ANIM_FRAMES; i++) SetTextureFilter(gameState->playerFrames[i], TEXTURE_FILTER_POINT);
    for (int i = 0; i < COIN_ANIM_FRAMES; i++) SetTextureFilter(gameState->coinFrames[i], TEXTURE_FILTER_POINT);

    if (gameState->currentLevel == 1) {
        gameState->currentMapRows = MAP_ROWS_2;
        gameState->currentMapCols = MAP_COLUMNS_2;
        memcpy(gameState->tileMap_2, LEVEL_2_DATA, sizeof(LEVEL_2_DATA));
    }
    else {
        gameState->currentMapRows = MAP_ROWS_1;
        gameState->currentMapCols = MAP_COLUMNS_1;
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
    if (f) { fread(&gameState->leaderboardCount, sizeof(int), 1, f); fread(gameState->leaderboard, sizeof(LeaderboardEntry), gameState->leaderboardCount, f); fclose(f); }

    gameState->batFrameCounter = 0;
    gameState->batCurrentFrames = 0;

    ResetGameState(gameState);
    PlaySound(gameState->soundLevelStart);
}

void HandleBounceCollision(GameState* gameState) {
    // 1. Usamos un margen pequeño (0.1f) para evitar errores de redondeo en los bordes
    int tileCol = (int)((gameState->playerX + TILE_SIZE / 2) / TILE_SIZE);
    int tileRow = (int)((gameState->playerY + TILE_SIZE / 2) / TILE_SIZE);

    if (tileRow < 0 || tileCol < 0) return;

    int currentTile = (gameState->currentLevel == 0) ?
        gameState->tileMap_1[tileRow][tileCol] :
        gameState->tileMap_2[tileRow][tileCol];

    // Solo entramos si es un tile de rebote (8 al 11)
    if (currentTile >= 8 && currentTile <= 11) {
        int nextVX = 0;
        int nextVY = 0;
        bool activated = false;

        // --- LÓGICA DE ESPEJOS SEGÚN ANCLAJES (Esquina Ocupada) ---
        switch (currentTile) {
        case 8: // ANCLAJE D-I
            if (gameState->velocityX < 0) { nextVY = -PLAYER_MOVE_SPEED; activated = true; }
            else if (gameState->velocityY > 0) { nextVX = PLAYER_MOVE_SPEED;  activated = true; }
            break;
        case 9: // ANCLAJE U-D
            if (gameState->velocityX > 0) { nextVY = PLAYER_MOVE_SPEED;  activated = true; }
            else if (gameState->velocityY < 0) { nextVX = -PLAYER_MOVE_SPEED; activated = true; }
            break;
        case 10: // ANCLAJE U-I
            if (gameState->velocityX < 0) { nextVY = PLAYER_MOVE_SPEED;  activated = true; }
            else if (gameState->velocityY < 0) { nextVX = PLAYER_MOVE_SPEED;  activated = true; }
            break;
        case 11: // ANCLAJE D-D
            if (gameState->velocityX > 0) { nextVY = -PLAYER_MOVE_SPEED; activated = true; }
            else if (gameState->velocityY > 0) { nextVX = -PLAYER_MOVE_SPEED; activated = true; }
            break;
        }

        if (activated) {
            // Aplicamos la dirección
            gameState->velocityX = nextVX;
            gameState->velocityY = nextVY;

            // --- EL CAMBIO CLAVE ---
            // En lugar de teletransportar UNA casilla entera (que puede dar en un muro),
            // centramos al jugador en el tile actual y lo empujamos 2 píxeles.
            gameState->playerX = (float)(tileCol * TILE_SIZE);
            gameState->playerY = (float)(tileRow * TILE_SIZE);

            // Este pequeño empujón saca al jugador del centro para que el siguiente frame
            // detecte el movimiento, pero no es tan grande como para atravesar muros.
            float push = 2.0f;
            if (gameState->velocityX > 0)      gameState->playerX += push;
            else if (gameState->velocityX < 0) gameState->playerX -= push;
            if (gameState->velocityY > 0)      gameState->playerY += push;
            else if (gameState->velocityY < 0) gameState->playerY -= push;

            // Actualizar rotación y sonido
            if (gameState->velocityX > 0)      gameState->playerRotation = 90;
            else if (gameState->velocityX < 0) gameState->playerRotation = 270;
            else if (gameState->velocityY > 0) gameState->playerRotation = 180;
            else if (gameState->velocityY < 0) gameState->playerRotation = 0;

            PlaySound(gameState->soundDash);
        }
    }
}

// ============================================================
//  GAMEUPDATE
// ============================================================
SceneType GameUpdate(GameState* gameState, MapState* mapState) {

    if (IsKeyPressed(KEY_M) && !gameState->levelCompleted &&
        !gameState->showingVictoryOptions && !gameState->enteringInitials && !gameState->showingLeaderboard)
        gameState->menuOpen = !gameState->menuOpen;

    if (gameState->menuOpen) {
        if (IsKeyPressed(KEY_UP))   gameState->menuOption = (gameState->menuOption - 1 + 4) % 4;
        if (IsKeyPressed(KEY_DOWN)) gameState->menuOption = (gameState->menuOption + 1) % 4;
        if (gameState->menuOption == 0) { if (IsKeyPressed(KEY_ENTER)) { ResetGameState(gameState); gameState->menuOpen = false; } }
        else if (gameState->menuOption == 1) {
            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_RIGHT))
            {
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
        if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) { gameState->velocityX = PLAYER_MOVE_SPEED;  gameState->playerRotation = 90;  gameState->timerStarted = 1; PlaySound(gameState->soundDash); }
        else if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A)) { gameState->velocityX = -PLAYER_MOVE_SPEED; gameState->playerRotation = 270; gameState->timerStarted = 1; PlaySound(gameState->soundDash); }
        else if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) { gameState->velocityY = PLAYER_MOVE_SPEED;  gameState->playerRotation = 180; gameState->timerStarted = 1; PlaySound(gameState->soundDash); }
        else if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) { gameState->velocityY = -PLAYER_MOVE_SPEED; gameState->playerRotation = 0;   gameState->timerStarted = 1; PlaySound(gameState->soundDash); }
    }

    // --- MOVIMIENTO X ---
    if (gameState->velocityX != 0) {
        int nextX = gameState->playerX + gameState->velocityX;
        int tileRow = gameState->playerY / TILE_SIZE;
        int nextTileCol = (gameState->velocityX > 0) ? (nextX + TILE_SIZE - 1) / TILE_SIZE : nextX / TILE_SIZE;
        if (gameState->currentLevel == 0) {
            nextTileCol = (nextTileCol < 0) ? 0 : (nextTileCol >= MAP_COLUMNS_1 ? MAP_COLUMNS_1 - 1 : nextTileCol);
            if (gameState->tileMap_1[tileRow][nextTileCol] == TILE_WALL) { gameState->playerX = (gameState->velocityX > 0) ? (nextTileCol * TILE_SIZE) - TILE_SIZE : (nextTileCol + 1) * TILE_SIZE; gameState->velocityX = 0; PlaySound(gameState->soundHitWall); }
            else if (nextX < 0 || nextX + TILE_SIZE > MAP_COLUMNS_1 * TILE_SIZE) { gameState->velocityX = 0; PlaySound(gameState->soundHitWall); }
            else gameState->playerX = nextX;
        }
        else {
            nextTileCol = (nextTileCol < 0) ? 0 : (nextTileCol >= MAP_COLUMNS_2 ? MAP_COLUMNS_2 - 1 : nextTileCol);
            if (gameState->tileMap_2[tileRow][nextTileCol] == TILE_WALL) { gameState->playerX = (gameState->velocityX > 0) ? (nextTileCol * TILE_SIZE) - TILE_SIZE : (nextTileCol + 1) * TILE_SIZE; gameState->velocityX = 0; PlaySound(gameState->soundHitWall); }
            else if (nextX < 0 || nextX + TILE_SIZE > MAP_COLUMNS_2 * TILE_SIZE) { gameState->velocityX = 0; PlaySound(gameState->soundHitWall); }
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
            if (gameState->tileMap_1[nextTileRow][tileCol] == TILE_WALL) { gameState->playerY = (gameState->velocityY > 0) ? (nextTileRow * TILE_SIZE) - TILE_SIZE : (nextTileRow + 1) * TILE_SIZE; gameState->velocityY = 0; PlaySound(gameState->soundHitWall); }
            else if (nextY < 0 || nextY + TILE_SIZE > MAP_ROWS_1 * TILE_SIZE) { gameState->velocityY = 0; PlaySound(gameState->soundHitWall); }
            else gameState->playerY = nextY;
        }
        else {
            nextTileRow = (nextTileRow < 0) ? 0 : (nextTileRow >= MAP_ROWS_2 ? MAP_ROWS_2 - 1 : nextTileRow);
            if (gameState->tileMap_2[nextTileRow][tileCol] == TILE_WALL) { gameState->playerY = (gameState->velocityY > 0) ? (nextTileRow * TILE_SIZE) - TILE_SIZE : (nextTileRow + 1) * TILE_SIZE; gameState->velocityY = 0; PlaySound(gameState->soundHitWall); }
            else if (nextY < 0 || nextY + TILE_SIZE > MAP_ROWS_2 * TILE_SIZE) { gameState->velocityY = 0; PlaySound(gameState->soundHitWall); }
            else gameState->playerY = nextY;
        }
    }

    if (gameState->velocityX != 0 || gameState->velocityY != 0) {
        HandleBounceCollision(gameState);
    }

    // --- MURCIELAGOS ---
    for (int i = 0; i < gameState->batCount; i++) {
        Bat& bat = gameState->bats[i];
        if (bat.stopTimer > 0.0f) {
            bat.stopTimer -= dt;
            continue;
        }
        float nextX = bat.x + bat.velocityX;
        float nextY = bat.y + bat.velocityY;
        float checkX = (bat.velocityX > 0) ? (nextX + TILE_SIZE - 1) : (bat.velocityX < 0 ? nextX : bat.x + 5);
        float checkY = (bat.velocityY > 0) ? (nextY + TILE_SIZE - 1) : (bat.velocityY < 0 ? nextY : bat.y + 5);

        if (CheckWallCollision(gameState, checkX, checkY)) {
            bat.stopTimer = BAT_STOP_TIME;
            bat.velocityX *= -1;
            bat.velocityY *= -1;
        }
        else {
            bat.x = nextX;
            bat.y = nextY;
        }
    }

    // --- COLISIONES CON ENEMIGOS ---
    if (!gameState->playerDead && !gameState->levelCompleted) {
        float pL = (float)gameState->playerX + 4;
        float pR = pL + TILE_SIZE - 8;
        float pT = (float)gameState->playerY + 4;
        float pB = pT + TILE_SIZE - 8;

        for (int i = 0; i < gameState->batCount; i++) {
            Bat& bat = gameState->bats[i];
            float bL = bat.x + 4;
            float bR = bL + TILE_SIZE - 8;
            float bT = bat.y + 4;
            float bB = bT + TILE_SIZE - 8;

            if (pL < bR && pR > bL && pT < bB && pB > bT) {
                gameState->playerDead = 1;
                break;
            }
        }
    }

    if (gameState->playerDead) {
        PlaySound(gameState->soundHitWall);
        ResetGameState(gameState);
        return SCENE_GAME;
    }

    // --- RECOLECCIÓN DE OBJETOS ---
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
// ============================================================
//  GAMEDRAW - Helpers
// ============================================================
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

// ============================================================
//  GAMEDRAW - Principal
// ============================================================
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

    // MAPA
    for (int row = 0; row < gameState->currentMapRows; row++) {
        for (int col = 0; col < gameState->currentMapCols; col++) {
            int tt = (gameState->currentLevel == 1) ? gameState->tileMap_2[row][col] : gameState->tileMap_1[row][col];
            if (tt == TILE_EMPTY) continue;

            int sx = col * TILE_SIZE - cameraX, sy = row * TILE_SIZE - cameraY;
            if (sx < -TILE_SIZE || sx > SCREEN_WIDTH + TILE_SIZE || sy < -TILE_SIZE || sy > SCREEN_HEIGHT + TILE_SIZE) continue;

            Rectangle src = { 0, 0, (float)TILE_SIZE, (float)TILE_SIZE };
            Rectangle dst = { (float)sx, (float)sy, (float)TILE_SIZE, (float)TILE_SIZE };
            Vector2 orig = { 0, 0 };

            switch (tt) {
            case TILE_WALL: {
                int wv = (gameState->currentLevel == 1) ? GetWallVariant2(gameState->tileMap_2, row, col) : GetWallVariant1(gameState->tileMap_1, row, col);
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

                // --- REBOTES ---
            case 8:
                DrawTexturePro(gameState->texRebote8, { 0,0,(float)gameState->texRebote8.width,(float)gameState->texRebote8.height }, dst, orig, 0, WHITE);
                break;
            case 9:
                DrawTexturePro(gameState->texRebote9, { 0,0,(float)gameState->texRebote9.width,(float)gameState->texRebote9.height }, dst, orig, 0, WHITE);
                break;
            case 10:
                DrawTexturePro(gameState->texRebote10, { 0,0,(float)gameState->texRebote10.width,(float)gameState->texRebote10.height }, dst, orig, 0, WHITE);
                break;
            case 11:
                DrawTexturePro(gameState->texRebote11, { 0,0,(float)gameState->texRebote11.width,(float)gameState->texRebote11.height }, dst, orig, 0, WHITE);
                break;

                // --- META (USA SOLO UNO) ---
            case 12:
                DrawTexturePro(gameState->levelEndTexture, { 0,0,(float)gameState->levelEndTexture.width,(float)gameState->levelEndTexture.height }, dst, orig, 0, WHITE);
                break;
            }
        }
    }

    // TRAIL (Rastro)
    if (gameState->velocityX != 0 || gameState->velocityY != 0) {
        Texture2D at = (gameState->velocityY != 0) ? gameState->trailVertical : gameState->trailHorizontal;
        for (int i = TRAIL_LENGTH - 1; i >= 0; i--) {
            float op = (float)(TRAIL_LENGTH - i) / (float)TRAIL_LENGTH;
            Color tc = { 255,255,255,(unsigned char)(255 * op) };
            Rectangle td = { gameState->trailPositions[i].x - cameraX, gameState->trailPositions[i].y - cameraY, (float)TILE_SIZE, (float)TILE_SIZE };
            DrawTexturePro(at, { 0,0,(float)at.width,(float)at.height }, td, { 0,0 }, 0.0f, tc);
        }
    }

    // MURCIELAGOS, JUGADOR Y HUD... (el resto sigue igual)
    // [Se mantiene el resto de tu código original para murciélagos, jugador y HUD]

    // MURCIELAGOS
    for (int i = 0; i < gameState->batCount; i++) {
        Bat& bat = gameState->bats[i];
        int bsx = (int)bat.x - cameraX, bsy = (int)bat.y - cameraY;
        if (bsx < -TILE_SIZE || bsx > SCREEN_WIDTH + TILE_SIZE || bsy < -TILE_SIZE || bsy > SCREEN_HEIGHT + TILE_SIZE) continue;
        Texture2D& bt = gameState->batTextures[batAnimSequence[gameState->batCurrentFrames]];
        DrawTexturePro(bt, { 0,0,(float)bt.width,(float)bt.height }, { (float)bsx,(float)bsy,(float)TILE_SIZE,(float)TILE_SIZE }, { 0,0 }, 0.0f, WHITE);
    }

    // JUGADOR
    Texture2D cpf = gameState->playerFrames[playerAnimSequence[gameState->playerAnimFrame]];
    Rectangle pd = { (float)(gameState->playerX - cameraX) + TILE_SIZE / 2.0f, (float)(gameState->playerY - cameraY) + TILE_SIZE / 2.0f, (float)TILE_SIZE, (float)TILE_SIZE };
    DrawTexturePro(cpf, { 0,0,(float)cpf.width,(float)cpf.height }, pd, { TILE_SIZE / 2.0f,TILE_SIZE / 2.0f }, gameState->playerRotation + 180, WHITE);

    // HUD Y PANELES
    DrawRectangle(0, 0, SCREEN_WIDTH, 60, BLACK);
    DrawText(TextFormat("SCORE: %d", gameState->score), 10, 10, 20, WHITE);
    int sc = (int)gameState->timer, cc = (int)((gameState->timer - sc) * 100);
    DrawText(TextFormat("%d.%02d", sc, cc), 10, 32, 24, gameState->timerStarted ? WHITE : GRAY);

    int ssz = 32, stx = (SCREEN_WIDTH - (gameState->starsTotal * 48)) / 2;
    for (int i = 0; i < gameState->starsTotal; i++) {
        Texture2D& st = (i < gameState->starsCollected) ? gameState->starCollectedTexture : gameState->starEmptyTexture;
        DrawTextureEx(st, { (float)(stx + i * 48),10.0f }, 0, (float)ssz / (st.width > 0 ? st.width : ssz), WHITE);
    }

    if (!gameState->levelCompleted && !gameState->showingVictoryOptions && !gameState->enteringInitials && !gameState->showingLeaderboard) {
        DrawRectangle(SCREEN_WIDTH - 50, 15, 35, 35, YELLOW);
        DrawRectangle(SCREEN_WIDTH - 43, 21, 8, 23, BLACK);
        DrawRectangle(SCREEN_WIDTH - 28, 21, 8, 23, BLACK);
    }

    if (gameState->menuOpen)              DrawMenuPanel(gameState);
    if (gameState->showingVictoryOptions) DrawVictoryPanel(gameState);
    if (gameState->enteringInitials)      DrawInitialsPanel(gameState);
    if (gameState->showingLeaderboard)    DrawLeaderboardPanel(gameState);
}

// ============================================================
//  GAMEUNLOAD
// ============================================================
void GameUnload(GameState* gameState) {
    for (int i = 0; i < WALL_VARIANT_COUNT; i++) UnloadTexture(gameState->wallTextures[i]);
    UnloadTexture(gameState->dotTexture); UnloadTexture(gameState->coinTexture);
    UnloadTexture(gameState->starTexture); UnloadTexture(gameState->levelEndTexture);
    UnloadTexture(gameState->trailHorizontal); UnloadTexture(gameState->trailVertical);
    for (int i = 0; i < PLAYER_ANIM_FRAMES; i++) UnloadTexture(gameState->playerFrames[i]);
    UnloadTexture(gameState->starCollectedTexture); UnloadTexture(gameState->starEmptyTexture);
    for (int i = 0; i < 4; i++) UnloadTexture(gameState->batTextures[i]);
    UnloadSound(gameState->soundDash); UnloadSound(gameState->soundHitWall);
    UnloadSound(gameState->soundCollectDot); UnloadSound(gameState->soundCollectCoin);
    UnloadSound(gameState->soundCollectStar); UnloadSound(gameState->soundLevelComplete);
    UnloadSound(gameState->soundLevelStart);
}