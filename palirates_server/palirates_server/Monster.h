#pragma once
#include <string>
#include <vector>
#include <sstream>

enum class Monster_Type : int
{
    ETC,
    Fishman,
    Anubis,
    Dragon
};


class Monster
{
public:
    int id;
    float x, y, z;
    float lookX, lookY, lookZ;
    int hp;
    int state; // 0: idle, 1: 이동, 2: 공격
    Monster_Type type;

    std::vector<float> trackPositions;
    std::vector<float> trackWeights;

    float stateElapsedTime = 0.0f;
    float stateChangeInterval = 2.0f;

    Monster(int id, float x, float y, float z, float lookX, float lookY, float lookZ, int hp, int state, Monster_Type type)
        : id(id), x(x), y(y), z(z), lookX(lookX), lookY(lookY), lookZ(lookZ), hp(hp), state(state), type(type)
    {
        trackPositions.resize(4, 1.0f);
        trackWeights.resize(4, 1.0f);
        trackWeights[0] = 1.0f;
    }

    Monster() : id(-1), x(0), y(0), z(0), lookX(0), lookY(1), lookZ(0), hp(100), state(0), type(Monster_Type::ETC)
    {
        trackPositions.resize(4, 1.0f);
        trackWeights.resize(4, 1.0f);
        trackWeights[0] = 1.0f;
    }
    void update(float newX, float newY, float newZ, float newlookVecX, float newlookVecY, float newlookVecZ, int newState, int newHp)
    {
        x = newX;
        y = newY;
        z = newZ;
        lookX = newlookVecX;
        lookY = newlookVecY;
        lookZ = newlookVecZ;
        state = newState;
        hp = newHp;
    }

    std::string Serialize()
    {
        std::ostringstream oss;
        oss << "MONSTER_UPDATE," << id << "," << x << "," << y << "," << z << ","
            << lookX << "," << lookY << "," << lookZ << "," << hp << "," << state << "," << (int)type;

        int trackCount = (int)trackPositions.size();
        oss << "," << trackCount;
        for (int i = 0; i < trackCount; ++i)
            oss << "," << trackPositions[i] << "," << trackWeights[i];

        return oss.str();
    }
};

class Fishman : public Monster
{
    public:
    Fishman(int id, float x, float y, float z, float lookX, float lookY, float lookZ, int hp)
        : Monster(id, x, y, z, lookX, lookY, lookZ, hp, 0, Monster_Type::Fishman)
    {
        trackPositions.resize(8, 1.0f);
        trackWeights.resize(8, 1.0f);
        trackWeights[0] = 1.0f;
    }
   
};

class Anubis : public Monster
{
    public:
    Anubis(int id, float x, float y, float z, float lookX, float lookY, float lookZ, int hp)
        : Monster(id, x, y, z, lookX, lookY, lookZ, hp, 0, Monster_Type::Anubis)
    {
        trackPositions.resize(9, 1.0f);
        trackWeights.resize(9, 1.0f);
        trackWeights[0] = 1.0f;
	}
};

class Dragon : public Monster
{
    public:
    Dragon(int id, float x, float y, float z, float lookX, float lookY, float lookZ, int hp)
        : Monster(id, x, y, z, lookX, lookY, lookZ, hp, 0, Monster_Type::Dragon)
    {
        trackPositions.resize(12, 1.0f);
        trackWeights.resize(12, 1.0f);
        trackWeights[0] = 1.0f;
    }
};