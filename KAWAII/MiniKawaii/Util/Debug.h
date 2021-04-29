#pragma once

#include <fstream>

enum class LOG_LEVEL
{
    LOG_LEVEL_INFO,
    LOG_LEVEL_WARNING,
    LOG_LEVEL_ERROR
};

class Debug
{
public:
    static void Log(LOG_LEVEL logLevel,
        const char* function,
        const char* file,
        int line,
        const std::string& message);

private:
    static std::string m_Message;
};

#define LOG(message)                                                                      \
    do                                                                                    \
    {                                                                                     \
        Debug::Log(LOG_LEVEL::LOG_LEVEL_INFO, __FUNCTION__, __FILE__, __LINE__, message); \
    } while(false)

#define LOG_WARNING(message)                                                                      \
    do                                                                                    \
    {                                                                                     \
        Debug::Log(LOG_LEVEL::LOG_LEVEL_WARNING, __FUNCTION__, __FILE__, __LINE__, message); \
    } while(false)

#define LOG_ERROR(message)                                                                      \
    do                                                                                    \
    {                                                                                     \
        Debug::Log(LOG_LEVEL::LOG_LEVEL_ERROR, __FUNCTION__, __FILE__, __LINE__, message); \
    } while(false)