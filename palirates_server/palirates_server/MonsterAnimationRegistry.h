#pragma once
#include "StateEnum.h"
#include "Monster.h"
#include <unordered_map>

class MonsterAnimationRegistry {
public:
    static int GetAnimationTrack(Monster_Type type, State state);
};
