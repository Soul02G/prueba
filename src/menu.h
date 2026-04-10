#pragma once
#include "raylib.h"
#include "scenes.h"

#define MENU_FRAME_COUNT       25
#define MENU_ANIMATION_SPEED   12
#define ZOOM_FRAME_COUNT       17
#define ZOOM_ANIMATION_SPEED   1

typedef struct {
    Texture2D backgroundFrames[MENU_FRAME_COUNT];
    int       currentBackgroundFrame;
    int       backgroundFrameTimer;

    Music     backgroundMusic;
    Sound     transitionSound;

    Texture2D zoomFrames[ZOOM_FRAME_COUNT];
    int       currentZoomFrame;
    int       zoomFrameTimer;

    // 0 = esperando input
    // 1 = sonido reproduciéndose (timer activo)
    // 2 = zoom reproduciéndose
    int       transitionState;
    float     soundTimer;       // tiempo transcurrido desde que empezó el sonido
    float     soundDuration;    // duración del sonido en segundos
} MenuState;

void  MenuLoad(MenuState* menuState);
SceneType MenuUpdate(MenuState* menuState);
void  MenuDraw(const MenuState* menuState, int screenWidth, int screenHeight);
void  MenuUnload(MenuState* menuState);