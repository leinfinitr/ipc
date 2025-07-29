#pragma once

#define LOG_INFO(format, ...) \
    do { \
        fprintf(stderr, "[IPC Info: %s(%d)] " format "\n", __FILE__, __LINE__, ##__VA_ARGS__); \
    } while (0)

#ifdef DEBUG
#define LOG_DEBUG(format, ...) \
    do { \
        fprintf(stderr, "[IPC Debug: %s(%d)] " format "\n", __FILE__, __LINE__, ##__VA_ARGS__); \
    } while (0)
#else
#define LOG_DEBUG(format, ...) \
    do { \
        /* No debug output in release mode */ \
    } while (0)
#endif // DEBUG