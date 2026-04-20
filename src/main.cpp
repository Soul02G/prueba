#include "raylib.h"
#include "scenes.h"
#include "map.h"
#include "game.h"
#include "menu.h"

int main() {
    // --- CONFIGURACIÓN INICIAL ---
    // Usamos las constantes definidas en game.h para mantener consistencia
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Tomb of The Mask");
    SetTargetFPS(120);
    SetAudioStreamBufferSizeDefault(2048);
    InitAudioDevice();

    // --- INICIALIZACIÓN DE ESTADOS ---
    GameState gameState = { 0 };
    MenuState menuState = { 0 };
    MapState  mapState = { 0 };

    SceneType currentScene = SCENE_MENU;

    // Carga inicial de recursos compartidos o permanentes
    MenuLoad(&menuState);
    MapLoad(&mapState);

    while (!WindowShouldClose()) {
        // --- ACTUALIZACIÓN ---
        SceneType nextScene = currentScene;

        switch (currentScene) {
        case SCENE_MENU:
            nextScene = MenuUpdate(&menuState);
            break;
        case SCENE_MAP:
            nextScene = MapUpdate(&mapState);
            break;
        case SCENE_GAME:
            // CORRECCIÓN: Ahora GameUpdate solo recibe gameState según game.h
            nextScene = GameUpdate(&gameState, &mapState);
            break;
        case SCENE_SETTINGS:
            nextScene = MapUpdate(&mapState);
            break;
        default: break;
        }

        // --- GESTIÓN DE CAMBIO DE ESCENA ---
        if (nextScene != currentScene) {

            // 1. Lógica de guardado al salir del juego
            if (currentScene == SCENE_GAME && gameState.levelCompleted) {
                MapRegisterLevelComplete(&mapState, mapState.selectedLevel, gameState.starsCollected, gameState.score / 10);
            }

            // 2. Descarga de la escena que termina (solo si es necesario)
            if (currentScene == SCENE_GAME) GameUnload(&gameState);
            // Nota: El menú y el mapa suelen mantenerse cargados para transiciones rápidas

            // 3. Preparación y Carga de la nueva escena
            switch (nextScene) {
            case SCENE_GAME:
                // Sincronizamos datos del selector de niveles al juego
                gameState.currentLevel = mapState.selectedLevel;
                gameState.masterVolume = mapState.masterVolume;
                gameState.musicEnabled = mapState.musicEnabled;
                GameLoad(&gameState);
                break;
            case SCENE_MENU:
                // Si el menú se hubiera descargado, se recargaría aquí
                break;
            default: break;
            }

            currentScene = nextScene;
        }

        // --- DIBUJADO ---
        BeginDrawing();
        ClearBackground(BLACK); // Aseguramos limpieza de frame

        switch (currentScene) {
        case SCENE_MENU:     MenuDraw(&menuState, SCREEN_WIDTH, SCREEN_HEIGHT); break;
        case SCENE_MAP:      MapDraw(&mapState, SCREEN_WIDTH, SCREEN_HEIGHT);  break;
        case SCENE_GAME:     GameDraw(&gameState);                              break;
        case SCENE_SETTINGS: MapDraw(&mapState, SCREEN_WIDTH, SCREEN_HEIGHT);  break;
        }

        EndDrawing();
    }

    // --- LIMPIEZA FINAL ---
    // Descargamos todo lo que esté activo
    if (currentScene == SCENE_GAME) GameUnload(&gameState);

    MenuUnload(&menuState);
    MapUnload(&mapState);

    CloseAudioDevice();
    CloseWindow();

    return 0;
}