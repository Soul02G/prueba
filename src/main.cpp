#include "raylib.h"
#include "scenes.h"
#include "game.h"
#include "menu.h"
#include "map.h"

int main() {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Tomb of The Mask");
    SetTargetFPS(120);
    SetAudioStreamBufferSizeDefault(2048);
    InitAudioDevice();

    GameState gameState = { 0 };
    MenuState menuState = { 0 };
    MapState  mapState = { 0 };

    SceneType currentScene = SCENE_MENU;
    MenuLoad(&menuState);
    MapLoad(&mapState);

    while (!WindowShouldClose()) {

        // Draw primero para que el último frame se vea antes de cambiar escena
        BeginDrawing();
        switch (currentScene) {
        case SCENE_MENU:     MenuDraw(&menuState, SCREEN_WIDTH, SCREEN_HEIGHT); break;
        case SCENE_MAP:      MapDraw(&mapState, SCREEN_WIDTH, SCREEN_HEIGHT); break;
        case SCENE_GAME:     GameDraw(&gameState);                              break;
        case SCENE_SETTINGS: MapDraw(&mapState, SCREEN_WIDTH, SCREEN_HEIGHT); break;
        }
        EndDrawing();

        // Update
        SceneType nextScene = currentScene;
        switch (currentScene) {
        case SCENE_MENU:     nextScene = MenuUpdate(&menuState);  break;
        case SCENE_MAP:      nextScene = MapUpdate(&mapState);    break;
        case SCENE_GAME:     nextScene = GameUpdate(&gameState);  break;
        case SCENE_SETTINGS: nextScene = MapUpdate(&mapState);    break;
        }

        // Transición de escena
        if (nextScene != currentScene) {
            // Cuando el juego termina, registra el progreso en el mapa
            if (currentScene == SCENE_GAME && gameState.levelCompleted) {
                MapRegisterLevelComplete(&mapState,
                    mapState.selectedLevel,
                    gameState.starsCollected,
                    gameState.score / 10);
            }

            switch (currentScene) {
            case SCENE_MENU: MenuUnload(&menuState); break;
            case SCENE_GAME: GameUnload(&gameState); break;
            default: break;
            }
            switch (nextScene) {
            case SCENE_MENU: MenuLoad(&menuState); break;
            case SCENE_GAME: GameLoad(&gameState); break;
            default: break;
            }
            currentScene = nextScene;
        }
    }

    // Limpieza
    switch (currentScene) {
    case SCENE_MENU: MenuUnload(&menuState); break;
    case SCENE_GAME: GameUnload(&gameState); break;
    default: break;
    }
    MapUnload(&mapState);

    CloseAudioDevice();
    CloseWindow();
    return 0;
}