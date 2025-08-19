#pragma once

#include <chrono>
#include <ctime>

#include "common.h"

inline void GetLocalTime(const time_t tt, std::tm& lt)
{
#ifdef _WIN32
    localtime_s(&lt, &tt);
#else
    localtime_r(&tt, &lt);
#endif
}

#define LOG_INFO(format, ...)                                                                  \
    do {                                                                                       \
        fprintf(stderr, "[IPC Info: %s(%d)] " format "\n", __FILE__, __LINE__, ##__VA_ARGS__); \
    } while (0)

#ifdef DEBUG
#define LOG_DEBUG(format, ...)                                                                                               \
    do {                                                                                                                     \
        const auto now = std::chrono::system_clock::now();                                                                   \
        const auto now_tt = std::chrono::system_clock::to_time_t(now);                                                       \
        std::tm now_lt {};                                                                                                   \
        GetLocalTime(now_tt, now_lt);                                                                                        \
        const auto now_us = std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()).count() % 1000000; \
        fprintf(stderr, "[IPC Debug @ T" FMT_TID " @ %02d:%02d:%02d.%06" PRId64 "] " format " @ %s:%d\n",                    \
            GetThreadId(), now_lt.tm_hour, now_lt.tm_min, now_lt.tm_sec, now_us, ##__VA_ARGS__, __FILE__, __LINE__);         \
        fflush(stderr);                                                                                                      \
    } while (0)
#else
#define LOG_DEBUG(format, ...)                \
    do {                                      \
        /* No debug output in release mode */ \
    } while (0)
#endif // DEBUG