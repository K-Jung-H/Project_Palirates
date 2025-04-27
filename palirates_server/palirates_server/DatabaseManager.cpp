/*#include "DatabaseManager.h"

DatabaseManager::DatabaseManager()
{
    conn = mysql_init(nullptr);
}

DatabaseManager::~DatabaseManager()
{
    Close();
}

bool DatabaseManager::Connect()
{
    if (!mysql_real_connect(conn, "localhost", "root", "password", "game_server", 3306, NULL, 0))
    {
        std::cerr << "[DB] 연결 실패: " << mysql_error(conn) << std::endl;
        return false;
    }
    std::cout << "[DB] 연결 성공!" << std::endl;
    return true;
}

void DatabaseManager::Close()
{
    if (conn)
    {
        mysql_close(conn);
        conn = nullptr;
    }
}

bool DatabaseManager::SavePlayerPosition(int playerId, float x, float y, float z)
{
    std::string query = "INSERT INTO players (player_id, x, y, z) VALUES (" +
        std::to_string(playerId) + "," +
        std::to_string(x) + "," +
        std::to_string(y) + "," +
        std::to_string(z) +
        ") ON DUPLICATE KEY UPDATE x=VALUES(x), y=VALUES(y), z=VALUES(z)";

    return mysql_query(conn, query.c_str()) == 0;
}
*/