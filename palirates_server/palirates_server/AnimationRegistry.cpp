#include "stdafx.h"
#include "AnimationRegistry.h"
#include "Monster.h"

int AnimationRegistry::GetMonsterAnimationTrack(Monster_Type type, State state) {
    switch (type) {
    case Monster_Type::Fishman:
        switch (state) {
        case State::Idle:        return TRACK_FISHMAN_IDLE;
        case State::Attack1:     return TRACK_FISHMAN_ATTACK1;
        case State::Attack2:     return TRACK_FISHMAN_ATTACK2;
        case State::Get_Hit:     return TRACK_FISHMAN_GET_HIT;
        case State::Run:         return TRACK_FISHMAN_WALK;
        case State::Knock_Down:  return TRACK_FISHMAN_DEAD;
        default: return TRACK_FISHMAN_IDLE;
        }
    case Monster_Type::Anubis:
        switch (state) {
        case State::Idle:        return TRACK_ANUBIS_IDLE;
        case State::Attack1:     return TRACK_ANUBIS_ATTACK1;
        case State::Attack2:     return TRACK_ANUBIS_ATTACK2;
        case State::Get_Hit:     return TRACK_ANUBIS_GET_HIT;
        case State::Run:         return TRACK_ANUBIS_WALK;
        case State::Knock_Down:  return TRACK_ANUBIS_DEAD;
        default: return TRACK_ANUBIS_IDLE;
        }
    case Monster_Type::Dragon:
        switch (state) {
        case State::Idle:        return TRACK_DRAGON_IDLE;
        case State::Attack1:     return TRACK_DRAGON_ATTACK1;
        case State::Attack2:     return TRACK_DRAGON_BREATHE;
        case State::Attack3:     return TRACK_DRAGON_FLY_BREATHE;
        case State::Jump:        return TRACK_DRAGON_FLY_IDLE;
        case State::Get_Hit:     return TRACK_DRAGON_GOT_HIT1;
        case State::Run:         return TRACK_DRAGON_RUN;
        case State::Knock_Down:  return TRACK_DRAGON_DEAD;
        default: return TRACK_DRAGON_IDLE;
        }
    case Monster_Type::Gargoyle:
        switch (state) {
        case State::Idle:        return TRACK_GARGOYLE_IDLE;
        case State::Attack1:     return TRACK_GARGOYLE_ATTACK1;
        case State::Attack2:     return TRACK_GARGOYLE_SKILL_1;
        case State::Get_Hit:     return TRACK_GARGOYLE_GET_HIT;
        case State::Run:         return TRACK_GARGOYLE_WALK;
        case State::Knock_Down:  return TRACK_GARGOYLE_DEAD;
        default: return TRACK_GARGOYLE_IDLE;
        }
    case Monster_Type::ETC:
        switch (state) {
        case State::Idle:        return TRACK_IDLE;
        case State::Attack1:     return TRACK_ATTACK1;
        case State::Attack2:     return TRACK_ATTACK2;
        case State::Get_Hit:     return TRACK_GET_HIT_F2;
        case State::Run:         return TRACK_RUN_FORWARD;
        case State::Knock_Down:  return TRACK_KNOCK_DOWN;
        default: return TRACK_FISHMAN_IDLE;
        }
    default:
        return 0;
    }
}

int AnimationRegistry::GetPlayerAnimationTrack(State state) {

    switch (state) {
    case State::Idle:        return TRACK_IDLE;
    case State::Run:        return TRACK_RUN_FORWARD;
    case State::Attack1:     return TRACK_ATTACK1;
    case State::Attack2:     return TRACK_ATTACK2;
    case State::Attack3:     return TRACK_ATTACK3;
    case State::Get_Hit_F2:     return TRACK_GET_HIT_F2;
    case State::Dive:         return TRACK_DIVEROLL_FORWARD;
    case State::Knock_Down:  return TRACK_KNOCK_DOWN;
    case State::Get_Up:  return TRACK_GET_UP;
    default: return TRACK_IDLE;
    }
    return 0;
}
