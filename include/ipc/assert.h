#pragma once

#ifndef _WIN32
#define ASSERT(expr, format, ...)                                       \
    do {                                                                \
        if (static_cast<bool>(expr)) {                                  \
            fprintf(stderr, "Error: %s(%d): " format ": %s\n",          \
                __FILE__, __LINE__, ##__VA_ARGS__, strerror(errno));    \
        }                                                               \
    } while (0)

#define ASSERT_EXIT(expr, format, ...)                                  \
    do {                                                                \
        if (static_cast<bool>(expr)) {                                  \
            fprintf(stderr, "Error: %s(%d): " format ": %s\n",          \
                __FILE__, __LINE__, ##__VA_ARGS__, strerror(errno));    \
            exit(EXIT_FAILURE);                                         \
        }                                                               \
    } while (0)

#define ASSERT_RETURN(expr, ret, format, ...)                           \
    do {                                                                \
        if (static_cast<bool>(expr)) {                                  \
            fprintf(stderr, "Error: %s(%d): " format ": %s\n",          \
                __FILE__, __LINE__, ##__VA_ARGS__, strerror(errno));    \
            return ret;                                                 \
        }                                                               \
    } while (0)
#else
#include <windows.h>
#include <string>
inline std::string win_strerror(DWORD errnum) {
    LPSTR buffer = nullptr;
    DWORD length = FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM | 
        FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr,
        errnum,
        0,  // Default language (English)
        (LPSTR)&buffer,
        0,
        nullptr
    );

    if (length == 0) {
        return "Unknown error";
    }

    std::string message(buffer, length);
    LocalFree(buffer);  // Free the allocated buffer
    return message;
}

#define ASSERT(expr, format, ...)                                       \
    do {                                                                \
        if (static_cast<bool>(expr)) {                                  \
            fprintf(stderr, "%s(%d): " format " : Error %lu %s",        \
                __FILE__, __LINE__, ##__VA_ARGS__,                      \
            GetLastError(), win_strerror(GetLastError()).c_str());      \
        }                                                               \
    } while (0)

#define ASSERT_EXIT(expr, format, ...)                                  \
    do {                                                                \
        if (static_cast<bool>(expr)) {                                  \
            fprintf(stderr, "%s(%d): " format " : Error %lu %s",        \
                __FILE__, __LINE__, ##__VA_ARGS__,                      \
            GetLastError(), win_strerror(GetLastError()).c_str());      \
            exit(EXIT_FAILURE);                                         \
        }                                                               \
    } while (0)

#define ASSERT_RETURN(expr, ret, format, ...)                           \
    do {                                                                \
        if (static_cast<bool>(expr)) {                                  \
            fprintf(stderr, "%s(%d): " format " : Error %lu %s",        \
                __FILE__, __LINE__, ##__VA_ARGS__,                      \
            GetLastError(), win_strerror(GetLastError()).c_str());      \
            return ret;                                                 \
        }                                                               \
    } while (0)
#endif