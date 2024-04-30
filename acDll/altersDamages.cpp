#include "pch.h"
#include "altersDamages.h"
#include "setupHooksAndNops.h"

#define teamOffset 0x238
#define playerNumber 0xF0
#define healthOffset 0x04
#define armorOffset 0x08
#define maxHealth 0x64
#define maxArmor 0x64
#define eliminate 0x00

int myTeam = 5; // initialized as 5 for error check. Only team numbers are 0/1


// player has to get hit first before one hit elimination and god mode applies
// __declspec(naked) doesn't allow easy creation of variables. so i use a global i created at the top of the project for my team value
void __declspec(naked)AltersDamagesTeamDetour()
{
	__asm
	{	mov eax, esi       //stolen bytes

		//do I already have a team value? if so, continue hack
		cmp [myTeam], 0x05
		jne ContinueHack

		//get team info, the local player is always player# 0
		cmp [ebx + playerNumber], 0x00
		je GetTeamInfo

		//if myTeam has not been harvested yet, nobody takes damage yet
		cmp [myTeam], 0x05
		je NoTeamValueAttained

		//if myTeam does have a value, process hack
		//are you on my team? if so get 100 health/armor. if not, get eliminated
		ContinueHack:
		push edx
		mov edx, [myTeam]
		cmp [ebx + teamOffset], edx //edx is now my team value
		pop edx
		jne code
		mov [ebx + healthOffset], maxHealth
		mov [ebx + armorOffset], maxArmor
		jmp altersDamagesDetour.returnJumpDetour

		// one shot enemies
		code:
		mov [ebx + healthOffset], eliminate
		jmp altersDamagesDetour.returnJumpDetour

		GetTeamInfo:
		push edx
		mov edx, [ebx + teamOffset]
		mov [myTeam], edx
		pop edx
		jmp altersDamagesDetour.returnJumpDetour

		NoTeamValueAttained:
		jmp altersDamagesDetour.returnJumpDetour
	}
}