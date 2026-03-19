#include "raylib.h"
#include "scenes.h"
#include "game.h"
#include "menu.h"

int main() {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Tomb of The Mask");
    SetTargetFPS(120);

    GameState gameState = { 0 };
    MenuState menuState = { 0 };

    Scene currentScene = SCENE_MENU;
    MenuLoad(&menuState);

    while (!WindowShouldClose()) {

        // Update
        Scene nextScene = currentScene;
        switch (currentScene) {
        case SCENE_MENU: nextScene = MenuUpdate(&menuState); break;
        case SCENE_GAME: nextScene = GameUpdate(&gameState); break;
        }

        // Transición de escena
        if (nextScene != currentScene) {
            switch (currentScene) {
            case SCENE_MENU: MenuUnload(&menuState); break;
            case SCENE_GAME: GameUnload(&gameState); break;
            }
            switch (nextScene) {
            case SCENE_MENU: MenuLoad(&menuState);   break;
            case SCENE_GAME: GameLoad(&gameState);   break;
            }
            currentScene = nextScene;
        }

        // Draw
        BeginDrawing();
        switch (currentScene) {
        case SCENE_MENU: MenuDraw(&menuState, SCREEN_WIDTH, SCREEN_HEIGHT); break;
        case SCENE_GAME: GameDraw(&gameState);                              break;
        }
        EndDrawing();
    }

    // Limpieza
    switch (currentScene) {
    case SCENE_MENU: MenuUnload(&menuState); break;
    case SCENE_GAME: GameUnload(&gameState); break;
    }

    CloseWindow();
    return 0;
}