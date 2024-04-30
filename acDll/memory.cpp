#include "pch.h"
#include "memory.h"
#include <stdexcept>

// reach inside Hook struct to acess our Hook constructor and define that function
Hook::Hook(BYTE* hookPosition, BYTE* desiredFunction, int lengthOfHook)
{
	// use arguments to define our Hook's variables
	this->hookPosition = hookPosition;
	this->desiredFunction = desiredFunction;
	this->lengthOfHook = lengthOfHook;

	if (this->lengthOfHook < 5) {
		// If the length of the hook is less than 5, throw an exception
		throw std::runtime_error("Hook length must be at least 5 bytes.\n");
	}

	// copy stolenBytes
	memcpy(stolenBytes, hookPosition, lengthOfHook);

	// allocate room for our trampoline
	trampoline = (BYTE*)VirtualAlloc(nullptr, 40, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

	// setup tramp jumps
	hookToTrampJump = (DWORD)(trampoline - hookPosition) - 5;
	returnJump = (DWORD)(hookPosition + lengthOfHook) - (DWORD)(
		trampoline + lengthOfHook + sizeof(preserveStack) + 5 + sizeof(releaseStack)
		) - 5;

	// setup detour jumps
	hookToDetourJump = (DWORD)(desiredFunction - hookPosition) - 5;
	returnJumpDetour = (DWORD)(hookPosition + lengthOfHook);
}

Hook::Hook()
{
	// do nothing
}

void Hook::ToggleTrampSBF()
{
	// toggle hook
	bActive = !bActive;

	if (bActive)
	{
		// making it so that we can execute/read/write bytes at hook location
		DWORD oldProtection = { 0 };
		VirtualProtect(hookPosition, lengthOfHook, PAGE_EXECUTE_READWRITE, &oldProtection);

		// is the trampoline already built? If so, skip making the trampoline
		if (bTrampBuilt)goto TrampAlreadyBuilt;

		// in our trampoline, jump to our space in the DLL for the specified function
		trampToFuncCall = (DWORD)desiredFunction - (DWORD)(trampoline + lengthOfHook + sizeof(preserveStack)) - 5;

		// stolen bytes placed, stack preserved, call our desired function
		memcpy(trampoline, (void*)hookPosition, lengthOfHook);
		memcpy(trampoline + lengthOfHook, preserveStack, sizeof(preserveStack));
		memset(trampoline + lengthOfHook + sizeof(preserveStack), 0xE8, 1);
		*(DWORD*)(trampoline + lengthOfHook + sizeof(preserveStack) + 1) = trampToFuncCall;

		// release stack, jump back to after hook
		memcpy(trampoline + lengthOfHook + sizeof(preserveStack) + 5, releaseStack, sizeof(releaseStack));
		memset(trampoline + lengthOfHook + sizeof(preserveStack) + 5 + sizeof(releaseStack), 0xE9, 1);
		*(DWORD*)(trampoline + lengthOfHook + sizeof(preserveStack) + 5 + sizeof(releaseStack) + 1) = returnJump;

		// trampoline is built
		bTrampBuilt = true;

	TrampAlreadyBuilt:
		// set hook inside game's memory
		memset(hookPosition, 0x90, lengthOfHook);
		*(BYTE*)hookPosition = 0xE9;
		*(DWORD*)(hookPosition + 1) = hookToTrampJump;

		// restore protections
		VirtualProtect(hookPosition, lengthOfHook, oldProtection, nullptr);
	}

	if (!bActive)
	{
		DWORD oldProtection = { 0 };
		VirtualProtect(hookPosition, lengthOfHook, PAGE_EXECUTE_READWRITE, &oldProtection);
		memcpy(hookPosition, stolenBytes, lengthOfHook);
		VirtualProtect(hookPosition, lengthOfHook, oldProtection, nullptr);
	}
}

void Hook::ToggleTrampSBL()
{
	// toggle hook
	bActive = !bActive;

	if (bActive)
	{
		// making it so that we can execute/read/write bytes at hook location
		DWORD oldProtection = { 0 };
		VirtualProtect(hookPosition, lengthOfHook, PAGE_EXECUTE_READWRITE, &oldProtection);

		// is the trampoline already built? If so, skip making the trampoline
		if (bTrampBuilt)goto TrampAlreadyBuilt;

		// in our trampoline, jump to our space in the DLL for the specified function
		trampToFuncCall = (DWORD)desiredFunction - (DWORD)(trampoline + lengthOfHook + sizeof(preserveStack)) - 5;

		// stolen bytes placed, stack preserved, call our desired function
		memcpy(trampoline, preserveStack, sizeof(preserveStack));
		memset(trampoline + sizeof(preserveStack), 0xE8, 1);
		*(DWORD*)(trampoline + sizeof(preserveStack) + 1) = trampToFuncCall;

		// release stack, jump back to after hook
		memcpy(trampoline + sizeof(preserveStack) + 5, releaseStack, sizeof(releaseStack));
		memcpy(trampoline + sizeof(preserveStack) + 5 + sizeof(releaseStack), (void*)hookPosition, lengthOfHook);
		memset(trampoline + sizeof(preserveStack) + 5 + sizeof(releaseStack) + lengthOfHook, 0xE9, 1);
		*(DWORD*)(trampoline + lengthOfHook + sizeof(preserveStack) + 5 + sizeof(releaseStack) + 1) = returnJump;

		// trampoline is built
		bTrampBuilt = true;

	TrampAlreadyBuilt:
		// set hook inside game's memory
		memset(hookPosition, 0x90, lengthOfHook);
		*(BYTE*)hookPosition = 0xE9;
		*(DWORD*)(hookPosition + 1) = hookToTrampJump;

		// restore protections
		VirtualProtect(hookPosition, lengthOfHook, oldProtection, nullptr);
	}

	if (!bActive)
	{
		DWORD oldProtection = { 0 };
		VirtualProtect(hookPosition, lengthOfHook, PAGE_EXECUTE_READWRITE, &oldProtection);
		memcpy(hookPosition, stolenBytes, lengthOfHook);
		VirtualProtect(hookPosition, lengthOfHook, oldProtection, nullptr);
	}
}

void Hook::ToggleDetour()
{
	// toggle hook
	bActive = !bActive;

	if (bActive)
	{
		// making it so that we can read/write bytes at hook location
		DWORD oldProtection = { 0 };
		VirtualProtect(hookPosition, lengthOfHook, PAGE_EXECUTE_READWRITE, &oldProtection);

		// set detour hook inside game's memory. The detour jumps back for us.
		memset(hookPosition, 0x90, lengthOfHook);
		*(BYTE*)hookPosition = 0xE9;
		*(DWORD*)(hookPosition + 1) = hookToDetourJump;

		// restore protections
		VirtualProtect(hookPosition, lengthOfHook, oldProtection, nullptr);
	}

	if (!bActive)
	{
		DWORD oldProtection = { 0 };
		VirtualProtect(hookPosition, lengthOfHook, PAGE_EXECUTE_READWRITE, &oldProtection);
		memcpy(hookPosition, stolenBytes, lengthOfHook);
		VirtualProtect(hookPosition, lengthOfHook, oldProtection, nullptr);
	}
}



Nop::Nop(BYTE* nopPosition, int length)
{
	this->nopPosition = nopPosition;
	this->length = length;
	memcpy(stolenBytes, nopPosition, length);
}

Nop::Nop()
{
	// do nothing
}

void Nop::ToggleNop()
{
	// toggle NOP
	bActive = !bActive;

	if (bActive)
	{
		DWORD oldProtection = { 0 };
		VirtualProtect(nopPosition, length, PAGE_EXECUTE_READWRITE, &oldProtection);
		memset(nopPosition, 0x90, length);
		VirtualProtect(nopPosition, length, oldProtection, &oldProtection);
	}

	if (!bActive)
	{
		DWORD oldProtection = { 0 };
		VirtualProtect(nopPosition, length, PAGE_EXECUTE_READWRITE, &oldProtection);
		memcpy(nopPosition, stolenBytes, length);
		VirtualProtect(nopPosition, length, oldProtection, &oldProtection);
	}
}

