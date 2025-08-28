#pragma once

#include "GameObject.h"
#include "Player.h"
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
    Gargoyle,
    ETC
};

class Monster : public Skinned_GameObject {
protected:
    Monster_Type type = Monster_Type::ETC;
    int monster_id = -1;
    int hp = 100;

    std::unique_ptr<MonsterStateMachine> m_StateMachine;
    const std::array<std::shared_ptr<Player>, MaxPlayer>* pPlayerList = nullptr;

public:
    
    float stateChangeInterval = 2.0f;
    XMFLOAT3 m_targetPos = { 0, 0, 0 };
    bool m_shouldRotate = false;
    float detectionRange = 0.0f;
    float attackRange = 0.0f;
    int attackPhase = -1;
    XMFLOAT3 m_faketargetPos = { 0, 0, 0 };

public:
    Monster(int id);
    Monster() = default;
    virtual ~Monster() = default;

    virtual void update(float deltaTime) override;
    void update_collision(float deltaTime, std::vector<BoundingOrientedBox> obblist);

    MonsterStateMachine* GetStateMachine() { return m_StateMachine.get(); }

    virtual int PlayAnimation(State state);
    virtual ServerSyncData MakeSyncData();

    void SetPlayerListPtr(const std::array<std::shared_ptr<Player>, MaxPlayer>* ptr) { pPlayerList = ptr; }
    const std::array<std::shared_ptr<Player>, MaxPlayer>* GetPlayerListPtr() const { return pPlayerList; }
    virtual std::optional<XMFLOAT3> FindNearestPlayerInRange(float range);
    virtual void SetTarget(const XMFLOAT3& targetPos);
    virtual void StartAttackCooldown();
    virtual bool IsAttackCooldownOver() const;

    Monster_Type GetType() const { return type; }
    int GetID() const { return monster_id; }
    void SetID(int id) { monster_id = id; }

    void InitAnimationController(const std::string& filepath, int animCount, int rootIdx, const std::unordered_set<int>& onceTracks) override;
    void InitStateMachine();

    void HitDamage(float damage);
    float GetHP() { return hp; }
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

class Gargoyle : public Monster {
public:
    Gargoyle(int id);
    // void update() override;
};

class TestPlayer : public Monster {
public:
    TestPlayer(int id);
    // void update() override;
};
