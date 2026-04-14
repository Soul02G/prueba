#pragma once
#include "raylib.h"
#include "scenes.h"
#define SCREEN_WIDTH          607
#define SCREEN_HEIGHT         800
#define PLAYER_MOVE_SPEED     9
#define TRAIL_LENGTH          15
#define TILE_SIZE             40
#define MAP_COLUMNS           17
#define MAP_ROWS              30
#define TILE_EMPTY            0
#define TILE_WALL             1
#define TILE_PLAYER_SPAWN     2
#define TILE_DOT              3
#define TILE_COIN             4
#define TILE_STAR             5
#define TILE_HORIZONTAL_BAT   6
#define TILE_VERTICAL_BAT     7
#define TILE_LEVEL_END        9
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
#define PLAYER_ANIM_FRAMES    5
#define PLAYER_ANIM_SEQ_LEN   8
#define LEADERBOARD_SIZE      10
#define TIMER_SECONDS         120
#define MAX_BATS        20
#define BAT_SPEED       1.0f
#define BAT_STOP_TIME   1.0f

typedef struct {
    char  initials[4];
    int   score;
    int   stars;
    float time;
} LeaderboardEntry;

typedef struct {
    float x;
    float y;
    int   isHorizontal;  // 1 = horizontal, 0 = vertical
    float velocityX;
    float velocityY;
    float stopTimer;     // Timer for 1 second pause when hitting wall
    int   deadly;// 1 = deadly, kills player on contact
} Bat;

typedef struct {
    Texture2D wallTextures[WALL_VARIANT_COUNT];
    Texture2D dotTexture;
    Texture2D coinTexture;
    Texture2D starTexture;
    Texture2D levelEndTexture;
    Texture2D trailHorizontal;
    Texture2D trailVertical;
    Texture2D playerFrames[PLAYER_ANIM_FRAMES];
    int       playerAnimFrame;
    int       playerAnimTimer;
    int       playerX;
    int       playerY;
    int       velocityX;
    int       velocityY;
    float     playerRotation;
    int       tileMap[MAP_ROWS][MAP_COLUMNS];
    int       score;
    int       levelCompleted;
    Vector2   trailPositions[TRAIL_LENGTH];
    float     blinkTimer;
    Texture2D starCollectedTexture;
    Texture2D starEmptyTexture;
    int       starsCollected;
    int       starsTotal;
    int       victoryStarsShown;
    float     victoryStarTimer;
    Sound     soundDash;
    Sound     soundHitWall;
    Sound     soundCollectDot;
    Sound     soundCollectCoin;
    Sound     soundCollectStar;
    Sound     soundLevelComplete;
    Sound     soundLevelStart;
    float     cameraX;
    float     cameraY;
    float     timer;
    int       timerStarted;
    int       timerExpired;
    int       showingLeaderboard;
    int       enteringInitials;
    int       showingVictoryOptions;
    char      initials[4];
    int       initialIndex;
    int       initialCharIndex[3];
    LeaderboardEntry leaderboard[LEADERBOARD_SIZE];
    int       leaderboardCount;
    // Bat-related fields
    Texture2D batTexture;
    Bat       bats[MAX_BATS];
    int       batCount;
    // Player death
    int       playerDead;
} GameState;

void      GameLoad(GameState* gameState);
SceneType GameUpdate(GameState* gameState);
void      GameDraw(GameState* gameState);
void      GameUnload(GameState* gameState);