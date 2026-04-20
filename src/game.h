#ifndef GAME_H
#define GAME_H

#include "raylib.h"
#include "scenes.h"
#include "map.h"

// --- DEFINICIONES DE PANTALLA Y MOVIMIENTO ---
#define SCREEN_WIDTH          607
#define SCREEN_HEIGHT         800
#define PLAYER_MOVE_SPEED     9
#define TRAIL_LENGTH          15
#define TILE_SIZE             40

// --- CONSTANTES DE ANIMACIÓN Y ENEMIGOS ---
#define BAT_SPEED             1.0f
#define BAT_STOP_TIME         1.0f
#define PLAYER_ANIM_SEQ_LEN   8
#define COIN_ANIM_SEQ_LEN     4
#define PLAYER_ANIM_FRAMES    5
#define COIN_ANIM_FRAMES      4
#define BAT_ANIM_SEQ_LEN      6

// --- DEFINICIONES DE TILES ---
#define TILE_EMPTY            0
#define TILE_WALL             1
#define TILE_PLAYER_SPAWN     2
#define TILE_DOT              3
#define TILE_COIN             4
#define TILE_STAR             5
#define TILE_HORIZONTAL_BAT   6
#define TILE_VERTICAL_BAT     7
#define TILE_LEVEL_END        9

// --- VARIANTES DE PAREDES (AUTO-TILING) ---
#define WALL_SOLID            0
#define WALL_BORDER_TOP       1
#define WALL_BORDER_BOTTOM    2
#define WALL_BORDER_LEFT      3
#define WALL_BORDER_RIGHT     4
#define WALL_CORNER_TL        5
#define WALL_CORNER_TR        6
#define WALL_CORNER_BL        7
#define WALL_CORNER_BR        8
#define WALL_INNER_CORNER_TL  9
#define WALL_INNER_CORNER_TR  10
#define WALL_INNER_CORNER_BL  11
#define WALL_INNER_CORNER_BR  12
#define WALL_VARIANT_COUNT    13

// --- LÍMITES DE MAPA Y LEADERBOARD ---
#define MAP_COLUMNS_1         27
#define MAP_ROWS_1            40
#define MAP_COLUMNS_2         25
#define MAP_ROWS_2            47
#define LEADERBOARD_SIZE      10
#define MAX_BATS              20

// --- ESTRUCTURAS ---

typedef struct {
    char  initials[4];
    int   score;
    int   stars;
    float time;
} LeaderboardEntry;

typedef struct {
    Vector2 position;
    float velocityX;    // Añade esto
    float velocityY;    // Añade esto
    bool isHorizontal;  // Añade esto
    bool deadly;        // Añade esto
    float speed;
    int type;
    int direction;
    float stopTimer;
    int currentFrame;
    int frameCounter;
    bool active;
    float x, y;
} Bat;

typedef struct {
    // Texturas y Audio
    Texture2D wallTextures[WALL_VARIANT_COUNT];
    Texture2D dotTexture;
    Texture2D coinTexture;
    Texture2D starTexture;
    Texture2D levelEndTexture;
    Texture2D trailHorizontal;
    Texture2D trailVertical;
    Texture2D playerFrames[PLAYER_ANIM_FRAMES];
    Texture2D coinFrames[COIN_ANIM_FRAMES];
    Texture2D starCollectedTexture;
    Texture2D starEmptyTexture;
    Texture2D batTextures[4];

    Sound soundDash;
    Sound soundHitWall;
    Sound soundCollectDot;
    Sound soundCollectCoin;
    Sound soundCollectStar;
    Sound soundLevelComplete;
    Sound soundLevelStart;

    // Estado del Jugador y Animaciones
    float playerX, playerY;
    int   velocityX, velocityY;
    float playerRotation;
    int   playerAnimFrame;
    float playerAnimTimer;
    int   coinAnimFrame;
    float coinAnimTimer;
    int   batFrameCounter;
    int   batCurrentFrames;
    Vector2 trailPositions[TRAIL_LENGTH];
    bool  playerDead;

    // Gestión de Niveles y Mapas
    int   currentLevel;       // 0 = Nivel 1, 1 = Nivel 2
    int   currentMapRows;
    int   currentMapCols;

    // Los mapas se declaran solo aquí dentro para evitar error C2086
    int   tileMap_1[MAP_ROWS_1][MAP_COLUMNS_1];
    int   tileMap_2[MAP_ROWS_2][MAP_COLUMNS_2];

    // Lógica de Juego y UI
    int   score;
    bool  levelCompleted;
    float blinkTimer;
    int   starsCollected;
    int   starsTotal;
    int   victoryStarsShown;
    float victoryStarTimer;
    float cameraX, cameraY;
    float timer;
    bool  timerStarted;
    bool  timerExpired;

    // Leaderboard y Menús
    bool  showingLeaderboard;
    bool  enteringInitials;
    bool  showingVictoryOptions;
    char  initials[4];
    int   initialIndex;
    int   initialCharIndex[3];
    LeaderboardEntry leaderboard[LEADERBOARD_SIZE];
    int   leaderboardCount;

    // Enemigos y Sistema
    Bat   bats[MAX_BATS];
    int   batCount;
    bool  menuOpen;
    int   menuOption;
    bool  musicEnabled;
    float masterVolume;

} GameState;

void      GameLoad(GameState* gameState);
SceneType GameUpdate(GameState* gameState, MapState* mapState);
void      GameDraw(GameState* gameState);
void      GameUnload(GameState* gameState);
void      ResetGameState(GameState* gameState);

#endif // GAME_H