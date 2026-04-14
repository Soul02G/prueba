#include "game.h"
#include <cstdlib>
#include <cmath>
#include <cstring>
#include <cstdio>

static const int playerAnimSequence[PLAYER_ANIM_SEQ_LEN] = { 0, 1, 0, 2, 3, 2, 0, 4 };
static const int playerAnimSpeed = 10;
static const char* INITIALS_CHARS = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
static const int   INITIALS_CHAR_COUNT = 36;

static const int initialMap[MAP_ROWS][MAP_COLUMNS] = {
    {0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,1,9,1,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,1,3,1,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,1,3,1,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,1,3,1,0,0,0,0,0},
    {0,0,0,0,0,0,1,1,1,1,3,1,0,0,0,0,0},
    {0,0,0,0,0,0,1,5,3,3,3,1,0,0,0,0,0},
    {0,0,0,0,0,0,1,3,6,1,1,1,0,0,0,0,0},
    {0,0,0,0,0,0,1,3,6,1,1,1,0,0,0,0,0},
    {0,0,0,0,0,0,1,3,3,3,1,0,0,0,0,0,0},
    {0,0,0,0,0,0,1,1,1,3,1,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,1,3,1,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,1,3,1,0,0,0,0,0,0},
    {0,0,0,0,0,0,1,1,1,3,1,1,1,0,0,0,0},
    {0,0,0,0,0,0,1,3,3,3,3,3,1,0,0,0,0},
    {0,0,0,0,0,0,1,3,0,3,1,4,1,0,0,0,0},
    {0,0,0,0,0,0,1,3,0,3,1,3,1,0,0,0,0},
    {0,0,0,0,0,0,1,3,3,5,1,3,1,0,0,0,0},
    {0,0,0,1,1,1,1,1,1,0,1,3,1,0,0,0,0},
    {0,0,0,1,3,3,3,3,1,0,1,3,1,0,0,0,0},
    {0,0,0,1,3,1,1,3,1,1,1,3,1,0,0,0,0},
    {0,0,0,1,3,1,1,3,4,3,3,3,1,0,0,0,0},
    {1,1,1,1,3,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,3,3,3,3,3,3,3,3,3,1,1,5,3,4,3,1},
    {1,3,7,7,4,1,1,1,1,3,3,3,3,0,0,3,1},
    {1,3,0,0,3,1,0,0,1,1,1,1,1,0,0,3,1},
    {1,3,3,3,3,1,0,0,0,1,0,0,0,0,0,3,1},
    {1,1,1,1,1,1,0,0,0,1,0,0,0,0,0,3,1},
    {0,0,0,0,0,0,0,0,0,1,0,0,2,0,0,3,1},
    {0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1}
};

static int TileIsWall(const int map[MAP_ROWS][MAP_COLUMNS], int row, int col) {
    if (row < 0 || row >= MAP_ROWS || col < 0 || col >= MAP_COLUMNS) return 0;
    return map[row][col] == TILE_WALL;
}

static int GetWallVariant(const int map[MAP_ROWS][MAP_COLUMNS], int row, int col) {
    int hasWallAbove = TileIsWall(map, row - 1, col);
    int hasWallBelow = TileIsWall(map, row + 1, col);
    int hasWallLeft = TileIsWall(map, row, col - 1);
    int hasWallRight = TileIsWall(map, row, col + 1);
    int hasWallAboveLeft = TileIsWall(map, row - 1, col - 1);
    int hasWallAboveRight = TileIsWall(map, row - 1, col + 1);
    int hasWallBelowLeft = TileIsWall(map, row + 1, col - 1);
    int hasWallBelowRight = TileIsWall(map, row + 1, col + 1);

    if (!hasWallAbove && !hasWallLeft)  return WALL_CORNER_TL;
    if (!hasWallAbove && !hasWallRight) return WALL_CORNER_TR;
    if (!hasWallBelow && !hasWallLeft)  return WALL_CORNER_BL;
    if (!hasWallBelow && !hasWallRight) return WALL_CORNER_BR;
    if (!hasWallAbove) return WALL_BORDER_TOP;
    if (!hasWallBelow) return WALL_BORDER_BOTTOM;
    if (!hasWallLeft)  return WALL_BORDER_LEFT;
    if (!hasWallRight) return WALL_BORDER_RIGHT;

    int missingDiagonals = (!hasWallAboveLeft) + (!hasWallAboveRight)
        + (!hasWallBelowLeft) + (!hasWallBelowRight);
    if (missingDiagonals == 1) {
        if (!hasWallAboveLeft)  return WALL_INNER_CORNER_TL;
        if (!hasWallAboveRight) return WALL_INNER_CORNER_TR;
        if (!hasWallBelowLeft)  return WALL_INNER_CORNER_BL;
        if (!hasWallBelowRight) return WALL_INNER_CORNER_BR;
    }
    return WALL_SOLID;
}

static void PlacePlayerAtSpawn(GameState* gameState) {
    for (int row = 0; row < MAP_ROWS; row++)
        for (int col = 0; col < MAP_COLUMNS; col++)
            if (gameState->tileMap[row][col] == TILE_PLAYER_SPAWN) {
                gameState->playerX = col * TILE_SIZE;
                gameState->playerY = row * TILE_SIZE;
                gameState->tileMap[row][col] = TILE_EMPTY;
                return;
            }
}

