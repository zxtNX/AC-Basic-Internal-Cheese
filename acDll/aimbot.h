#pragma once
#include "structs.h"

float NormalizeAngle(float angle);
void Aimbot(Entity* closestEnemy, Entity* localPlayer);
float CalculateFOVAngle(Entity* localPlayer, float relX, float relY, float relZ);
bool BeTargetable(float toZ, float toY, float toX, float fromZ, float fromY, float fromX);

extern bool bAimbotStatus;
