/*#pragma once
#include <mysql/mysql.h>
#include <iostream>
#include <string>

class DatabaseManager
{
private:
    MYSQL* conn;

public:
    DatabaseManager();
    ~DatabaseManager();

    bool Connect();
    void Close();

    bool SavePlayerPosition(int playerId, float x, float y, float z);
    bool LoadPlayerPosition(int playerId, float& x, float& y, float& z);
};
*/