static void CollectTileUnderPlayer(GameState* gameState) {
    int col = (gameState->playerX + TILE_SIZE / 2) / TILE_SIZE;
    int row = (gameState->playerY + TILE_SIZE / 2) / TILE_SIZE;
    if (col < 0 || col >= MAP_COLUMNS || row < 0 || row >= MAP_ROWS) return;

    int tile = gameState->tileMap[row][col];
    if (tile == TILE_DOT) {
        gameState->score += 10;
        gameState->tileMap[row][col] = TILE_EMPTY;
        PlaySound(gameState->soundCollectDot);
    }
    if (tile == TILE_COIN) {
        gameState->score += 50;
        gameState->tileMap[row][col] = TILE_EMPTY;
        PlaySound(gameState->soundCollectCoin);
    }
    if (tile == TILE_STAR) {
        gameState->score += 100;
        gameState->starsCollected++;
        gameState->tileMap[row][col] = TILE_EMPTY;
        PlaySound(gameState->soundCollectStar);
    }
    if (tile == TILE_LEVEL_END && !gameState->levelCompleted) {
        gameState->levelCompleted = 1;
        gameState->victoryStarTimer = 0.0f;
        gameState->victoryStarsShown = 0;
        gameState->enteringInitials = 1;
        gameState->initialIndex = 0;
        gameState->initials[0] = INITIALS_CHARS[gameState->initialCharIndex[0]];
        gameState->initials[1] = INITIALS_CHARS[gameState->initialCharIndex[1]];
        gameState->initials[2] = INITIALS_CHARS[gameState->initialCharIndex[2]];
        gameState->initials[3] = '\0';
        PlaySound(gameState->soundLevelComplete);
    }
}

static void InsertLeaderboard(GameState* gs) {
    if (gs->leaderboardCount < LEADERBOARD_SIZE)
        gs->leaderboardCount++;

    LeaderboardEntry entry;
    strncpy(entry.initials, gs->initials, 4);
    entry.score = gs->score;
    entry.stars = gs->starsCollected;
    entry.time = gs->timer;

    int pos = gs->leaderboardCount - 1;
    gs->leaderboard[pos] = entry;

    for (int i = pos; i > 0; i--) {
        if (gs->leaderboard[i].time < gs->leaderboard[i - 1].time) {
            LeaderboardEntry tmp = gs->leaderboard[i];
            gs->leaderboard[i] = gs->leaderboard[i - 1];
            gs->leaderboard[i - 1] = tmp;
        }
        else break;
    }

    FILE* f = fopen("leaderboard.bin", "wb");
    if (f) {
        fwrite(&gs->leaderboardCount, sizeof(int), 1, f);
        fwrite(gs->leaderboard, sizeof(LeaderboardEntry), gs->leaderboardCount, f);
        fclose(f);
    }
}

// Helper para resetear el estado de juego sin recargar texturas/sonidos
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

    for (int row = 0; row < MAP_ROWS; row++)
        for (int col = 0; col < MAP_COLUMNS; col++)
            gameState->tileMap[row][col] = initialMap[row][col];

    gameState->starsTotal = 0;
    for (int row = 0; row < MAP_ROWS; row++)
        for (int col = 0; col < MAP_COLUMNS; col++)
            if (gameState->tileMap[row][col] == TILE_STAR)
                gameState->starsTotal++;

    // Spawn bats from map tiles
    gameState->batCount = 0;
    for (int row = 0; row < MAP_ROWS; row++) {
        for (int col = 0; col < MAP_COLUMNS; col++) {
            int tile = gameState->tileMap[row][col];
            if (tile == TILE_HORIZONTAL_BAT || tile == TILE_VERTICAL_BAT) {
                if (gameState->batCount < MAX_BATS) {
      Bat& bat = gameState->bats[gameState->batCount];
           bat.x = (float)(col * TILE_SIZE);
        bat.y = (float)(row * TILE_SIZE);
      bat.isHorizontal = (tile == TILE_HORIZONTAL_BAT) ? 1 : 0;
bat.stopTimer = 0.0f;
                  bat.deadly = 1;  // Bats are deadly by default
                    // Horizontal bats start moving right, vertical bats start moving up
  if (bat.isHorizontal) {
      bat.velocityX = BAT_SPEED;
         bat.velocityY = 0.0f;
   } else {
   bat.velocityX = 0.0f;
             bat.velocityY = -BAT_SPEED;
   }
      gameState->batCount++;
     }
         gameState->tileMap[row][col] = TILE_EMPTY;  // Clear the spawn tile
   }
        }
    }

    PlacePlayerAtSpawn(gameState);

    for (int i = 0; i < TRAIL_LENGTH; i++)
        gameState->trailPositions[i] = { (float)gameState->playerX, (float)gameState->playerY };

    gameState->cameraX = (float)(gameState->playerX - SCREEN_WIDTH / 2 + TILE_SIZE / 2);
    gameState->cameraY = (float)(gameState->playerY - SCREEN_HEIGHT / 2 + TILE_SIZE / 2);
}

