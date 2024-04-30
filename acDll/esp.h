#pragma once
#include "structs.h"

void SetupGL();
void DrawEverything(std::vector<EntityInfo>& entityInfos, Entity* localPlayer, float maxFov);
void DrawBoxAndSnaplines(EntityInfo& info, Entity* localPlayer);
void DrawSnaplines(EntityInfo& info, Entity* localPlayer, float boxBottomY);
void DrawCircle(float cx, float cy, float r, int num_segments);
void DrawCenterCircle(float cx, float cy, float r, int num_segments);
void DrawHealthBar(EntityInfo& info, Entity* localPlayer, float distanceAdjustment, float topAdjustment);
bool WorldToScreen(EntityInfo& info);

extern int viewport[4];
extern bool bEspStatus;
