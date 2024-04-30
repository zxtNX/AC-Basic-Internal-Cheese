#include "pch.h"

#include "setupHooksAndNops.h"
#include "mainHackLoop.h"
#include "altersDamages.h"

#include <cstdlib>
#include <Windows.h>


// Declare all hooks and NOPs
Hook mainHackLoopTramp;
Nop infAmmoNOP;
Hook altersDamagesDetour;

void SetupHooksAndNops()
{
	// make sure we have our module bases
	BYTE* moduleBaseAssaultCube = (BYTE*)GetModuleHandle(L"ac_client.exe");
	BYTE* moduleBaseOpenGl = (BYTE*)GetModuleHandle(L"opengl32.dll");

	// make temporary hooks that will feed their information into the blank declarations of our global hooks
	// main hack loop hook
	try {
		BYTE* mainHackLoopHookDst = moduleBaseOpenGl + 0x47ADE;
		BYTE* usedFunc = (BYTE*)MainHackLoop;
		int hookLen = 5;

		Hook mainHackLoopTrampTemp(mainHackLoopHookDst, usedFunc, hookLen);
		mainHackLoopTramp = mainHackLoopTrampTemp;
	}
	catch (const std::runtime_error& e) {
		std::string error_message = "Error creating hook: ";
		error_message += e.what();
		OutputDebugStringA(error_message.c_str()); // Output debug strings in VS 2022 console
		std::exit(EXIT_FAILURE);
	}

	// infinite ammo nop
	try {
		BYTE* infAmmoNopDst = moduleBaseAssaultCube + 0x637E9;
		int nopLen = 2;

		Nop infAmmoNopTemp(infAmmoNopDst, nopLen);
		infAmmoNOP = infAmmoNopTemp;
	}
	catch (const std::runtime_error& e) {
		std::string error_message = "Error creating hook: ";
		error_message += e.what();
		OutputDebugStringA(error_message.c_str()); // Output debug strings in VS 2022 console
		std::exit(EXIT_FAILURE);
	}

	// alters damages detour hook
	try {
		BYTE* altersDamagesDst = moduleBaseAssaultCube + 0x29D1F;
		BYTE* usedFunc = (BYTE*)AltersDamagesTeamDetour;
		int hookLen = 5;

		Hook altersDamagesDetourTemp(altersDamagesDst, usedFunc, hookLen);
		altersDamagesDetour = altersDamagesDetourTemp;
	}
	catch (const std::runtime_error& e) {
		std::string error_message = "Error creating hook: ";
		error_message += e.what();
		OutputDebugStringA(error_message.c_str()); // Output debug strings in VS 2022 console
		std::exit(EXIT_FAILURE);
	}
}

