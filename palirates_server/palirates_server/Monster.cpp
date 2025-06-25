#include "stdafx.h"
#include "Monster.h"


Monster::Monster(int id) : monster_id(id)
{
    trackPositions.resize(4, 1.0f);
    trackWeights.resize(4, 1.0f);
    trackWeights[0] = 1.0f;
}

Monster::Monster()
{
    type = Monster_Type::ETC;
    trackPositions.resize(4, 1.0f);
    trackWeights.resize(4, 1.0f);
    trackWeights[0] = 1.0f;
}


Fishman::Fishman(int id) : Monster(id)
{
    Monster_Type::Fishman;
    trackPositions.resize(8, 1.0f);
    trackWeights.resize(8, 1.0f);
    trackWeights[0] = 1.0f;
}

Anubis::Anubis(int id) : Monster(id)
{
    Monster_Type::Anubis;
    trackPositions.resize(9, 1.0f);
    trackWeights.resize(9, 1.0f);
    trackWeights[0] = 1.0f;
}

Dragon::Dragon(int id) : Monster(id)
{
    Monster_Type::Dragon;
    trackPositions.resize(12, 1.0f);
    trackWeights.resize(12, 1.0f);
    trackWeights[0] = 1.0f;
}