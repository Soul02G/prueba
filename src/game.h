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
#define MONKEY_FRAMES         11

// --- DEFINICIONES DE TILES ---
#define TILE_EMPTY            0
#define TILE_WALL             1
#define TILE_PLAYER_SPAWN     2
#define TILE_DOT              3
#define TILE_COIN             4
#define TILE_STAR             5
#define TILE_HORIZONTAL_BAT   6
#define TILE_VERTICAL_BAT     7

// Rebotes
#define TILE_REBOTE_D_I       8
#define TILE_REBOTE_U_D       9
#define TILE_REBOTE_U_I       10
#define TILE_REBOTE_D_D       11

#define TILE_LEVEL_END        12

// Pinchos
#define TILE_SPIKE_UP         13
#define TILE_SPIKE_DOWN       14
#define TILE_SPIKE_LEFT       15
#define TILE_SPIKE_RIGHT      16

// Tótems
#define TILE_TOTEM_UP         17
#define TILE_TOTEM_DOWN       18
#define TILE_TOTEM_LEFT       19
#define TILE_TOTEM_RIGHT      20

// Mono
#define TILE_MONKEY_TRIGGER   21
#define TILE_MONKEY_SPAWN     22

// Bloques alternantes
#define TILE_BLOCK_RED        24   // empieza sólido
#define TILE_BLOCK_BLUE       25   // empieza intangible

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
#define MAP_COLUMNS_3         27
#define MAP_ROWS_3            40
#define MAP_COLUMNS_4         27
#define MAP_ROWS_4            40
#define MAP_COLUMNS_5         27
#define MAP_ROWS_5            40
#define LEADERBOARD_SIZE      10
#define MAX_BATS              20
#define MAX_SPIKES            64
#define MAX_TOTEMS            32
#define MAX_ARROWS            64
#define ARROW_SPEED           4.0f
#define TOTEM_FIRE_INTERVAL   1.0f

// --- ESTRUCTURAS ---

typedef struct {
    char initials[4];
    int score;
    int stars;
    float time;
} LeaderboardEntry;

typedef struct {
    Vector2 position;
    float velocityX;
    float velocityY;
    bool isHorizontal;
    bool deadly;
    float speed;
    int type;
    int direction;
    float stopTimer;
    int currentFrame;
    int frameCounter;
    bool active;
    float x, y;
} Bat;

// --- MONO ---

typedef enum {
    MONKEY_IDLE,
    MONKEY_WAITING_TO_SPAWN,
    MONKEY_DESCENDING,
    MONKEY_AT_SPAWN,
    MONKEY_ASCENDING,
} MonkeyState;

typedef struct {
    float x, y;
    float spawnX, spawnY;
    float startY;
    float targetY;
    MonkeyState state;
    int frame;
    float animTimer;
    float timer;
    float pauseTimer;
    bool active;
    bool hasDropped;
    int triggerID;
} Monkey;

typedef struct {
    float x, y;
    bool active;
    float speed;
} MonkeyDrop;

// --- FLECHAS ---

typedef struct {
    float x, y;
    float vx, vy;
    bool active;
    float rotation;
} Arrow;

// --- GAME STATE ---

