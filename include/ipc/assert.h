#pragma once

#ifndef _WIN32
#define ASSERT_EXIT(expr, format, ...)                 \
    do {                                               \
        if (static_cast<bool>(expr)) {                 \
            fprintf(stderr, "Error: " format ": %s\n", \
                ##__VA_ARGS__, strerror(errno));       \
            exit(EXIT_FAILURE);                        \
        }                                              \
    } while (0)

#define ASSERT_RETURN(expr, ret, format, ...)          \
    do {                                               \
        if (static_cast<bool>(expr)) {                 \
            fprintf(stderr, "Error: " format ": %s\n", \
                ##__VA_ARGS__, strerror(errno));       \
            return ret;                                \
        }                                              \
    } while (0)
#else
#include <string>
inline std::string safe_strerror(int errnum)
{
    char buffer[256];
    strerror_s(buffer, sizeof(buffer), errnum);
    return std::string(buffer);
}

#define ASSERT_EXIT(expr, format, ...)                        \
    do {                                                      \
        if (static_cast<bool>(expr)) {                        \
            fprintf(stderr, "Error: " format ": %s\n",        \
                ##__VA_ARGS__, safe_strerror(errno).c_str()); \
            exit(EXIT_FAILURE);                               \
        }                                                     \
    } while (0)

#define ASSERT_RETURN(expr, ret, format, ...)                 \
    do {                                                      \
        if (static_cast<bool>(expr)) {                        \
            fprintf(stderr, "Error: " format ": %s\n",        \
                ##__VA_ARGS__, safe_strerror(errno).c_str()); \
            return ret;                                       \
        }                                                     \
    } while (0)
#endif