#include "menu.h"
#include <stdio.h>

void MenuLoad(MenuState* ms) {
    char path[256];
    for (int i = 0; i < MENU_FRAME_COUNT; i++) {
        sprintf(path, "resources\\frame_%02d.png", i + 1);
        ms->frames[i] = LoadTexture(path);
    }
    ms->currentFrame = 0;
    ms->frameTimer = 0;

    InitAudioDevice();
    ms->music = LoadMusicStream("cosas de la wiki\\main screen track.wav");
    PlayMusicStream(ms->music);
}

Scene MenuUpdate(MenuState* ms) {
    UpdateMusicStream(ms->music);

    ms->frameTimer++;
    if (ms->frameTimer >= MENU_ANIM_SPEED) {
        ms->frameTimer = 0;
        ms->currentFrame = (ms->currentFrame + 1) % MENU_FRAME_COUNT;
    }

    if (GetKeyPressed() != 0)
        return SCENE_GAME;

    return SCENE_MENU;
}

void MenuDraw(const MenuState* ms, int screenWidth, int screenHeight) {
    ClearBackground(BLACK);

    Texture2D frame = ms->frames[ms->currentFrame];

    float scale = (float)screenWidth / (float)frame.width;
    float visibleH = (float)screenHeight / scale;
    float cropOffsetY = (frame.height - visibleH) / 2.0f;
    if (cropOffsetY < 0) cropOffsetY = 0;

    Rectangle src = { 0, cropOffsetY, (float)frame.width, visibleH };
    Rectangle dest = { 0, 0, (float)screenWidth, (float)screenHeight };
    Vector2 origin = { 0, 0 };

    DrawTexturePro(frame, src, dest, origin, 0.0f, WHITE);
}

void MenuUnload(MenuState* ms) {
    StopMusicStream(ms->music);
    UnloadMusicStream(ms->music);
    CloseAudioDevice();

    for (int i = 0; i < MENU_FRAME_COUNT; i++)
        UnloadTexture(ms->frames[i]);
}