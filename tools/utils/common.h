#pragma once

#include <fstream>
#include <cstdint>
#include <cinttypes>
#include <type_traits>
#ifdef _WIN32
#include <windows.h>
#include <process.h>
#else
#include <unistd.h>
#include <sys/syscall.h>
#endif

#define FMT_32D "%" PRId32
#define FMT_32U "%" PRIu32
#define FMT_32X "%" PRIx32
#define FMT_64D "%" PRId64
#define FMT_64U "%" PRIu64
#define FMT_64X "%" PRIx64

#ifdef _WIN32
    typedef int64_t       TID;
    typedef pid_t         PID;
    #define FMT_TID       FMT_64D
    #define FMT_PID       FMT_64D
#else
    typedef int32_t       TID;
    typedef pid_t         PID;
    #define FMT_TID       FMT_32D
    #define FMT_PID       FMT_32D
#endif

inline TID GetThreadId()
{
    #ifdef _WIN32
    static const thread_local TID tid = GetCurrentThreadId();
    #else
    static const thread_local TID tid = syscall(SYS_gettid);
    #endif
    return tid;
}