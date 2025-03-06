#pragma once
#include <unordered_map>
#include "InGameCharacter.h"

class InGameCharacterManager
{
private:
    std::unordered_map<int, InGameCharacter> characters;  // 플레이어 ID를 키로 사용

public:
    void addCharacter(int playerId, float x, float y, float z)
    {
        characters[playerId] = InGameCharacter(playerId, x, y, z);
    }

    void updateCharacter(int playerId, float x, float y, float z, int state)
    {
        if (characters.find(playerId) != characters.end())
        {
            characters[playerId].setPosition(x, y, z);
            characters[playerId].state = state;
        }
        else
        {
            // 새 플레이어 추가 (예: 새로운 유저가 접속한 경우)
            addCharacter(playerId, x, y, z);
        }
    }

    void printAllCharacters()
    {
        for (const auto& [id, character] : characters)
        {
            character.printPosition();
        }
    }
};
