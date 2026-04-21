#include "intro.h"

static int introPhase = 0;
static Texture2D texLogo = { 0 };

void UpdateDrawCredits(SceneType* currentScene)
{
    // Cargar el logo solo una vez
    if (texLogo.id == 0)
        texLogo = LoadTexture("resources/pangallin_rojo.png");

    const char* nombres[5] = {
        "Sergio Garcia Perez",
        "Jordi Fernandez Prat",
        "Sergio Andres Llanos Orozco",
        "Andrea Vasco Peiro",
        "Vladimir Solovev"
    };

    const char* descLines[7] = {
        "This project is a recreation of the original video game",
        "Tomb of the Mask, which is an arcade-style labyrinth game",
        "where the difficulty increases with each level.",
        "The gameplay revolves around a dash mechanic that allows",
        "the player to move from wall to wall. Players must collect",
        "coins and stars, outsmart deadly traps like spike walls,",
        "and dodge enemies to reach the finish line."
    };

    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();

    bool anyKey = IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_ENTER) ||
        IsKeyPressed(KEY_W) || IsKeyPressed(KEY_A) ||
        IsKeyPressed(KEY_S) || IsKeyPressed(KEY_D) ||
        IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_DOWN) ||
        IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_RIGHT);

    if (anyKey)
    {
        if (introPhase == 0)
            introPhase = 1;
        else
        {
            introPhase = 0;
            UnloadTexture(texLogo);
            texLogo = { 0 };
            *currentScene = SCENE_MENU;
        }
    }

    BeginDrawing();
    ClearBackground(BLACK);

    if (introPhase == 0)
    {
        // --- PANG-ALL-IN STUDIO ---
        const char* studioName = "PANG-ALL-IN STUDIO";
        DrawText(studioName,
            (screenWidth - MeasureText(studioName, 26)) / 2,
            screenHeight / 2 - 280, 26, WHITE);

        // Logo debajo del nombre del estudio
        if (texLogo.id > 0)
        {
            float logoSize = 80.0f;
            float logoX = (screenWidth - logoSize) / 2.0f;
            float logoY = screenHeight / 2.0f - 248.0f;
            DrawTexturePro(texLogo,
                { 0, 0, (float)texLogo.width, (float)texLogo.height },
                { logoX, logoY, logoSize, logoSize },
                { 0, 0 }, 0.0f, WHITE);
        }

        // Separador
        DrawLine(screenWidth / 2 - 150, screenHeight / 2 - 155,
            screenWidth / 2 + 150, screenHeight / 2 - 155,
            Color{ 80, 80, 80, 255 });

        // Líneas de título
        const char* line1 = "PRIMERO DE DISENO Y DESARROLLO DE VIDEOJUEGOS";
        DrawText(line1,
            (screenWidth - MeasureText(line1, 18)) / 2,
            screenHeight / 2 - 148, 18, GRAY);

        const char* line2 = "PROYECTO 1";
        DrawText(line2,
            (screenWidth - MeasureText(line2, 18)) / 2,
            screenHeight / 2 - 122, 18, GRAY);

        const char* line3 = "Tutor: ALEJANDRO PARIS";
        DrawText(line3,
            (screenWidth - MeasureText(line3, 18)) / 2,
            screenHeight / 2 - 96, 18, GRAY);

        // Separador
        DrawLine(screenWidth / 2 - 150, screenHeight / 2 - 72,
            screenWidth / 2 + 150, screenHeight / 2 - 72,
            Color{ 80, 80, 80, 255 });

        // Nombres
        int fontSize = 24;
        int spacing = 42;
        int startY = screenHeight / 2 - 58;

        for (int i = 0; i < 5; i++)
        {
            DrawText(nombres[i],
                (screenWidth - MeasureText(nombres[i], fontSize)) / 2,
                startY + i * spacing,
                fontSize, WHITE);
        }
    }
    else
    {
        // --- FASE 1: Descripción ---
        int fontSize = 19;
        int spacing = 34;
        int totalH = 7 * spacing;
        int startY = (screenHeight - totalH) / 2 - 20;

        for (int i = 0; i < 7; i++)
        {
            DrawText(descLines[i],
                (screenWidth - MeasureText(descLines[i], fontSize)) / 2,
                startY + i * spacing,
                fontSize, WHITE);
        }
    }

    DrawText("Presiona ESPACIO para continuar",
        (screenWidth - MeasureText("Presiona ESPACIO para continuar", 18)) / 2,
        screenHeight - 55, 18, GRAY);

    EndDrawing();
}