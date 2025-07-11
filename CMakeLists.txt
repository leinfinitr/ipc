cmake_minimum_required(VERSION 3.12)

project(ipc VERSION 1.0.0 LANGUAGES C CXX)

if(CMAKE_BUILD_TYPE STREQUAL "DEBUG")
    add_compile_definitions(DEBUG)
    if(MSVC)
        add_compile_options(/Zi /Od)
    else()
        add_compile_options(-g -O0)
    endif()
endif()

set(CMAKE_CXX_STANDARD 17)
if(MSVC)
    set(CMAKE_C_FLAGS_RELEASE   "$ENV{CFLAGS}   /O2 /DRELEASE_MODE")
    set(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} /O2 /DRELEASE_MODE")
    set(CMAKE_C_FLAGS_DEBUG     "$ENV{CFLAGS}   /Od /Zi /DDEBUG_MODE")
    set(CMAKE_CXX_FLAGS_DEBUG   "$ENV{CXXFLAGS} /Od /Zi /DDEBUG_MODE")
else()
    set(CMAKE_C_FLAGS_RELEASE   "$ENV{CFLAGS}   -O3 -DRELEASE_MODE")
    set(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -DRELEASE_MODE")
    set(CMAKE_C_FLAGS_DEBUG     "$ENV{CFLAGS}   -O0 -g2 -ggdb -DDEBUG_MODE")
    set(CMAKE_CXX_FLAGS_DEBUG   "$ENV{CXXFLAGS} -O0 -g2 -ggdb -DDEBUG_MODE")
endif()

set(LIBIPC_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(LIBIPC_PROJECT_DIR ${CMAKE_CURRENT_SOURCE_DIR})

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