void GameLoad(GameState* gameState) {
    const char* wallTextureFiles[WALL_VARIANT_COUNT] = {
        "resources\\tile_wall_solid.png",
        "resources\\tile_wall_top.png",
        "resources\\tile_wall_bottom.png",
        "resources\\tile_wall_left.png",
        "resources\\tile_wall_right.png",
        "resources\\tile_wall_tl.png",
        "resources\\tile_wall_tr.png",
        "resources\\tile_wall_bl.png",
        "resources\\tile_wall_br.png",
        "resources\\tile_wall_inner_tl.png",
        "resources\\tile_wall_inner_tr.png",
        "resources\\tile_wall_inner_bl.png",
        "resources\\tile_wall_inner_br.png"
    };
    for (int i = 0; i < WALL_VARIANT_COUNT; i++) {
        gameState->wallTextures[i] = LoadTexture(wallTextureFiles[i]);
        SetTextureFilter(gameState->wallTextures[i], TEXTURE_FILTER_POINT);
    }

    gameState->dotTexture = LoadTexture("resources\\tile_dot.png");
    gameState->coinTexture = LoadTexture("resources\\tile_coin.png");
    gameState->starTexture = LoadTexture("resources\\tile_star.png");
    gameState->levelEndTexture = LoadTexture("resources\\tile_end.png");
    gameState->trailHorizontal = LoadTexture("resources\\trail.png");
    gameState->trailVertical = LoadTexture("resources\\trailVert.png");

    gameState->playerFrames[0] = LoadTexture("resources\\totm.png");
    gameState->playerFrames[1] = LoadTexture("resources\\totm_1.png");
    gameState->playerFrames[2] = LoadTexture("resources\\totm_2.png");
    gameState->playerFrames[3] = LoadTexture("resources\\totm_3.png");
    gameState->playerFrames[4] = LoadTexture("resources\\totm_4.png");

    SetTextureFilter(gameState->dotTexture, TEXTURE_FILTER_POINT);
    SetTextureFilter(gameState->coinTexture, TEXTURE_FILTER_POINT);
    SetTextureFilter(gameState->starTexture, TEXTURE_FILTER_POINT);
    SetTextureFilter(gameState->levelEndTexture, TEXTURE_FILTER_POINT);
    SetTextureFilter(gameState->trailHorizontal, TEXTURE_FILTER_POINT);
    SetTextureFilter(gameState->trailVertical, TEXTURE_FILTER_POINT);
    for (int i = 0; i < PLAYER_ANIM_FRAMES; i++)
        SetTextureFilter(gameState->playerFrames[i], TEXTURE_FILTER_POINT);

    gameState->starCollectedTexture = LoadTexture("resources\\star_collected.png");
    gameState->starEmptyTexture = LoadTexture("resources\\star_empty.png");
    SetTextureFilter(gameState->starCollectedTexture, TEXTURE_FILTER_POINT);
    SetTextureFilter(gameState->starEmptyTexture, TEXTURE_FILTER_POINT);

    // Load bat texture
    gameState->batTexture = LoadTexture("resources\\bat_drawing.png");
    SetTextureFilter(gameState->batTexture, TEXTURE_FILTER_POINT);

    gameState->leaderboardCount = 0;
    FILE* f = fopen("leaderboard.bin", "rb");
    if (f) {
        fread(&gameState->leaderboardCount, sizeof(int), 1, f);
        fread(gameState->leaderboard, sizeof(LeaderboardEntry), gameState->leaderboardCount, f);
        fclose(f);
    }

    gameState->soundDash = LoadSound("cosas de la wiki\\dash.wav");
    gameState->soundHitWall = LoadSound("cosas de la wiki\\hitting wall.wav");
    gameState->soundCollectDot = LoadSound("cosas de la wiki\\collecting dots.wav");
    gameState->soundCollectCoin = LoadSound("cosas de la wiki\\collecting coin.wav");
    gameState->soundCollectStar = LoadSound("cosas de la wiki\\collecting star.wav");
    gameState->soundLevelComplete = LoadSound("cosas de la wiki\\level complete.wav");
    gameState->soundLevelStart = LoadSound("cosas de la wiki\\start of the level.wav");

    // FIX Bug 2: usar ResetGameState para inicializar todo el estado de juego,
    // garantizando que showingVictoryOptions y levelCompleted arrancan a 0
    ResetGameState(gameState);

    PlaySound(gameState->soundLevelStart);
}

