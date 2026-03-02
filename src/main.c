#include "raylib.h"

#define TRAIL_LENGTH 15

int main() {

    const int screenWidth = 1800;
    const int screenHeight = 900;

    InitWindow(screenWidth, screenHeight, "Tomb of The Mask");
    SetTargetFPS(120);

    Texture2D player = LoadTexture("resources\\totm.png");
    Texture2D trail = LoadTexture("resources\\trail.png"); // textura para la estela

    SetTextureFilter(player, TEXTURE_FILTER_POINT);
    SetTextureFilter(trail, TEXTURE_FILTER_POINT);

    int x = 30;
    int y = 30;

    int xspd = 0, yspd = 0;
    int move_spd = 30;
    float scale = 5.0f;

    Vector2 trailPositions[TRAIL_LENGTH] = { 0 };
    for (int i = 0; i < TRAIL_LENGTH; i++) {
        trailPositions[i] = (Vector2){ (float)x, (float)y };
    }

    while (!WindowShouldClose()) {

        // Movimiento
        if (xspd == 0 && yspd == 0)
        {
            if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) xspd = move_spd;
            else if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A)) xspd = -move_spd;
            else if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) yspd = move_spd;
            else if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) yspd = -move_spd;
        }

        // Limites con escala
        if (x + (int)(player.width * scale) + xspd >= screenWidth - 30) {
            x = screenWidth - 30 - (int)(player.width * scale);
            xspd = 0;
        }
        else if (x + xspd <= 30) {
            x = 30;
            xspd = 0;
        }
        else {
            x += xspd;
        }

        if (y + (int)(player.height * scale) + yspd >= screenHeight - 30) {
            y = screenHeight - 30 - (int)(player.height * scale);
            yspd = 0;
        }
        else if (y + yspd <= 30) {
            y = 30;
            yspd = 0;
        }
        else {
            y += yspd;
        }

        // Actualizar trail solo si el jugador se mueve
        if (xspd != 0 || yspd != 0) {
            for (int i = TRAIL_LENGTH - 1; i > 0; i--) {
                trailPositions[i] = trailPositions[i - 1];
            }
            trailPositions[0] = (Vector2){ (float)x, (float)y };
        }

        BeginDrawing();
        ClearBackground(BLACK);

        // Dibujar trail solo si el jugador se mueve
        if (xspd != 0 || yspd != 0) {
            for (int i = TRAIL_LENGTH - 1; i >= 0; i--) {
                float alpha = (float)(TRAIL_LENGTH - i) / (float)TRAIL_LENGTH; // Más cercano = más opaco
                Color tint = (Color){ 255, 255, 255, (unsigned char)(255 * alpha) };
                DrawTextureEx(trail, trailPositions[i], 0.0f, scale, tint);
            }
        }

        // Dibujar jugador
        DrawTextureEx(player, (Vector2) { (float)x, (float)y }, 0.0f, scale, WHITE);

        // Dibujar muros
        DrawRectangle(0, 0, 30, screenHeight, GRAY);
        DrawRectangle(screenWidth - 30, 0, 30, screenHeight, GRAY);
        DrawRectangle(0, 0, screenWidth, 30, GRAY);
        DrawRectangle(0, screenHeight - 30, screenWidth, 30, GRAY);

        EndDrawing();
    }

    UnloadTexture(player);
    UnloadTexture(trail);
    CloseWindow();
    return 0;
}