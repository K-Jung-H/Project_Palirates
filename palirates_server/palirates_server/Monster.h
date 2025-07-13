#pragma once

#include "GameObject.h"
#include "Object_StateMachine.h"
#include "StateEnum.h"
#include "AnimationTrackEnum.h"
#include <string>
#include <vector>
#include <sstream>
#include <memory>

class MonsterStateMachine;

constexpr int ENCODE_MONSTER_ID(int type, int index) {
    return (type << 24) | (index & 0xFFFFFF);
}

constexpr int GET_MONSTER_TYPE(int id) {
    return (id >> 24) & 0xFF;
}

constexpr int GET_MONSTER_INDEX(int id) {
    return id & 0xFFFFFF;
}

enum class Monster_Type : int {
    Fishman,
    Anubis,
    Dragon,
    ETC
};

class Monster : public Skinned_GameObject {
protected:
    Monster_Type type = Monster_Type::ETC;
    int monster_id = -1;
    int hp = 100;

    std::unique_ptr<MonsterStateMachine> m_StateMachine;

public:
    float stateElapsedTime = 0.0f;
    float stateChangeInterval = 2.0f;

public:
    Monster(int id);
    Monster() = default;
    virtual ~Monster() = default;

    virtual void update(float deltaTime) override;

    MonsterStateMachine* GetStateMachine() { return m_StateMachine.get(); }

    virtual void PlayAnimation(State state);
    virtual ServerSyncData MakeSyncData();

    virtual GameObject* FindNearestPlayerInRange(float range);
    virtual void SetTarget(GameObject* target);
    virtual void StartAttackCooldown();
    virtual bool IsAttackCooldownOver() const;

    Monster_Type GetType() const { return type; }
    int GetID() const { return monster_id; }
    void SetID(int id) { monster_id = id; }

    void InitAnimationController(const std::string& filepath, int animCount, int rootIdx, const std::unordered_set<int>& onceTracks);
};

class Fishman : public Monster {
public:
    Fishman(int id);
    //void update() override;
};

class Anubis : public Monster {
public:
    Anubis(int id);
    //void update() override;
};

class Dragon : public Monster {
public:
    Dragon(int id);
   // void update() override;
};