typedef struct {

    // --- TEXTURAS ---

    Texture2D wallTextures[WALL_VARIANT_COUNT];
    Texture2D dotTexture;
    Texture2D coinTexture;
    Texture2D starTexture;
    Texture2D levelEndTexture;
    Texture2D spikeTexture;
    Texture2D spikeUnfold;
    Texture2D texBlockRed;
    Texture2D texBlockBlue;
    Texture2D trailHorizontal;
    Texture2D trailVertical;
    Texture2D playerFrames[PLAYER_ANIM_FRAMES];
    Texture2D coinFrames[COIN_ANIM_FRAMES];
    Texture2D starCollectedTexture;
    Texture2D starEmptyTexture;
    Texture2D batTextures[4];

    // Rebotes
    Texture2D texRebote8;
    Texture2D texRebote9;
    Texture2D texRebote10;
    Texture2D texRebote11;

    // Tótems
    Texture2D texTotem;
    Texture2D texArrow;

    // Mono
    Texture2D texMonkeyFrames[MONKEY_FRAMES];
    Texture2D texMonkeyDrop;

    // --- AUDIO ---

    Sound soundDash;
    Sound soundHitWall;
    Sound soundCollectDot;
    Sound soundCollectCoin;
    Sound soundCollectStar;
    Sound soundLevelComplete;
    Sound soundLevelStart;

    // --- ESTADO DEL JUGADOR ---

    float playerX, playerY;
    int velocityX, velocityY;
    float playerRotation;
    int playerAnimFrame;
    float playerAnimTimer;
    int coinAnimFrame;
    float coinAnimTimer;
    int batFrameCounter;
    int batCurrentFrames;
    Vector2 trailPositions[TRAIL_LENGTH];
    bool playerDead;
    bool playerDeadScreen;

    // --- MAPAS Y NIVELES ---

    int currentLevel;
    int currentMapRows;
    int currentMapCols;
    int tileMap_1[MAP_ROWS_1][MAP_COLUMNS_1];
    int tileMap_2[MAP_ROWS_2][MAP_COLUMNS_2];
    int tileMap_3[MAP_ROWS_3][MAP_COLUMNS_3];
    int tileMap_4[MAP_ROWS_4][MAP_COLUMNS_4];
    int tileMap_5[MAP_ROWS_5][MAP_COLUMNS_5];

    // --- GAMEPLAY Y UI ---

    int score;
    int coinsCollected;
    bool levelCompleted;
    float blinkTimer;
    int starsCollected;
    int starsTotal;
    int victoryStarsShown;
    float victoryStarTimer;
    float cameraX, cameraY;
    float timer;
    bool timerStarted;
    bool timerExpired;

    // --- LEADERBOARD Y MENÚS ---

    bool showingLeaderboard;
    bool enteringInitials;
    bool showingVictoryOptions;
    char initials[4];
    int initialIndex;
    int initialCharIndex[3];
    LeaderboardEntry leaderboard[LEADERBOARD_SIZE];
    int leaderboardCount;

    // --- ENEMIGOS Y SISTEMA ---

    Bat bats[MAX_BATS];
    int batCount;
    bool menuOpen;
    int menuOption;
    bool musicEnabled;
    float masterVolume;

    // Último rebote
    int lastBounceTileCol;
    int lastBounceTileRow;

    // --- SISTEMA DE PINCHOS ---
    int   spikeCount;
    int   spikeCol[MAX_SPIKES];
    int   spikeRow[MAX_SPIKES];
    int   spikeState[MAX_SPIKES];  // 0=off 1=tick 2=on 3=damage
    float spikeTimer[MAX_SPIKES];

    // --- SISTEMA DE TÓTEMS ---
    int   totemCount;
    int   totemCol[MAX_TOTEMS];
    int   totemRow[MAX_TOTEMS];
    int   totemDir[MAX_TOTEMS];    // 0=up 1=down 2=left 3=right
    float totemTimers[MAX_TOTEMS];

    // --- FLECHAS ---
    Arrow arrows[MAX_ARROWS];
    int arrowCount;

    // --- MONO Y COCO ---
    Monkey monkey;
    MonkeyDrop monkeyDrop;
    bool monkeyTriggered;

    // --- BLOQUES ALTERNANTES ---
    // false = rojo sólido / azul intangible
    // true  = rojo intangible / azul sólido
    bool blockToggle;

} GameState;

// --- PROTOTIPOS ---

void GameLoad(GameState* gameState);
SceneType GameUpdate(GameState* gameState, MapState* mapState);
void GameDraw(GameState* gameState);
void GameUnload(GameState* gameState);
void ResetGameState(GameState* gameState);

#endif