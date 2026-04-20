#include "raylib.h"
#include "scenes.h"
#include "map.h"
#include "game.h"
#include "menu.h"
#include "intro.h"

int main() {
    // --- CONFIGURACIÓN INICIAL ---
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Tomb of The Mask");
    SetTargetFPS(120);
    SetAudioStreamBufferSizeDefault(2048);
    InitAudioDevice();

    // --- INICIALIZACIÓN DE ESTADOS ---
    GameState gameState = { 0 };
    MenuState menuState = { 0 };
    MapState  mapState = { 0 };

    // Empezamos en los créditos
    SceneType currentScene = SCENE_CREDITS;

    // Carga inicial de recursos para que el Menú esté listo al pulsar espacio
    MenuLoad(&menuState);
    MapLoad(&mapState);

    while (!WindowShouldClose()) {

        SceneType nextScene = currentScene;

        // --- 1. LÓGICA / ACTUALIZACIÓN ---
        if (currentScene == SCENE_CREDITS) {
            // UpdateDrawCredits gestiona su propio dibujado y actualiza la escena a MENU si se pulsa tecla
            UpdateDrawCredits(&currentScene);
            nextScene = currentScene;
        }
        else {
            // Actualización para el resto de escenas
            switch (currentScene) {
            case SCENE_MENU:
                nextScene = MenuUpdate(&menuState);
                break;
            case SCENE_MAP:
                nextScene = MapUpdate(&mapState);
                break;
            case SCENE_GAME:
                nextScene = GameUpdate(&gameState, &mapState);
                break;
            case SCENE_SETTINGS:
                nextScene = MapUpdate(&mapState);
                break;
            default: break;
            }
        }

        // --- 2. GESTIÓN DE CAMBIO DE ESCENA ---
        if (nextScene != currentScene) {

            // Lógica al SALIR de una escena
            if (currentScene == SCENE_GAME && gameState.levelCompleted) {
                MapRegisterLevelComplete(&mapState, mapState.selectedLevel, gameState.starsCollected, gameState.score / 10);
            }
            if (currentScene == SCENE_GAME) {
                GameUnload(&gameState);
            }

            // Lógica al ENTRAR en una escena nueva
            switch (nextScene) {
            case SCENE_GAME:
                gameState.currentLevel = mapState.selectedLevel;
                gameState.masterVolume = mapState.masterVolume;
                gameState.musicEnabled = mapState.musicEnabled;
                GameLoad(&gameState);
                break;
            case SCENE_MENU:
                // El menú ya está cargado por MenuLoad al inicio
                break;
            default: break;
            }

            currentScene = nextScene;
        }

        // --- 3. DIBUJADO ---
        // Solo dibujamos aquí si NO estamos en créditos (porque la intro ya dibujó arriba)
        if (currentScene != SCENE_CREDITS) {
            BeginDrawing();
            ClearBackground(BLACK);

            switch (currentScene) {
            case SCENE_MENU:
                MenuDraw(&menuState, SCREEN_WIDTH, SCREEN_HEIGHT);
                break;
            case SCENE_MAP:
                MapDraw(&mapState, SCREEN_WIDTH, SCREEN_HEIGHT);
                break;
            case SCENE_GAME:
                GameDraw(&gameState);
                break;
            case SCENE_SETTINGS:
                MapDraw(&mapState, SCREEN_WIDTH, SCREEN_HEIGHT);
                break;
            default: break;
            }
            EndDrawing();
        }
    }

    // --- LIMPIEZA FINAL ---
    if (currentScene == SCENE_GAME) GameUnload(&gameState);
    MenuUnload(&menuState);
    MapUnload(&mapState);

    CloseAudioDevice();
    CloseWindow();

    return 0;
}