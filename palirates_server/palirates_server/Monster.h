#pragma once
#include "GameObject.h"
#include "Object_StateMachine.h"
#include <string>
#include <vector>
#include <sstream>

class MonsterStateMachine;

enum class Monster_Type : int
{
    Fishman,
    Anubis,
    Dragon,
    ETC
};

enum class Monster_State : int
{
    Idle ,
    Walk,
    Attack,
    ETC
};


class Monster : public Skinned_GameObject
{
protected:
    Monster_Type type;
    Monster_State monster_state; 
    int monster_id;
    std::unique_ptr<MonsterStateMachine> m_StateMachine;

public:
    int hp;

    float stateElapsedTime = 0.0f;
    float stateChangeInterval = 2.0f;

public:
    Monster(int id);
    Monster();


    virtual void update() {}

    Monster_State GetState() { return monster_state; }
    void SetState(Monster_State new_state) { monster_state = new_state; }

    virtual MonsterStateMachine* GetStateMachine() { return m_StateMachine.get(); }
    virtual ServerSyncData MakeSyncData();
};



class Fishman : public Monster
{
public:
    Fishman(int id);

    virtual void animate(float Elapsedtime) {}
    virtual void update() {}
};

class Anubis : public Monster
{
public:
    Anubis(int id);

    virtual void animate(float Elapsedtime) {}
    virtual void update() {}
};

class Dragon : public Monster
{
public:
    Dragon(int id);

    virtual void animate(float Elapsedtime) {}
    virtual void update() {}
};
