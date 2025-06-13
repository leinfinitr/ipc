#pragma once

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
