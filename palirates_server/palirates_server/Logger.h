#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <mutex>

class Logger
{
private:
    std::ofstream logFile;
    std::mutex logMutex;
    std::string logFilePath = "logs/server_log.txt";

public:
    Logger();
    ~Logger();

    void Log(const std::string& message);
};
