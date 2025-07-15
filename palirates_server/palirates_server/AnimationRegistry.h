#pragma once
#include "StateEnum.h"
#include "Monster.h"
#include <unordered_map>

class AnimationRegistry {
public:
    static int GetMonsterAnimationTrack(Monster_Type type, State state);
    static int GetPlayerAnimationTrack(State state);
};
