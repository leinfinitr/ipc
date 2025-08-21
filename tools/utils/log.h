#pragma once

#include <chrono>
#include <cstdint>
#include <cstring>
#include <ctime>
#include <iomanip>
#include <sstream>

#include "common.h"

#define XLOG_FD stderr

#define FLUSH_XLOG()     \
    do {                 \
        fflush(XLOG_FD); \
    } while (0);

#ifdef DEBUG_MODE
#define FLUSH_XLOG_IF_DEBG() FLUSH_XLOG()
#else
#define FLUSH_XLOG_IF_DEBG()
#endif

#define LOG_LEVEL_ERRO 0
#define LOG_LEVEL_WARN 1
#define LOG_LEVEL_INFO 2
#define LOG_LEVEL_DEBG 3

inline int GetLogLevelFromEnv()
{
    char* level = nullptr;

#ifdef _MSC_VER
    size_t len = 0;
    _dupenv_s(&level, &len, "XLOG_LEVEL");
#else
    level = std::getenv("XLOG_LEVEL");
#endif

    if (level == nullptr)
        return LOG_LEVEL_INFO;
    if (strcmp(level, "ERRO") == 0)
        return LOG_LEVEL_ERRO;
    if (strcmp(level, "WARN") == 0)
        return LOG_LEVEL_WARN;
    if (strcmp(level, "INFO") == 0)
        return LOG_LEVEL_INFO;
    if (strcmp(level, "DEBG") == 0)
        return LOG_LEVEL_DEBG;
    // default log level is INFO, log greater than INFO will be ignored
    return LOG_LEVEL_INFO;
}

inline int GetLogLevel()
{
    static const int level = GetLogLevelFromEnv();
    return level;
}

inline void GetLocalTime(const time_t tt, std::tm& lt)
{
#ifdef _WIN32
    localtime_s(&lt, &tt);
#else
    localtime_r(&tt, &lt);
#endif
}

#define XLOG_HELPER(level, level_str, format, ...)                                                                           \
    do {                                                                                                                     \
        if (level > GetLogLevel())                                                                                           \
            break;                                                                                                           \
        const auto now = std::chrono::system_clock::now();                                                                   \
        const auto now_tt = std::chrono::system_clock::to_time_t(now);                                                       \
        std::tm now_lt {};                                                                                                   \
        GetLocalTime(now_tt, now_lt);                                                                                        \
        const auto now_us = std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()).count() % 1000000; \
        fprintf(XLOG_FD, "[IPC %s @ T" FMT_TID " @ %02d:%02d:%02d.%06" PRId64 "] " format "\n",                              \
            level_str, GetThreadId(), now_lt.tm_hour, now_lt.tm_min, now_lt.tm_sec, now_us __VA_OPT__(, ) __VA_ARGS__);      \
        FLUSH_XLOG_IF_DEBG();                                                                                                \
    } while (0);

// first unfold the arguments, then unfold XLOG
#define XLOG(level, level_str, format, ...) \
    UNFOLD(XLOG_HELPER UNFOLD((level, level_str, format __VA_OPT__(, ) __VA_ARGS__)))

#define XLOG_WITH_CODE(level, level_str, format, ...) \
    UNFOLD(XLOG_HELPER UNFOLD((level, level_str, format " @ %s:%d" __VA_OPT__(, ) __VA_ARGS__, __FILE__, __LINE__)))

#ifdef DEBUG_MODE
#define XDEBG(format, ...) XLOG_WITH_CODE(LOG_LEVEL_DEBG, "DEBG", format __VA_OPT__(, ) __VA_ARGS__)
#define XINFO(format, ...) XLOG_WITH_CODE(LOG_LEVEL_INFO, "INFO", format __VA_OPT__(, ) __VA_ARGS__)
#else
#define XDEBG(format, ...)
#define XINFO(format, ...) XLOG(LOG_LEVEL_INFO, "INFO", format __VA_OPT__(, ) __VA_ARGS__)
#endif

#define XWARN(format, ...) XLOG_WITH_CODE(LOG_LEVEL_WARN, "WARN", format __VA_OPT__(, ) __VA_ARGS__)
#define XERRO(format, ...)                                                        \
    do {                                                                          \
        XLOG_WITH_CODE(LOG_LEVEL_ERRO, "ERRO", format __VA_OPT__(, ) __VA_ARGS__) \
        FLUSH_XLOG();                                                             \
    } while (0);

#define XERRO_UNSUPPORTED() XERRO("%s is not supported on %s", __func__, OS_STR);
