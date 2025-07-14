#include "stdafx.h"
#include "MonsterAnimationRegistry.h"

int MonsterAnimationRegistry::GetAnimationTrack(Monster_Type type, State state) {
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
        case State::Get_Hit:     return TRACK_DRAGON_GOT_HIT1;
        case State::Run:         return TRACK_DRAGON_RUN;
        case State::Knock_Down:  return TRACK_DRAGON_DEAD;
        default: return TRACK_DRAGON_IDLE;
        }
    case Monster_Type::ETC:
        switch (state) {
        case State::Idle:        return TRACK_IDLE;
        case State::Attack1:     return TRACK_ATTACK1;
        case State::Attack2:     return TRACK_FISHMAN_ATTACK2;
        case State::Get_Hit:     return TRACK_FISHMAN_GET_HIT;
        case State::Run:         return TRACK_FISHMAN_WALK;
        case State::Knock_Down:  return TRACK_FISHMAN_DEAD;
        default: return TRACK_FISHMAN_IDLE;
        }
    default:
        return 0;
    }
}
