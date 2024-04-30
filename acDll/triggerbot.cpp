#include "pch.h"
#include "triggerbot.h"
#include "globals.h"

bool bTriggerbotStatus = false;

typedef Entity* (__cdecl* tGetCrossHairEnt)();
tGetCrossHairEnt GetCrossHairEnt = nullptr;

void Triggerbot(Entity* localPlayer)
{
    GetCrossHairEnt = (tGetCrossHairEnt)(gModuleBaseAssaultCube + 0x607c0);
    Entity* crossHairEnt = GetCrossHairEnt();

    if (crossHairEnt) {
        if (localPlayer->team != crossHairEnt->team)
        {
            localPlayer->bAttack = 1;
        }
    }
    else
    {
        localPlayer->bAttack = 0;
    }
}