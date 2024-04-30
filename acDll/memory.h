#pragma once

struct Hook
{
	// get to our hook
	BYTE* hookPosition{ nullptr };

	// store stolen bytes
	BYTE stolenBytes[20]{ 0 };
	int lengthOfHook = { 0 };

	// is hook active?
	bool bActive = false;


	// tramp hook setup
	BYTE* desiredFunction{ nullptr };
	DWORD hookToTrampJump = { 0 }; // jump at the hook to our trampoline
	DWORD trampToFuncCall = { 0 }; // call our function from the trampoline
	DWORD returnJump = { 0 };      // jump at the end of our trampoline back to the flow of execution
	
	// trampoline placement
	BYTE* trampoline{ nullptr };

	// Don't corrupt the stack
	BYTE preserveStack[2]{ 0x60, 0x9C }; // pushad, pushfd
	BYTE releaseStack[2]{ 0x9D, 0x61 };  // popfd, popad
	bool bTrampBuilt = false;

	// detour hook setup
	DWORD hookToDetourJump;
	DWORD returnJumpDetour;

	// functionalities
	void ToggleTrampSBF(); // to toggle trampoline with stolen byte first (SBF)
	void ToggleTrampSBL(); // to toggle trampoline with stolen byte last (SBL)
	void ToggleDetour();   // to toggle detour

	// constructors
	Hook(BYTE* hookPosition, BYTE* desiredFunction, int lengthOfHook);
	Hook();
};

struct Nop
{
	// where at and for how long?
	BYTE* nopPosition{ nullptr };
	int length = { 0 };
	
	// save original code (bytes) for toggling off
	BYTE stolenBytes[30]{ 0 };

	// used by function to know whether to store original bytes, NOP,
	// or replace nopPosition with stolenBytes
	bool bActive = false;

	// functionality
	void ToggleNop();

	// constructors
	Nop(BYTE* nopPosition, int length);
	Nop();
};