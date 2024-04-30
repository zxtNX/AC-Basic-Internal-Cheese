#pragma once
#pragma comment(lib, "OpenGL32.lib")

#include <Windows.h>
#include <cstdint>
#include <vector>
#include <gl/GL.h>


class Entity
{
public:
	char pad_0000[4]; //0x0000
	float xHeadCoord; //0x0004
	float yHeadCoord; //0x0008
	float zHeadCoord; //0x000C
	char pad_0010[36]; //0x0010
	float xOriginCoord; //0x0034
	float yOriginCoord; //0x0038
	float zOriginCoord; //0x003C
	float yaw; //0x0040
	float pitch; //0x0044
	char pad_0048[58]; //0x0048
	bool bDead; //0x0082
	char pad_0083[117]; //0x0083
	int32_t health; //0x00F8
	char pad_00FC[84]; //0x00FC
	int32_t arAmmo; //0x0150
	char pad_0154[144]; //0x0154
	int32_t playerNumber; //0x01E4
	char pad_01E8[60]; //0x01E8
	int8_t bAttack; //0x0224
	char name[16]; //0x0225
	char pad_0235[247]; //0x0235
	int32_t team; //0x032C
	char pad_0330[68]; //0x0330
	class Weapons* pWeapons; //0x0374
};

class Weapons
{
public:
	char pad_0000[20]; //0x0000
	class WeaponInfos* pWeaponInfos; //0x0014
	char pad_0018[44]; //0x0018
};

class WeaponInfos
{
public:
	int32_t weaponMagAmmo; //0x0000
	char pad_0004[64]; //0x0004
};

struct EntityInfo
{
	Entity* entity;
	float distanceFromPlayer;
	bool isTargetable;  // Si l'entité peut être ciblée
	bool isInFOV;  // Si l'entité est dans le FOV du localPlayer
	bool isVisible;  // Si l'entité est visible ou non

	float screenX = NULL;
	float screenY = NULL;
};

struct Vector4
{
	float x = NULL;
	float y = NULL;
	float z = NULL;
	float w = NULL;
};

struct Vector3
{
	float x = NULL;
	float y = NULL;
	float z = NULL;
};

struct Vector2
{
	float coordOne = NULL;
	float coordTwo = NULL;
};

struct Color
{
	const GLubyte red[3] = { 255, 0, 0 };
	const GLubyte green[3] = { 0, 255, 0 };
	const GLubyte grey[3] = { 127, 127, 127 };
};

extern std::vector<EntityInfo> entityInfos;
extern Color color;