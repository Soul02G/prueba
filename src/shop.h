#pragma once
#include "scenes.h"

struct MapState;

void ShopLoad(struct MapState* mapState);
SceneType ShopUpdate(struct MapState* mapState);
void ShopDraw(const struct MapState* mapState, int screenWidth, int screenHeight);
void ShopUnload(void);