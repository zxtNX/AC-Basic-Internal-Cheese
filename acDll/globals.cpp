#include "pch.h"
#include "globals.h"

BYTE* gModuleBaseAssaultCube = (BYTE*)GetModuleHandle(L"ac_client.exe");
BYTE* gModuleBaseOpenGl = (BYTE*)GetModuleHandle(L"opengl32.dll");

int gNumberOfPlayers = *(int*)(gModuleBaseAssaultCube + 0x10F500);
float gPI = 3.1415927f;