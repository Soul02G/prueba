#include "intro.h"

void UpdateDrawCredits(SceneType* currentScene)
{
    const char* nombres[5] = {
        "Sergio Garcia Perez",
        "Jordi Fernandez Prat",
        "Sergio Andres Llanos Orozco",
        "Andrea Vasco Peiro",
        "Vladimir Solovev"
    };

    // Input: cualquier tecla común cambia a la escena del Menú (o Gameplay)
    if (IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_ENTER) ||
        IsKeyPressed(KEY_W) || IsKeyPressed(KEY_A) ||
        IsKeyPressed(KEY_S) || IsKeyPressed(KEY_D) ||
        IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_DOWN) ||
        IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_RIGHT))
    {
        // Normalmente la intro va al MENU, pero si quieres ir directo al juego:
        *currentScene = SCENE_MENU;
        return;
    }

    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();

    BeginDrawing();
    ClearBackground(BLACK);

    int fontSize = 30;
    int spacing = 50;
    int totalH = 5 * spacing;
    int startY = (screenHeight - totalH) / 2;

    for (int i = 0; i < 5; i++)
    {
        int textW = MeasureText(nombres[i], fontSize);
        DrawText(nombres[i],
            (screenWidth - textW) / 2,
            startY + i * spacing,
            fontSize, WHITE);
    }

    DrawText("Presiona ESPACIO para continuar",
        screenWidth / 2 - MeasureText("Presiona ESPACIO para continuar", 20) / 2,
        screenHeight - 60, 20, GRAY);

    EndDrawing();
}