SceneType GameUpdate(GameState* gameState) {
    float dt = GetFrameTime();

    gameState->blinkTimer += dt;
    if (gameState->blinkTimer > 1.0f) gameState->blinkTimer = 0.0f;

    // --- ENTERING INITIALS ---
    if (gameState->enteringInitials) {
        int idx = gameState->initialIndex;

        if (IsKeyPressed(KEY_UP)) {
            gameState->initialCharIndex[idx] = (gameState->initialCharIndex[idx] + 1) % INITIALS_CHAR_COUNT;
            gameState->initials[idx] = INITIALS_CHARS[gameState->initialCharIndex[idx]];
        }
        if (IsKeyPressed(KEY_DOWN)) {
            gameState->initialCharIndex[idx] = (gameState->initialCharIndex[idx] - 1 + INITIALS_CHAR_COUNT) % INITIALS_CHAR_COUNT;
            gameState->initials[idx] = INITIALS_CHARS[gameState->initialCharIndex[idx]];
        }
        if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_ENTER)) {
            if (idx < 2) {
                gameState->initialIndex++;
            }
            else {
                gameState->enteringInitials = 0;
                gameState->showingLeaderboard = 1;
                InsertLeaderboard(gameState);
            }
        }
        if (IsKeyPressed(KEY_LEFT) && idx > 0) {
            gameState->initialIndex--;
        }
        if (IsKeyPressed(KEY_BACKSPACE)) {
            gameState->enteringInitials = 0;
            gameState->showingLeaderboard = 0;
            gameState->showingVictoryOptions = 1;
        }
        return SCENE_GAME;
    }

    // --- VICTORY OPTIONS (ver leaderboard o salir) ---
    if (gameState->showingVictoryOptions) {
        if (IsKeyPressed(KEY_L)) {
            gameState->showingVictoryOptions = 0;
            gameState->showingLeaderboard = 1;
        }
        if (IsKeyPressed(KEY_M)) return SCENE_MAP;
        // FIX Bug 1: SPACE reinicia el nivel correctamente reseteando levelCompleted y todo el estado
        if (IsKeyPressed(KEY_SPACE)) {
            ResetGameState(gameState);
            PlaySound(gameState->soundLevelStart);
            return SCENE_GAME;
        }
        return SCENE_GAME;
    }

    // --- LEADERBOARD ---
    if (gameState->showingLeaderboard) {
        if (IsKeyPressed(KEY_ESCAPE)) return SCENE_MAP;
        if (IsKeyPressed(KEY_L))      gameState->showingVictoryOptions = 1;
        if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
            ResetGameState(gameState);
            PlaySound(gameState->soundLevelStart);
        }
        return SCENE_GAME;
    }

    // --- TIMER ---
    if (gameState->timerStarted && !gameState->levelCompleted) {
        gameState->timer += dt;
    }

    // --- INPUT ---
    if (gameState->velocityX == 0 && gameState->velocityY == 0) {
        if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) { gameState->velocityX = PLAYER_MOVE_SPEED; gameState->playerRotation = 90;  gameState->timerStarted = 1; PlaySound(gameState->soundDash); }
        else if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A)) { gameState->velocityX = -PLAYER_MOVE_SPEED; gameState->playerRotation = 270; gameState->timerStarted = 1; PlaySound(gameState->soundDash); }
        else if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) { gameState->velocityY = PLAYER_MOVE_SPEED; gameState->playerRotation = 180; gameState->timerStarted = 1; PlaySound(gameState->soundDash); }
        else if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) { gameState->velocityY = -PLAYER_MOVE_SPEED; gameState->playerRotation = 0;   gameState->timerStarted = 1; PlaySound(gameState->soundDash); }
    }

    // --- MOVEMENT X ---
    if (gameState->velocityX != 0) {
        int nextX = gameState->playerX + gameState->velocityX;
        int tileRow = gameState->playerY / TILE_SIZE;
        int nextTileCol = (gameState->velocityX > 0)
            ? (nextX + TILE_SIZE - 1) / TILE_SIZE : nextX / TILE_SIZE;
        nextTileCol = (nextTileCol < 0) ? 0 : (nextTileCol >= MAP_COLUMNS ? MAP_COLUMNS - 1 : nextTileCol);

        if (gameState->tileMap[tileRow][nextTileCol] == TILE_WALL) {
            gameState->playerX = (gameState->velocityX > 0)
                ? (nextTileCol * TILE_SIZE) - TILE_SIZE : (nextTileCol + 1) * TILE_SIZE;
            gameState->velocityX = 0;
            PlaySound(gameState->soundHitWall);
        }
        else if (nextX < 0 || nextX + TILE_SIZE > MAP_COLUMNS * TILE_SIZE) {
            gameState->velocityX = 0;
            PlaySound(gameState->soundHitWall);
        }
        else {
            gameState->playerX = nextX;
        }
    }

    // --- MOVEMENT Y ---
    if (gameState->velocityY != 0) {
        int nextY = gameState->playerY + gameState->velocityY;
        int tileCol = gameState->playerX / TILE_SIZE;
        int nextTileRow = (gameState->velocityY > 0)
            ? (nextY + TILE_SIZE - 1) / TILE_SIZE : nextY / TILE_SIZE;
        nextTileRow = (nextTileRow < 0) ? 0 : (nextTileRow >= MAP_ROWS ? MAP_ROWS - 1 : nextTileRow);

        if (gameState->tileMap[nextTileRow][tileCol] == TILE_WALL) {
            gameState->playerY = (gameState->velocityY > 0)
                ? (nextTileRow * TILE_SIZE) - TILE_SIZE : (nextTileRow + 1) * TILE_SIZE;
            gameState->velocityY = 0;
            PlaySound(gameState->soundHitWall);
        }
        else if (nextY < 0 || nextY + TILE_SIZE > MAP_ROWS * TILE_SIZE) {
            gameState->velocityY = 0;
            PlaySound(gameState->soundHitWall);
        }
        else {
            gameState->playerY = nextY;
        }
    }

    // --- BAT MOVEMENT ---
    for (int i = 0; i < gameState->batCount; i++) {
     Bat& bat = gameState->bats[i];
        
        // If bat is stopped, count down the timer
        if (bat.stopTimer > 0.0f) {
        bat.stopTimer -= dt;
    continue;
        }
     
        // Calculate next position
        float nextX = bat.x + bat.velocityX;
  float nextY = bat.y + bat.velocityY;
        
        // Check for wall collision
        int batCol = (int)(bat.x + TILE_SIZE / 2) / TILE_SIZE;
        int batRow = (int)(bat.y + TILE_SIZE / 2) / TILE_SIZE;
        
        int nextCol, nextRow;
        if (bat.isHorizontal) {
    // Check the tile ahead horizontally
   nextCol = (bat.velocityX > 0) 
              ? (int)(nextX + TILE_SIZE - 1) / TILE_SIZE 
       : (int)nextX / TILE_SIZE;
       nextRow = batRow;
        } else {
        // Check the tile ahead vertically
     nextCol = batCol;
          nextRow = (bat.velocityY > 0) 
             ? (int)(nextY + TILE_SIZE - 1) / TILE_SIZE 
       : (int)nextY / TILE_SIZE;
        }
        
   // Clamp to map bounds
   nextCol = (nextCol < 0) ? 0 : (nextCol >= MAP_COLUMNS ? MAP_COLUMNS - 1 : nextCol);
        nextRow = (nextRow < 0) ? 0 : (nextRow >= MAP_ROWS ? MAP_ROWS - 1 : nextRow);
     
        // Check if next tile is a wall
     if (gameState->tileMap[nextRow][nextCol] == TILE_WALL) {
            // Stop for 1 second and reverse direction
bat.stopTimer = BAT_STOP_TIME;
     bat.velocityX *= -1;
            bat.velocityY *= -1;
        } else {
// Move the bat
       bat.x = nextX;
     bat.y = nextY;
      }
    }

    // --- BAT-PLAYER COLLISION ---
    if (!gameState->playerDead) {
        for (int i = 0; i < gameState->batCount; i++) {
      Bat& bat = gameState->bats[i];
            
            // Simple AABB collision check
     float playerLeft = (float)gameState->playerX;
     float playerRight = (float)gameState->playerX + TILE_SIZE;
       float playerTop = (float)gameState->playerY;
            float playerBottom = (float)gameState->playerY + TILE_SIZE;
  
     float batLeft = bat.x;
            float batRight = bat.x + TILE_SIZE;
        float batTop = bat.y;
     float batBottom = bat.y + TILE_SIZE;
         
      // Check for overlap
  if (playerLeft < batRight && playerRight > batLeft &&
            playerTop < batBottom && playerBottom > batTop) {
    // Collision detected - transfer deadly property to player
     if (bat.deadly) {
       gameState->playerDead = 1;
    }
         }
        }
    }

    // --- HANDLE PLAYER DEATH ---
  if (gameState->playerDead) {
     // Reset the game state after death
   ResetGameState(gameState);
   PlaySound(gameState->soundHitWall);  // Use hit wall sound for death (or add a death sound)
       return SCENE_GAME;
   }

    CollectTileUnderPlayer(gameState);

    int playerIsMoving = (gameState->velocityX != 0 || gameState->velocityY != 0);
    if (playerIsMoving) {
        gameState->playerAnimFrame = 0;
        gameState->playerAnimTimer = 0;
        for (int i = TRAIL_LENGTH - 1; i > 0; i--)
            gameState->trailPositions[i] = gameState->trailPositions[i - 1];
        gameState->trailPositions[0] = { (float)gameState->playerX, (float)gameState->playerY };
    }
    else {
        if (++gameState->playerAnimTimer >= playerAnimSpeed) {
            gameState->playerAnimTimer = 0;
            gameState->playerAnimFrame = (gameState->playerAnimFrame + 1) % PLAYER_ANIM_SEQ_LEN;
        }
        for (int i = 0; i < TRAIL_LENGTH; i++)
            gameState->trailPositions[i] = { (float)gameState->playerX, (float)gameState->playerY };
    }

    return SCENE_GAME;
}

