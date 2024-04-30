#pragma once
#include "memory.h"

extern Hook mainHackLoopTramp;
extern Nop infAmmoNOP;
extern Hook altersDamagesDetour;

void SetupHooksAndNops();

