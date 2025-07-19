#pragma once
#include "StateEnum.h"
#include <unordered_map>

enum class Monster_Type : int;

class AnimationRegistry {
public:
    static int GetMonsterAnimationTrack(Monster_Type type, State state);
    static int GetPlayerAnimationTrack(State state);
};
