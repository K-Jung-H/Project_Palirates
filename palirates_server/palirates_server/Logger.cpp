#include "Logger.h"
#include <ctime>

Logger::Logger()
{
    logFile.open(logFilePath, std::ios::app);
}

Logger::~Logger()
{
    if (logFile.is_open())
    {
        logFile.close();
    }
}

void Logger::Log(const std::string& message)
{
    std::lock_guard<std::mutex> lock(logMutex);

    std::time_t now = std::time(nullptr);
    struct tm timeInfo;
    localtime_s(&timeInfo, &now);

    char timeStr[20];
    std::strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", &timeInfo);

    std::string logEntry = "[" + std::string(timeStr) + "] " + message;

    std::cout << logEntry << std::endl;

    if (logFile.is_open())
    {
        logFile << logEntry << std::endl;
    }
}
