#include "menu.h"
#include <stdio.h>

void MenuLoad(MenuState* menuState) {
    char filePath[256];

    for (int i = 0; i < MENU_FRAME_COUNT; i++) {
        sprintf(filePath, "resources\\frame_%02d.png", i + 1);
        menuState->backgroundFrames[i] = LoadTexture(filePath);
    }

    for (int i = 0; i < ZOOM_FRAME_COUNT; i++) {
        sprintf(filePath, "resources\\zoom_%02d.png", i + 1);
        menuState->zoomFrames[i] = LoadTexture(filePath);
    }

    menuState->currentBackgroundFrame = 0;
    menuState->backgroundFrameTimer = 0;
    menuState->currentZoomFrame = 0;
    menuState->zoomFrameTimer = 0;
    menuState->transitionState = 0;
    menuState->soundTimer = 0.0f;
    menuState->soundDuration = 1.10f; // duracion exacta del wav

    menuState->backgroundMusic = LoadMusicStream("cosas de la wiki\\main screen track.wav");
    menuState->transitionSound = LoadSound("cosas de la wiki\\zoom in start the game.wav");
    PlayMusicStream(menuState->backgroundMusic);
}

SceneType MenuUpdate(MenuState* menuState) {
    float dt = GetFrameTime();
    UpdateMusicStream(menuState->backgroundMusic);

    // Avanza animación de fondo siempre
    menuState->backgroundFrameTimer++;
    if (menuState->backgroundFrameTimer >= MENU_ANIMATION_SPEED) {
        menuState->backgroundFrameTimer = 0;
        menuState->currentBackgroundFrame = (menuState->currentBackgroundFrame + 1) % MENU_FRAME_COUNT;
    }

    // Estado 0: esperando input
    if (menuState->transitionState == 0) {
        if (GetKeyPressed() != 0) {
            StopMusicStream(menuState->backgroundMusic);
            PlaySound(menuState->transitionSound);
            menuState->currentZoomFrame = 0;
            menuState->zoomFrameTimer = 0;
            menuState->transitionState = 1;
        }
        return SCENE_MENU;
    }

    // Estado 1: sonido y zoom arrancan a la vez
    if (menuState->transitionState == 1) {
        menuState->zoomFrameTimer++;
        if (menuState->zoomFrameTimer >= ZOOM_ANIMATION_SPEED) {
            menuState->zoomFrameTimer = 0;
            menuState->currentZoomFrame++;
        }
        if (menuState->currentZoomFrame >= ZOOM_FRAME_COUNT)
            return SCENE_MAP;
    }

    return SCENE_MENU;
}

void MenuDraw(const MenuState* menuState, int screenWidth, int screenHeight) {
    ClearBackground(BLACK);

    // Fondo animado
    Texture2D bg = menuState->backgroundFrames[menuState->currentBackgroundFrame];
    float scaleX = (float)screenWidth / (float)bg.width;
    float scaleY = (float)screenHeight / (float)bg.height;
    float scale = (scaleX < scaleY) ? scaleX : scaleY;
    float visibleH = (float)screenHeight / scale;
    float cropOffsetY = (bg.height - visibleH) / 2.0f;
    if (cropOffsetY < 0) cropOffsetY = 0;

    DrawTexturePro(bg,
        { 0, cropOffsetY, (float)bg.width, visibleH },
        { 0, 0, (float)screenWidth, (float)screenHeight },
        { 0, 0 }, 0.0f, WHITE);

    // Zoom desde estado 1
    if (menuState->transitionState == 1 && menuState->currentZoomFrame < ZOOM_FRAME_COUNT) {
        Texture2D zf = menuState->zoomFrames[menuState->currentZoomFrame];
        float zscaleX = (float)screenWidth / (float)zf.width;
        float zscaleY = (float)screenHeight / (float)zf.height;
        float zscale = (zscaleX > zscaleY) ? zscaleX : zscaleY;
        float zvisibleW = (float)screenWidth / zscale;
        float zvisibleH = (float)screenHeight / zscale;
        float zcropX = (zf.width - zvisibleW) / 2.0f;
        float zcropY = (zf.height - zvisibleH) / 2.0f;
        if (zcropX < 0) zcropX = 0;
        if (zcropY < 0) zcropY = 0;

        DrawTexturePro(zf,
            { zcropX, zcropY, zvisibleW, zvisibleH },
            { 0, 0, (float)screenWidth, (float)screenHeight },
            { 0, 0 }, 0.0f, WHITE);
    }
}

void MenuUnload(MenuState* menuState) {
    StopMusicStream(menuState->backgroundMusic);
    UnloadMusicStream(menuState->backgroundMusic);
    UnloadSound(menuState->transitionSound);
    for (int i = 0; i < MENU_FRAME_COUNT; i++) UnloadTexture(menuState->backgroundFrames[i]);
    for (int i = 0; i < ZOOM_FRAME_COUNT; i++) UnloadTexture(menuState->zoomFrames[i]);
}