#include "raylib.h"

#define TRAIL_LENGTH 15

int main() {

    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "Tomb of The Mask");
    SetTargetFPS(120);

    // Animación
    Texture2D frames[5];
    frames[0] = LoadTexture("resources\\totm.png");
    frames[1] = LoadTexture("resources\\totm_1.png");
    frames[2] = LoadTexture("resources\\totm_2.png");
    frames[3] = LoadTexture("resources\\totm_3.png");
    frames[4] = LoadTexture("resources\\totm_4.png");

    int animOrder[8] = { 0, 1, 0, 2, 3, 2, 0, 4 };
    int animFrame = 0;
    int animTimer = 0;
    int animSpeed = 10;

    Texture2D trail = LoadTexture("resources\\trail.png");
    Texture2D vertTrail = LoadTexture("resources\\trailVert.png");
    Texture2D rebote = LoadTexture("resources\\rebot.png");

    SetTextureFilter(frames[0], TEXTURE_FILTER_POINT);
    SetTextureFilter(trail, TEXTURE_FILTER_POINT);
    SetTextureFilter(vertTrail, TEXTURE_FILTER_POINT);

    int x = 0;
    int y = 0;

    int xspd = 0, yspd = 0;
    int move_spd = 30;
    float scale = 5.0f;

    float rotation = 0.0f;

    int playerWidth = (int)(frames[0].width * scale);
    int playerHeight = (int)(frames[0].height * scale);

    // Rebote fijo
    int reboteWidth = (int)(rebote.width * scale);
    int reboteHeight = (int)(rebote.height * scale);
    int reboteX = screenWidth - reboteWidth;
    int reboteY = screenHeight - reboteHeight;

    Vector2 trailPositions[TRAIL_LENGTH];
    for (int i = 0; i < TRAIL_LENGTH; i++) {
        trailPositions[i] = (Vector2){ (float)x, (float)y };
    }

    while (!WindowShouldClose()) {

        // Movimiento
        if (xspd == 0 && yspd == 0)
        {
            if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) {
                xspd = move_spd;
                rotation = 90;
            }
            else if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A)) {
                xspd = -move_spd;
                rotation = 270;
            }
            else if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) {
                yspd = move_spd;
                rotation = 180;
            }
            else if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) {
                yspd = -move_spd;
                rotation = 0;
            }
        }

        if (x + playerWidth + xspd >= screenWidth) {
            x = screenWidth - playerWidth;
            xspd = 0;
        }
        else if (x + xspd <= 0) {
            x = 0;
            xspd = 0;
        }
        else {
            x += xspd;
        }

        if (y + playerHeight + yspd >= screenHeight) {
            y = screenHeight - playerHeight;
            yspd = 0;
        }
        else if (y + yspd <= 0) {
            y = 0;
            yspd = 0;
        }
        else {
            y += yspd;
        }

        int isMoving = (xspd != 0 || yspd != 0);

        if (isMoving) {
            animFrame = 0;
            animTimer = 0;

            for (int i = TRAIL_LENGTH - 1; i > 0; i--) {
                trailPositions[i] = trailPositions[i - 1];
            }
            trailPositions[0] = (Vector2){ (float)x, (float)y };
        }
        else {
            animTimer++;
            if (animTimer >= animSpeed) {
                animTimer = 0;
                animFrame = (animFrame + 1) % 8;
            }

            for (int i = 0; i < TRAIL_LENGTH; i++) {
                trailPositions[i] = (Vector2){ (float)x, (float)y };
            }
        }

        // Colisión con bloque de rebote
        if (x < reboteX + reboteWidth &&
            x + playerWidth > reboteX &&
            y < reboteY + reboteHeight &&
            y + playerHeight > reboteY)
        {
            if (xspd != 0) {
                // Venía en horizontal → lo saca por arriba del rebote y lo lanza hacia arriba
                x = screenWidth - playerWidth;
                y = reboteY - playerHeight;  // lo saca justo encima del rebote
                xspd = 0;
                yspd = -move_spd;
                rotation = 0;
            }
            else if (yspd != 0) {
                // Venía en vertical → lo saca por la izquierda del rebote y lo lanza hacia la izquierda
                x = reboteX - playerWidth;  // lo saca justo a la izquierda del rebote
                y = screenHeight - playerHeight;
                yspd = 0;
                xspd = -move_spd;
                rotation = 270;
            }
        }

        BeginDrawing();
        ClearBackground(BLACK);

        // 1. Trail
        if (isMoving) {
            for (int i = TRAIL_LENGTH - 1; i >= 0; i--) {
                float alpha = (float)(TRAIL_LENGTH - i) / (float)TRAIL_LENGTH;
                Color tint = (Color){ 255, 255, 255, (unsigned char)(255 * alpha) };

                Texture2D currentTrail;
                if (yspd != 0) currentTrail = vertTrail;
                else currentTrail = trail;

                DrawTextureEx(currentTrail, trailPositions[i], 0.0f, scale, tint);
            }
        }

        // 2. Jugador
        Texture2D current = frames[animOrder[animFrame]];
        Rectangle source = { 0, 0, (float)current.width, (float)current.height };
        Rectangle dest = {
            (float)x + playerWidth / 2,
            (float)y + playerHeight / 2,
            (float)playerWidth,
            (float)playerHeight
        };
        Vector2 origin = { playerWidth / 2, playerHeight / 2 };
        DrawTexturePro(current, source, dest, origin, rotation + 180, WHITE);

        // 3. Rebote encima con blend additive (el negro no tapa)
        Rectangle reboteSrc = { 0, 0, (float)rebote.width, (float)rebote.height };
        Rectangle reboteDest = {
            (float)reboteX + reboteWidth / 2,
            (float)reboteY + reboteHeight / 2,
            (float)reboteWidth,
            (float)reboteHeight
        };
        Vector2 reboteOrigin = { reboteWidth / 2, reboteHeight / 2 };

        BeginBlendMode(BLEND_ADDITIVE);
        DrawTexturePro(rebote, reboteSrc, reboteDest, reboteOrigin, -90.0f, WHITE);
        EndBlendMode();

        EndDrawing();
    }

    for (int i = 0; i < 5; i++) UnloadTexture(frames[i]);
    UnloadTexture(trail);
    UnloadTexture(vertTrail);
    UnloadTexture(rebote);

    CloseWindow();
    return 0;
}