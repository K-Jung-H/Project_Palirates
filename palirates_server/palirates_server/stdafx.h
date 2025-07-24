#pragma once

#define WIN32_LEAN_AND_MEAN   
#define NOMINMAX              

#include <windows.h> 
#include <wrl.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

#include <cmath>
#include <Mmsystem.h>
#include <vector>
#include <string>
#include <unordered_map>
#include <iostream>
#include <mutex>
#include <random>
#include <tchar.h>
#include <unordered_set>
#include <unordered_map>
#include <optional>
//#include <DirectXMath.h>
#include "DX_Setter.h"

using namespace std;

#define ANIMATION_TYPE_ONCE				0
#define ANIMATION_TYPE_LOOP				1
#define ANIMATION_TYPE_PINGPONG			2

#define ANIMATION_CALLBACK_EPSILON		0.00165f

#define MaxPlayer 6

enum Scene_Type
{
	Lobby,
	Board,
    Stage_1,
    Stage_2,
    Stage_3,
    Stage_4,
    Stage_5,
    Stage_6,
    Stage_7,
	Test,
	etc,
	None,
};

enum KeyIndex
{
    KEY_INDEX_W = 0,
    KEY_INDEX_S = 1,
    KEY_INDEX_A = 2,
    KEY_INDEX_D = 3,
    KEY_INDEX_Q = 4,
    KEY_INDEX_E = 5,
    KEY_INDEX_SHIFT = 6,
    KEY_INDEX_ENTER = 7
};

// 실제 플래그 enum
enum InputFlags : uint32_t
{
    INPUT_NONE = 0,
    INPUT_W = 1 << KEY_INDEX_W,
    INPUT_S = 1 << KEY_INDEX_S,
    INPUT_A = 1 << KEY_INDEX_A,
    INPUT_D = 1 << KEY_INDEX_D,
    INPUT_Q = 1 << KEY_INDEX_Q,
    INPUT_E = 1 << KEY_INDEX_E,
    INPUT_SHIFT = 1 << KEY_INDEX_SHIFT,
    INPUT_ENTER = 1 << KEY_INDEX_ENTER
};

struct MonsterDamageInfo
{
    int monsterID;
    float damage;
};

extern BYTE ReadStringFromFile(FILE* pInFile, char* pstrToken);
extern int ReadIntegerFromFile(FILE* pInFile);
extern float ReadFloatFromFile(FILE* pInFile);