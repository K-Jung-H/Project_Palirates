#include "stdafx.h"
#include "Monster.h"


Monster::Monster(int id) : monster_id(id)
{

}

Monster::Monster()
{
    type = Monster_Type::ETC;

}


Fishman::Fishman(int id) : Monster(id)
{
    Monster_Type::Fishman;

}

Anubis::Anubis(int id) : Monster(id)
{
    Monster_Type::Anubis;

}

Dragon::Dragon(int id) : Monster(id)
{
    Monster_Type::Dragon;

}