#include "pch.h"
#include "mainHackLoop.h"
#include "consoleAndInput.h"
#include "globals.h"
#include "structs.h"
#include "aimbot.h"
#include "triggerbot.h"
#include "esp.h"

#define numberOfPlayers *(int*)(gModuleBaseAssaultCube + 0x10F500)
#define everyoneButMe 1
#define MAX_FOV 10.0f

uintptr_t* entList = (uintptr_t*)(gModuleBaseAssaultCube + 0x10F4F8);
Entity* localPlayer = *(Entity**)(gModuleBaseAssaultCube + 0x10F4F4);

std::vector<EntityInfo> entityInfos;

bool bTargetLocked = false;
Entity* lockedTarget = nullptr;

void MainHackLoop()
{
    int currentNumberOfPlayers = numberOfPlayers;
    if (gNumberOfPlayers != currentNumberOfPlayers) {
        gNumberOfPlayers = currentNumberOfPlayers;
    }

    entityInfos.clear(); // Nettoyer la liste à chaque mise à jour pour éviter des données obsolètes
    for (int i = everyoneButMe; i < gNumberOfPlayers; i++) {
        Entity* ent = *(Entity**)(*entList + (i * sizeof(uintptr_t)));

        float relX = ent->xHeadCoord - localPlayer->xHeadCoord;
        float relY = ent->yHeadCoord - localPlayer->yHeadCoord;
        float relZ = ent->zHeadCoord - localPlayer->zHeadCoord;
        float distance = static_cast<float>(sqrt(relX * relX + relY * relY + relZ * relZ));
        float fovAngle = CalculateFOVAngle(localPlayer, relX, relY, relZ);

        bool targetable = false;
        bool isInFOV = false;

        EntityInfo info = { ent, distance, targetable, isInFOV };

        if (ent && !ent->bDead && ent->team != localPlayer->team) {
            if (fovAngle <= MAX_FOV)
                isInFOV = true;

            targetable = BeTargetable(localPlayer->zHeadCoord, localPlayer->yHeadCoord, localPlayer->xHeadCoord,
                ent->zHeadCoord, ent->yHeadCoord, ent->xHeadCoord);

            info.isTargetable = targetable;
            info.isInFOV = isInFOV;
        }
        entityInfos.push_back(info);
    }

    
    Entity* bestTarget = nullptr;
    float smallestAngle = FLT_MAX;

    for (const auto& info : entityInfos) {
        if (info.isTargetable && info.isInFOV) {
            float relX = info.entity->xHeadCoord - localPlayer->xHeadCoord;
            float relY = info.entity->yHeadCoord - localPlayer->yHeadCoord;
            float relZ = info.entity->zHeadCoord - localPlayer->zHeadCoord;
            // Calculer l'angle entre la direction du regard du joueur local et la direction de la cible
            float angleToTarget = CalculateFOVAngle(localPlayer, relX, relY, relZ);

            if (angleToTarget < smallestAngle) {
                smallestAngle = angleToTarget;
                bestTarget = info.entity;
            }
        }
    }

    if (!(GetAsyncKeyState(VK_XBUTTON2) & 0x8000)) {
        bTargetLocked = false;
    }

    if (bAimbotStatus && (GetAsyncKeyState(VK_XBUTTON2) & 0x8000)) {
        if (!bTargetLocked && bestTarget) {  // Vérifier si aucune cible n'est verrouillée et qu'il y a une cible à verrouiller
            bTargetLocked = true;
            lockedTarget = bestTarget;
        }
    }

    if (bTriggerbotStatus && (GetAsyncKeyState(VK_XBUTTON1) & 0x8000)) {
        if (!bTargetLocked && bestTarget) {  // Vérifiez si aucune cible n'est verrouillée, qu'il y a une cible à verrouiller et qu'elle est dans le FOV
            bTargetLocked = true;
            lockedTarget = bestTarget;
        }
    }

    if (bTargetLocked && lockedTarget && (GetAsyncKeyState(VK_XBUTTON2) & 0x8000)) {  // Si une cible est verrouillée
        Aimbot(lockedTarget, localPlayer);
    }

    if ((GetAsyncKeyState(VK_XBUTTON1) & 0x8000)) {
        Triggerbot(localPlayer);
    }

    if (bEspStatus) {
        DrawEverything(entityInfos, localPlayer, MAX_FOV);
    }

    GetInput();
}