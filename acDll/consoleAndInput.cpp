#include "pch.h"
#include "consoleAndInput.h"
#include "setupHooksAndNops.h"
#include "structs.h"
#include "globals.h"
#include "aimbot.h"
#include "triggerbot.h"
#include "esp.h"

#include <iostream>

bool bBreakHackThreadWhileLoop = false;

void GetInput()
{
	// End hack, BreakHackLoop
	if (GetAsyncKeyState(VK_END) & 1)
	{
		BreakHackLoop();
	}

	// Toggle infinite ammo NOP
	if (GetAsyncKeyState(VK_NUMPAD0) & 1)
	{
		infAmmoNOP.ToggleNop();
		PrintConsole();
	}

	// Toggle alters damage detour hook
	if (GetAsyncKeyState(VK_NUMPAD1) & 1)
	{
		altersDamagesDetour.ToggleDetour();
		PrintConsole();
	}

	// Toggle aimbot
	if (GetAsyncKeyState(VK_NUMPAD2) & 1)
	{
		bAimbotStatus = !bAimbotStatus;
		PrintConsole();
	}

	// Toggle triggerbot
	if (GetAsyncKeyState(VK_NUMPAD3) & 1)
	{
		bTriggerbotStatus = !bTriggerbotStatus;
		PrintConsole();
	}

	// Toggle ESP
	if (GetAsyncKeyState(VK_NUMPAD4) & 1)
	{
		bEspStatus = !bEspStatus;
		PrintConsole();
	}
}

void PrintConsole()
{
    // erase previous information on console
    system("CLS");

    std::cout << "Press NUMPAD0 for Infinite Ammo" << '\n';
    std::cout << "Press NUMPAD1 to alters damages that you and your team receives and damages you and your team deals" << '\n';
	std::cout << "Press NUMPAD2 to activate aimbot" << '\n';
	std::cout << "Press NUMPAD3 to activate triggerbot" << '\n';
	std::cout << "Press NUMPAD4 to activate ESP" << '\n';
    std::cout << "Press END to uninject" << '\n';

    std::cout << "====================================================================================================" << '\n';
    
    std::cout << "Infinite Ammo: " << (infAmmoNOP.bActive ? "ON" : "OFF") << '\n';
	std::cout << "Altering damages: " << (altersDamagesDetour.bActive ? "ON" : "OFF") << "\n";
	std::cout << "Aimbot: " << (bAimbotStatus ? "ON" : "OFF") << '\n';
	std::cout << "Triggerbot: " << (bTriggerbotStatus ? "ON" : "OFF") << '\n';
	std::cout << "ESP: " << (bEspStatus ? "ON" : "OFF") << std::endl;
}

void PrintPositions()
{
	system("CLS");
	uintptr_t* entList = (uintptr_t*)(gModuleBaseAssaultCube + 0x10F4F8);
	for (int i = 0; i < gNumberOfPlayers; i++)
	{
		//iterate through entList
		Entity* ent = *(Entity**)(*entList + (i * 4));
		if (ent != nullptr)
		{
			std::cout << "Entity " << i << "'s xHeadCoord address is: " << &(ent->xHeadCoord) << " and has the value of: " << ent->xHeadCoord << '\n';
			std::cout << "Entity " << i << "'s yHeadCoord address is: " << &(ent->yHeadCoord) << " and has the value of: " << ent->yHeadCoord << '\n';
			std::cout << "Entity " << i << "'s zHeadCoord address is: " << &(ent->zHeadCoord) << " and has the value of: " << ent->zHeadCoord << '\n';
		}
		else
		{
			std::cout << "Entity " << i << " is null." << std::endl;
		}
		std::cout << "=======================================================================================================" << std::endl;
	}
}

void BreakHackLoop()
{
    // make sure everything is deactivated
    if (infAmmoNOP.bActive)
        infAmmoNOP.ToggleNop();

    if (altersDamagesDetour.bActive)
        altersDamagesDetour.ToggleDetour();

	if (bAimbotStatus)
		bAimbotStatus = false;

	if (bTriggerbotStatus)
		bTriggerbotStatus = false;

	if (bEspStatus)
		bEspStatus = false;

	// toggle off main hack loop trampoline hook, then break the while loop inside the HackThread()
	mainHackLoopTramp.ToggleTrampSBF();
	bBreakHackThreadWhileLoop = true;
}

void ToggleConsole()
{
	static bool bActive = false;
	static FILE* f;
	bActive = !bActive;

	if (bActive)
	{
		AllocConsole();
		freopen_s(&f, "CONOUT$", "w", stdout);
	}

	if (!bActive)
	{
		fclose(f);
		FreeConsole();
	}
}