#include "raylib.h"

#define TILE_SIZE 30


int main() {

    InitWindow(1900 * TILE_SIZE, 800 * TILE_SIZE, "Mapa grande");
    SetTargetFPS(120);

    // animación
    Texture2D frames[5];
    frames[0] = LoadTexture("resources\\totm.png");
    frames[1] = LoadTexture("resources\\totm_1.png");
    frames[2] = LoadTexture("resources\\totm_2.png");
    frames[3] = LoadTexture("resources\\totm_3.png");
    frames[4] = LoadTexture("resources\\totm_4.png");

    int animOrder[8] = { 0,1,0,2,3,2,0,4 };
    int animFrame = 0;
    int animTimer = 0;
    int animSpeed = 10;

    float x = TILE_SIZE;
    float y = TILE_SIZE;
    float xspd = 0;
    float yspd = 0;
    float move_spd = 4.0f;

    while (!WindowShouldClose()) {

        // movimiento slide
        if (IsKeyDown(KEY_RIGHT)) { xspd = move_spd; yspd = 0; }
        else if (IsKeyDown(KEY_LEFT)) { xspd = -move_spd; yspd = 0; }
        else if (IsKeyDown(KEY_DOWN)) { yspd = move_spd; xspd = 0; }
        else if (IsKeyDown(KEY_UP)) { yspd = -move_spd; xspd = 0; }

        float newX = x + xspd;
        float newY = y + yspd;

        int tileX = (int)(newX / TILE_SIZE);
        int tileY = (int)(newY / TILE_SIZE);

        if (map[tileY][tileX] == 0) {
            x = newX;
            y = newY;
        }

        // animación idle
        animTimer++;
        if (animTimer >= animSpeed) {
            animTimer = 0;
            animFrame = (animFrame + 1) % 8;
        }

        BeginDrawing();
        ClearBackground(BLACK);

        // dibujar mapa
        for (int j = 0; j < MAP_H; j++) {
            for (int i = 0; i < MAP_W; i++) {
                if (map[j][i] == 1)
                    DrawRectangle(i * TILE_SIZE, j * TILE_SIZE, TILE_SIZE, TILE_SIZE, GRAY);
            }
        }

        // jugador
        Texture2D current = frames[animOrder[animFrame]];
        DrawTextureEx(current, (Vector2) { x, y }, 0.0f, 2.0f, WHITE);

        EndDrawing();
    }

    for (int i = 0; i < 5; i++) UnloadTexture(frames[i]);
    CloseWindow();
    return 0;
}