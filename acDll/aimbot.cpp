#include "pch.h"
#include "aimbot.h"
#include "globals.h"

#include <cmath>
#include <random>

#define findTargetOffset 0x8ABD0


bool bAimbotStatus = false;

template<typename T>
constexpr const T& clamp(const T& value, const T& min, const T& max) {
    return (value < min) ? min : ((value > max) ? max : value);
}

// Défini une constante pour la vitesse de transition de l'aimbot
const float AIM_SPEED = 0.1f; // 0.1 radians par mise à jour
const float MAX_ANGULAR_SPEED = 5.0f;
const float AIM_VARIABILITY = 0.07f; // Variabilité de l'aim en degrés
const float AIM_PAUSE_PROBABILITY = 0.1f; // Probabilité de faire une pause dans l'aim

bool BeTargetable(float toZ, float toY, float toX, float fromZ, float fromY, float fromX) {
    bool bTargetable = false;
    void* FindTarget = (void*)(gModuleBaseAssaultCube + findTargetOffset);
    __asm {
        push toZ
        push toY
        push toX
        push fromZ
        push fromY
        push fromX
        mov eax, 0
        mov cl, 0
        call FindTarget
        mov[bTargetable], al
        add esp, 0x18
    }
    return bTargetable;
}

float NormalizeAngle(float angle) {
    while (angle <= -180.0f) {
        angle += 360.0f;
    }
    while (angle > 180.0f) {
        angle -= 360.0f;
    }
    return angle;
}

float CalculateFOVAngle(Entity* localPlayer, float relX, float relY, float relZ) {
    float angleXY = static_cast<float>(atan2(relY, relX) * 180.0 / gPI + 90.0);  // Angle XY par rapport à l'axe X
    float angleZ = static_cast<float>(atan2(relZ, sqrt(relX * relX + relY * relY)) * 180.0 / gPI);  // Angle Z par rapport à XY

    float adjustedAngleXY = static_cast<float>(fmod(angleXY - localPlayer->yaw + 360.0, 360.0));
    if (adjustedAngleXY > 180.0)
        adjustedAngleXY -= 360.0;  // Normaliser entre -180 et 180

    float adjustedAngleZ = static_cast<float>(fmod(angleZ - localPlayer->pitch + 360.0, 360.0));
    if (adjustedAngleZ > 180.0)
        adjustedAngleZ -= 360.0;  // Normaliser entre -180 et 180

    // Calculer l'angle de vue en combinant l'angle XY et l'angle Z
    float fovAngle = static_cast<float>(sqrt(adjustedAngleXY * adjustedAngleXY + adjustedAngleZ * adjustedAngleZ));
    return fabs(fovAngle);
}

void Aimbot(Entity* closestEnemy, Entity* localPlayer)
{
    float relX = closestEnemy->xHeadCoord - localPlayer->xHeadCoord;
    float relY = closestEnemy->yHeadCoord - localPlayer->yHeadCoord;
    float relZ = closestEnemy->zHeadCoord - localPlayer->zHeadCoord;

    // Check if the target is obstructed by an obstacle (e.g. a wall)
    bool isObstructed = !BeTargetable(closestEnemy->zHeadCoord, closestEnemy->yHeadCoord, closestEnemy->xHeadCoord,
        localPlayer->zHeadCoord, localPlayer->yHeadCoord, localPlayer->xHeadCoord);

    if (!isObstructed) {
        // Calculate target angles
        float tempYaw = atan2(relY, relX) * 180 / gPI + 90;
        float tempPitch = atan2(relZ, sqrt(relX * relX + relY * relY)) * 180 / gPI;

        tempYaw = NormalizeAngle(tempYaw);
        tempPitch = NormalizeAngle(tempPitch);

        // Add variability to the aim
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> variabilityDistribution(-AIM_VARIABILITY, AIM_VARIABILITY);
        tempYaw += variabilityDistribution(gen);
        tempPitch += variabilityDistribution(gen);

        // Calculate the angle difference between current orientation and target
        float yawDiff = tempYaw - localPlayer->yaw;
        float pitchDiff = tempPitch - localPlayer->pitch;

        // Normalize angle differences
        yawDiff = NormalizeAngle(yawDiff);
        pitchDiff = NormalizeAngle(pitchDiff);

        // Limit maximum angular speed
        yawDiff = clamp(yawDiff, -MAX_ANGULAR_SPEED, MAX_ANGULAR_SPEED);
        pitchDiff = clamp(pitchDiff, -MAX_ANGULAR_SPEED, MAX_ANGULAR_SPEED);

        // Apply interpolation to smooth the aiming movement
        localPlayer->yaw += yawDiff * AIM_SPEED;
        localPlayer->pitch += pitchDiff * AIM_SPEED;

        // Add a random pause in aiming
        std::uniform_real_distribution<float> pauseDistribution(0.0f, 1.0f);
        if (pauseDistribution(gen) < AIM_PAUSE_PROBABILITY)
        {
            localPlayer->yaw += yawDiff * AIM_SPEED * 0.1f; // Reduce speed to simulate pause
            localPlayer->pitch += pitchDiff * AIM_SPEED * 0.1f;
        }
    }
}