void GameDraw(GameState* gameState) {
    ClearBackground(BLACK);

    int mapPixelWidth = MAP_COLUMNS * TILE_SIZE;
    int mapPixelHeight = MAP_ROWS * TILE_SIZE;

    float targetCameraX = (float)(gameState->playerX - SCREEN_WIDTH / 2 + TILE_SIZE / 2);
    float targetCameraY = (float)(gameState->playerY - SCREEN_HEIGHT / 2 + TILE_SIZE / 2);

    const float cameraSmoothing = 0.15f;
    gameState->cameraX += (targetCameraX - gameState->cameraX) * cameraSmoothing;
    gameState->cameraY += (targetCameraY - gameState->cameraY) * cameraSmoothing;

    if (gameState->cameraX < 0) gameState->cameraX = 0;
    if (gameState->cameraX > mapPixelWidth - SCREEN_WIDTH)  gameState->cameraX = (float)(mapPixelWidth - SCREEN_WIDTH);
    if (mapPixelWidth < SCREEN_WIDTH)  gameState->cameraX = -(float)(SCREEN_WIDTH - mapPixelWidth) / 2.0f;
    if (gameState->cameraY < 0) gameState->cameraY = 0;
    if (gameState->cameraY > mapPixelHeight - SCREEN_HEIGHT) gameState->cameraY = (float)(mapPixelHeight - SCREEN_HEIGHT);
    if (mapPixelHeight < SCREEN_HEIGHT) gameState->cameraY = -(float)(SCREEN_HEIGHT - mapPixelHeight) / 2.0f;

    int cameraX = (int)gameState->cameraX;
    int cameraY = (int)gameState->cameraY;

    Color collectibleColor = (gameState->blinkTimer < 0.5f)
        ? Color{ 255, 220,   0, 255 }
    : Color{ 180,   0, 220, 255 };

    // --- TILEMAP ---
    for (int row = 0; row < MAP_ROWS; row++) {
        for (int col = 0; col < MAP_COLUMNS; col++) {
            int tileType = gameState->tileMap[row][col];
            if (tileType == TILE_EMPTY) continue;
            int screenX = col * TILE_SIZE - cameraX;
            int screenY = row * TILE_SIZE - cameraY;
            if (screenX < -TILE_SIZE || screenX > SCREEN_WIDTH + TILE_SIZE) continue;
            if (screenY < -TILE_SIZE || screenY > SCREEN_HEIGHT + TILE_SIZE) continue;

            Rectangle tileSource = { 0, 0, (float)TILE_SIZE, (float)TILE_SIZE };
            Rectangle tileDest = { (float)screenX, (float)screenY, (float)TILE_SIZE, (float)TILE_SIZE };
            Vector2   tileOrigin = { 0, 0 };

            switch (tileType) {
            case TILE_WALL: {
                int wallVariant = GetWallVariant(gameState->tileMap, row, col);
                DrawTexturePro(gameState->wallTextures[wallVariant], tileSource, tileDest, tileOrigin, 0, WHITE);
                break;
            }
            case TILE_DOT:      DrawTexturePro(gameState->dotTexture, { 0, 0, (float)gameState->dotTexture.width,      (float)gameState->dotTexture.height }, tileDest, tileOrigin, 0, collectibleColor); break;
            case TILE_COIN:     DrawTexturePro(gameState->coinTexture, { 0, 0, (float)gameState->coinTexture.width,     (float)gameState->coinTexture.height }, tileDest, tileOrigin, 0, collectibleColor); break;
            case TILE_STAR:     DrawTexturePro(gameState->starTexture, { 0, 0, (float)gameState->starTexture.width,     (float)gameState->starTexture.height }, tileDest, tileOrigin, 0, collectibleColor); break;
            case TILE_LEVEL_END:DrawTexturePro(gameState->levelEndTexture, { 0, 0, (float)gameState->levelEndTexture.width, (float)gameState->levelEndTexture.height }, tileDest, tileOrigin, 0, WHITE);            break;
            }
        }
    }

    // --- TRAIL ---
    int playerIsMoving = (gameState->velocityX != 0 || gameState->velocityY != 0);
    if (playerIsMoving) {
        Texture2D activeTrail = (gameState->velocityY != 0) ? gameState->trailVertical : gameState->trailHorizontal;
        for (int i = TRAIL_LENGTH - 1; i >= 0; i--) {
            float     opacity = (float)(TRAIL_LENGTH - i) / (float)TRAIL_LENGTH;
            Color     trailColor = { 255, 255, 255, (unsigned char)(255 * opacity) };
            Rectangle trailSource = { 0, 0, (float)activeTrail.width, (float)activeTrail.height };
            Rectangle trailDest = {
                gameState->trailPositions[i].x - cameraX,
                gameState->trailPositions[i].y - cameraY,
                (float)TILE_SIZE, (float)TILE_SIZE
            };
            DrawTexturePro(activeTrail, trailSource, trailDest, { 0, 0 }, 0.0f, trailColor);
        }
    }

    // --- BATS ---
    for (int i = 0; i < gameState->batCount; i++) {
        Bat& bat = gameState->bats[i];
        int screenX = (int)bat.x - cameraX;
        int screenY = (int)bat.y - cameraY;
      
        // Skip if off-screen
        if (screenX < -TILE_SIZE || screenX > SCREEN_WIDTH + TILE_SIZE) continue;
        if (screenY < -TILE_SIZE || screenY > SCREEN_HEIGHT + TILE_SIZE) continue;
      
        Rectangle batSource = { 0, 0, (float)gameState->batTexture.width, (float)gameState->batTexture.height };
        Rectangle batDest = { (float)screenX, (float)screenY, (float)TILE_SIZE, (float)TILE_SIZE };
        
        DrawTexturePro(gameState->batTexture, batSource, batDest, { 0, 0 }, 0.0f, WHITE);
    }

    // --- PLAYER ---
    Texture2D currentPlayerFrame = gameState->playerFrames[playerAnimSequence[gameState->playerAnimFrame]];
    Rectangle playerSource = { 0, 0, (float)currentPlayerFrame.width, (float)currentPlayerFrame.height };
    Rectangle playerDest = {
        (float)(gameState->playerX - cameraX) + TILE_SIZE / 2.0f,
        (float)(gameState->playerY - cameraY) + TILE_SIZE / 2.0f,
        (float)TILE_SIZE, (float)TILE_SIZE
    };
    DrawTexturePro(currentPlayerFrame, playerSource, playerDest, { TILE_SIZE / 2.0f, TILE_SIZE / 2.0f }, gameState->playerRotation + 180, WHITE);

    // --- HUD ---
    {
        int starSize = 32;
        int starPadding = 16;
        int totalWidth = gameState->starsTotal * (starSize + starPadding) - starPadding;
        int startX = (SCREEN_WIDTH - totalWidth) / 2;
        int startY = 10;
        int hudH = starSize + startY * 2;

        DrawRectangle(0, 0, SCREEN_WIDTH, hudH, BLACK);

        DrawText(TextFormat("SCORE: %d", gameState->score), 10, 10, 20, WHITE);

        // Timer: siempre visible, ascendente, gris antes del primer movimiento
        {
            int   secs = (int)gameState->timer;
            int   centesimas = (int)((gameState->timer - secs) * 100);
            Color timerColor = gameState->timerStarted ? WHITE : Color{ 150, 150, 150, 255 };
            DrawText(TextFormat("%d.%02d", secs, centesimas), 10, 32, 24, timerColor);
        }

        for (int i = 0; i < gameState->starsTotal; i++) {
            Texture2D& starTex = (i < gameState->starsCollected)
                ? gameState->starCollectedTexture : gameState->starEmptyTexture;
            Rectangle src = { 0, 0, (float)starTex.width, (float)starTex.height };
            Rectangle dest = { (float)(startX + i * (starSize + starPadding)), (float)startY,
                               (float)starSize, (float)starSize };
            DrawTexturePro(starTex, src, dest, { 0, 0 }, 0.0f, WHITE);
        }
    }

    // --- ENTERING INITIALS ---
    if (gameState->enteringInitials) {
        DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Color{ 0, 0, 0, 180 });
        int panelW = 360, panelH = 260;
        int panelX = (SCREEN_WIDTH - panelW) / 2, panelY = (SCREEN_HEIGHT - panelH) / 2;
        DrawRectangle(panelX, panelY, panelW, panelH, Color{ 210, 180, 20, 255 });
        DrawRectangleLines(panelX, panelY, panelW, panelH, Color{ 100, 80, 0, 255 });

        const char* title = "INTRODUCE TUS INICIALES";
        DrawText(title, panelX + (panelW - MeasureText(title, 18)) / 2, panelY + 20, 18, Color{ 40, 30, 0, 255 });

        int letterSize = 48;
        int letterGap = 20;
        int lettersW = 3 * letterSize + 2 * letterGap;
        int lettersX = panelX + (panelW - lettersW) / 2;
        int lettersY = panelY + 70;

        for (int i = 0; i < 3; i++) {
            int lx = lettersX + i * (letterSize + letterGap);
            bool selected = (i == gameState->initialIndex);
            Color bgCol = selected ? Color{ 255, 240, 80, 255 } : Color{ 180, 150, 0, 255 };
            Color txtCol = Color{ 40, 30, 0, 255 };
            DrawRectangle(lx, lettersY, letterSize, letterSize + 10, bgCol);
            DrawRectangleLines(lx, lettersY, letterSize, letterSize + 10, Color{ 80, 60, 0, 255 });
            char buf[2] = { gameState->initials[i], '\0' };
            DrawText(buf, lx + (letterSize - MeasureText(buf, 36)) / 2, lettersY + 6, 36, txtCol);
            if (selected) {
                DrawText("^", lx + (letterSize - MeasureText("^", 16)) / 2, lettersY + letterSize + 16, 16, Color{ 40, 30, 0, 255 });
                DrawText("v", lx + (letterSize - MeasureText("v", 16)) / 2, lettersY - 20, 16, Color{ 40, 30, 0, 255 });
            }
        }

        const char* h1 = "ARRIBA/ABAJO: cambiar letra";
        const char* h2 = "DERECHA/ENTER: siguiente    IZQUIERDA: anterior";
        const char* h3 = "BACKSPACE: no guardar partida";
        DrawText(h1, panelX + (panelW - MeasureText(h1, 13)) / 2, panelY + 170, 13, Color{ 40, 30, 0, 200 });
        DrawText(h2, panelX + (panelW - MeasureText(h2, 13)) / 2, panelY + 190, 13, Color{ 40, 30, 0, 200 });
        DrawText(h3, panelX + (panelW - MeasureText(h3, 13)) / 2, panelY + 210, 13, Color{ 40, 30, 0, 200 });
        return;
    }

    // --- VICTORY OPTIONS ---
    if (gameState->showingVictoryOptions) {
        DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Color{ 0, 0, 0, 180 });
        int panelW = 360, panelH = 230;
        int panelX = (SCREEN_WIDTH - panelW) / 2, panelY = (SCREEN_HEIGHT - panelH) / 2;
        DrawRectangle(panelX, panelY, panelW, panelH, Color{ 210, 180, 20, 255 });
        DrawRectangleLines(panelX, panelY, panelW, panelH, Color{ 100, 80, 0, 255 });
        DrawRectangleLines(panelX + 2, panelY + 2, panelW - 4, panelH - 4, Color{ 255, 240, 80, 255 });
        const char* vTitle = "VICTORIA";
        DrawText(vTitle, panelX + (panelW - MeasureText(vTitle, 28)) / 2, panelY + 20, 28, Color{ 40, 30, 0, 255 });
        const char* opt1 = "L - Ver leaderboard";
        const char* opt2 = "SPACE - Volver a jugar";
        const char* opt3 = "M - Menu";
        DrawText(opt1, panelX + (panelW - MeasureText(opt1, 18)) / 2, panelY + 90, 18, Color{ 40, 30, 0, 255 });
        DrawText(opt2, panelX + (panelW - MeasureText(opt2, 18)) / 2, panelY + 120, 18, Color{ 40, 30, 0, 255 });
        DrawText(opt3, panelX + (panelW - MeasureText(opt3, 18)) / 2, panelY + 150, 18, Color{ 40, 30, 0, 255 });
        return;
    }

    // --- LEADERBOARD ---
    if (gameState->showingLeaderboard) {
        DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Color{ 0, 0, 0, 210 });
        int panelW = 420, panelH = 520;
        int panelX = (SCREEN_WIDTH - panelW) / 2, panelY = (SCREEN_HEIGHT - panelH) / 2;
        DrawRectangle(panelX, panelY, panelW, panelH, Color{ 210, 180, 20, 255 });
        DrawRectangleLines(panelX, panelY, panelW, panelH, Color{ 100, 80, 0, 255 });
        DrawRectangleLines(panelX + 2, panelY + 2, panelW - 4, panelH - 4, Color{ 255, 240, 80, 255 });

        const char* title = "LEADERBOARD";
        DrawText(title, panelX + (panelW - MeasureText(title, 28)) / 2, panelY + 16, 28, Color{ 40, 30, 0, 255 });
        DrawLine(panelX + 20, panelY + 54, panelX + panelW - 20, panelY + 54, Color{ 100, 80, 0, 180 });

        DrawText("POS", panelX + 20, panelY + 64, 14, Color{ 40, 30, 0, 200 });
        DrawText("NOM", panelX + 80, panelY + 64, 14, Color{ 40, 30, 0, 200 });
        DrawText("PUNT", panelX + 160, panelY + 64, 14, Color{ 40, 30, 0, 200 });
        DrawText("ESTR", panelX + 260, panelY + 64, 14, Color{ 40, 30, 0, 200 });
        DrawText("TEMPS", panelX + 330, panelY + 64, 14, Color{ 40, 30, 0, 200 });
        DrawLine(panelX + 20, panelY + 82, panelX + panelW - 20, panelY + 82, Color{ 100, 80, 0, 180 });

        for (int i = 0; i < gameState->leaderboardCount; i++) {
            LeaderboardEntry& e = gameState->leaderboard[i];
            int rowY = panelY + 90 + i * 34;
            bool isPlayer = (strcmp(e.initials, gameState->initials) == 0 && i == gameState->leaderboardCount - 1);
            Color rowCol = isPlayer ? Color{ 255, 255, 100, 255 } : Color{ 40, 30, 0, 255 };
            if (isPlayer) DrawRectangle(panelX + 10, rowY - 4, panelW - 20, 28, Color{ 100, 80, 0, 100 });

            int mins = (int)e.time / 60;
            int secs = (int)e.time % 60;
            int cent = (int)((e.time - (int)e.time) * 100);
            DrawText(TextFormat("%d.", i + 1), panelX + 20, rowY, 18, rowCol);
            DrawText(e.initials, panelX + 80, rowY, 18, rowCol);
            DrawText(TextFormat("%d", e.score), panelX + 160, rowY, 18, rowCol);
            DrawText(TextFormat("%d", e.stars), panelX + 260, rowY, 18, rowCol);
            DrawText(TextFormat("%d:%02d.%02d", mins, secs, cent), panelX + 330, rowY, 18, rowCol);
        }

        const char* h1 = "ENTER: jugar de nuevo    ESC: mapa";
        DrawText(h1, panelX + (panelW - MeasureText(h1, 13)) / 2, panelY + panelH - 24, 13, Color{ 40, 30, 0, 200 });
        return;
    }

    // --- VICTORY PANEL ---
    if (gameState->levelCompleted) {
        DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Color{ 0, 0, 0, 180 });

        int panelW = 380, panelH = 420;
        int panelX = (SCREEN_WIDTH - panelW) / 2;
        int panelY = (SCREEN_HEIGHT - panelH) / 2;

        DrawRectangle(panelX, panelY, panelW, panelH, Color{ 180, 150,  0, 255 });
        DrawRectangle(panelX + 4, panelY + 4, panelW - 8, panelH - 8, Color{ 210, 180, 20, 255 });
        for (int ly = panelY + 4; ly < panelY + panelH - 4; ly += 4)
            DrawLine(panelX + 4, ly, panelX + panelW - 4, ly, Color{ 220, 190, 30, 60 });
        DrawRectangleLines(panelX, panelY, panelW, panelH, Color{ 100,  80,  0, 255 });
        DrawRectangleLines(panelX + 2, panelY + 2, panelW - 4, panelH - 4, Color{ 255, 240, 80, 255 });

        int closeX = panelX + panelW - 36, closeY = panelY + 10;
        DrawRectangle(closeX, closeY, 26, 26, Color{ 160, 130, 0, 255 });
        DrawRectangleLines(closeX, closeY, 26, 26, Color{ 80, 60, 0, 255 });
        DrawText("X", closeX + 7, closeY + 5, 16, Color{ 40, 30, 0, 255 });

        const char* faseText = "FASE 1";
        DrawText(faseText, panelX + (panelW - MeasureText(faseText, 18)) / 2, panelY + 16, 18, Color{ 40, 30, 0, 255 });
        const char* victoriaText = "VICTORIA";
        DrawText(victoriaText, panelX + (panelW - MeasureText(victoriaText, 32)) / 2, panelY + 42, 32, Color{ 40, 30, 0, 255 });
        DrawLine(panelX + 20, panelY + 84, panelX + panelW - 20, panelY + 84, Color{ 100, 80, 0, 180 });

        int starSize = 64, starSpacing = 10;
        int starsRowW = gameState->starsTotal * (starSize + starSpacing) - starSpacing;
        int starStartX = panelX + (panelW - starsRowW) / 2;
        int starStartY = panelY + 95;

        for (int i = 0; i < gameState->starsTotal; i++) {
            int sx = starStartX + i * (starSize + starSpacing);
            bool collected = (i < gameState->victoryStarsShown);
            bool animating = (i == gameState->victoryStarsShown && gameState->starsCollected > gameState->victoryStarsShown);

            Rectangle src, dest;
            if (animating) {
                float scale = 1.0f + 0.3f * (float)sin((gameState->victoryStarTimer / 0.5f) * 3.14159f);
                int   sz = (int)(starSize * scale);
                int   offset = (sz - starSize) / 2;
                src = { 0, 0, (float)gameState->starEmptyTexture.width, (float)gameState->starEmptyTexture.height };
                dest = { (float)(sx - offset), (float)(starStartY - offset), (float)sz, (float)sz };
                DrawTexturePro(gameState->starEmptyTexture, src, dest, { 0, 0 }, 0.0f, WHITE);
            }
            else if (collected) {
                src = { 0, 0, (float)gameState->starCollectedTexture.width, (float)gameState->starCollectedTexture.height };
                dest = { (float)sx, (float)starStartY, (float)starSize, (float)starSize };
                DrawTexturePro(gameState->starCollectedTexture, src, dest, { 0, 0 }, 0.0f, WHITE);
            }
            else {
                src = { 0, 0, (float)gameState->starEmptyTexture.width, (float)gameState->starEmptyTexture.height };
                dest = { (float)sx, (float)starStartY, (float)starSize, (float)starSize };
                DrawTexturePro(gameState->starEmptyTexture, src, dest, { 0, 0 }, 0.0f, WHITE);
            }
        }

        DrawLine(panelX + 20, panelY + 175, panelX + panelW - 20, panelY + 175, Color{ 100, 80, 0, 180 });

        const char* puntosLabel = "PUNTOS RECOGIDOS";
        DrawText(puntosLabel, panelX + (panelW - MeasureText(puntosLabel, 14)) / 2, panelY + 185, 14, Color{ 40, 30, 0, 200 });

        int barX = panelX + 30, barY = panelY + 207, barW = panelW - 60, barH = 22;
        DrawRectangle(barX, barY, barW, barH, Color{ 100, 80, 0, 255 });
        int filled = (gameState->score * barW) / 9999;
        if (filled > barW) filled = barW;
        if (filled > 0) DrawRectangle(barX + 2, barY + 2, filled - 4, barH - 4, Color{ 255, 220, 0, 255 });
        DrawCircle(barX + barW + 14, barY + barH / 2, 10, Color{ 255, 220, 0, 255 });
        DrawCircleLines(barX + barW + 14, barY + barH / 2, 10, Color{ 100, 80, 0, 255 });

        DrawText(TextFormat("%d", gameState->score), panelX + (panelW - MeasureText(TextFormat("%d", gameState->score), 16)) / 2, panelY + 237, 16, Color{ 40, 30, 0, 255 });
        DrawLine(panelX + 20, panelY + 262, panelX + panelW - 20, panelY + 262, Color{ 100, 80, 0, 180 });

        int btnX = panelX + 20, btnY = panelY + panelH - 80, btnW = panelW - 40, btnH = 50;
        DrawRectangle(btnX, btnY, btnW, btnH, Color{ 255, 220, 0, 255 });
        DrawRectangleLines(btnX, btnY, btnW, btnH, Color{ 100,  80,   0, 255 });
        DrawRectangleLines(btnX + 2, btnY + 2, btnW - 4, btnH - 4, Color{ 255, 255, 150, 255 });
        const char* nextText = "GUARDAR PUNTUACION";
        DrawText(nextText, btnX + (btnW - MeasureText(nextText, 22)) / 2, btnY + 13, 22, Color{ 40, 30, 0, 255 });

        const char* hintText = "PULSA ENTER O ESPACIO";
        DrawText(hintText, panelX + (panelW - MeasureText(hintText, 12)) / 2, panelY + panelH - 22, 12, Color{ 40, 30, 0, 180 });
    }
}

void GameUnload(GameState* gameState) {
    for (int i = 0; i < WALL_VARIANT_COUNT; i++) UnloadTexture(gameState->wallTextures[i]);
    UnloadTexture(gameState->dotTexture);
    UnloadTexture(gameState->coinTexture);
    UnloadTexture(gameState->starTexture);
    UnloadTexture(gameState->levelEndTexture);
    UnloadTexture(gameState->trailHorizontal);
    UnloadTexture(gameState->trailVertical);
    for (int i = 0; i < PLAYER_ANIM_FRAMES; i++) UnloadTexture(gameState->playerFrames[i]);
    UnloadTexture(gameState->starCollectedTexture);
    UnloadTexture(gameState->starEmptyTexture);
    UnloadTexture(gameState->batTexture);
    UnloadSound(gameState->soundDash);
    UnloadSound(gameState->soundHitWall);
    UnloadSound(gameState->soundCollectDot);
    UnloadSound(gameState->soundCollectCoin);
    UnloadSound(gameState->soundCollectStar);
    UnloadSound(gameState->soundLevelComplete);
    UnloadSound(gameState->soundLevelStart);
}