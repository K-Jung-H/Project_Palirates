#pragma once
#include "GameObject.h"
#include <string>
#include <vector>
#include <sstream>

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

    //std::string Serialize()
    //{
    //    std::ostringstream oss;
    //    oss << "MONSTER_UPDATE," << monster_id << "," << x << "," << y << "," << z << ","
    //        << lookX << "," << lookY << "," << lookZ << "," << hp << "," << state << "," << (int)type;

    //    int trackCount = (int)trackPositions.size();
    //    oss << "," << trackCount;
    //    for (int i = 0; i < trackCount; ++i)
    //        oss << "," << trackPositions[i] << "," << trackWeights[i];

    //    return oss.str();
    //}
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
