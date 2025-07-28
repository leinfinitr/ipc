cmake_minimum_required(VERSION 3.12)
project(ipc VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(LIBIPC_PROJECT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(LIBIPC_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(LIBIPC_TOOLS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/tools)

if(CMAKE_BUILD_TYPE STREQUAL "DEBUG")
    # Add predefined macros to the compiler
    # These macros will be added to the source code
    add_compile_definitions(DEBUG)
endif()

if(MSVC)
    # Overrides the default Debug compilation options of CMake,
    # avoiding conflicts between /MTd(ipc) and /MDd(gtest).
    set(CMAKE_C_FLAGS_DEBUG   "$ENV{CFLAGS}   /Od /Zi")
    set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} /Od /Zi")
endif()

add_subdirectory(src)

if (BUILD_TEST)
    set(GOOGLETEST_VERSION 1.17.0)
    add_subdirectory(3rdparty/gtest EXCLUDE_FROM_ALL)
    add_subdirectory(test)
endif()

install (
    DIRECTORY ${LIBIPC_INCLUDE_DIR}/
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)
