#include "raylib.h"
#include "scenes.h"
#include "map.h"
#include "game.h"
#include "menu.h"
#include "intro.h"

int main() {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Tomb of The Mask");
    SetTargetFPS(120);
    SetAudioStreamBufferSizeDefault(2048);
    InitAudioDevice();

    GameState gameState = { 0 };
    MenuState menuState = { 0 };
    MapState  mapState = { 0 };
    SceneType currentScene = SCENE_CREDITS;

    MenuLoad(&menuState);
    MapLoad(&mapState);

    while (!WindowShouldClose()) {
        SceneType nextScene = currentScene;

        if (currentScene == SCENE_CREDITS) {
            UpdateDrawCredits(&currentScene);
            nextScene = currentScene;
        }
        else {
            switch (currentScene) {
            case SCENE_MENU:
                nextScene = MenuUpdate(&menuState);
                break;
            case SCENE_MAP:
                nextScene = MapUpdate(&mapState);
                break;
            case SCENE_GAME:
                nextScene = GameUpdate(&gameState, &mapState);

                if (gameState.levelCompleted && !mapState.levels[mapState.selectedLevel].completed) {
                    MapAddCoins(&mapState, gameState.coinsCollected);
                    MapRegisterLevelComplete(&mapState, mapState.selectedLevel, gameState.starsCollected, 0);
                }

                break;
            case SCENE_SETTINGS:
                nextScene = MapUpdate(&mapState);
                break;
            default: break;
            }
        }

        if (nextScene != currentScene) {
            if (currentScene == SCENE_GAME) {
                GameUnload(&gameState);
                ResetGameState(&gameState);
            }
            switch (nextScene) {
            case SCENE_GAME:
                gameState.currentLevel = mapState.selectedLevel;
                gameState.masterVolume = mapState.masterVolume;
                gameState.musicEnabled = mapState.musicEnabled;
                GameLoad(&gameState);
                break;
            case SCENE_MENU:
                break;
            default: break;
            }
            currentScene = nextScene;
        }

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

    if (currentScene == SCENE_GAME) GameUnload(&gameState);
    MenuUnload(&menuState);
    MapUnload(&mapState);
    CloseAudioDevice();
    CloseWindow();
    return 0;
}