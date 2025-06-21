if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    # msgq library
    file(GLOB_RECURSE MSGQ_SRCS
        ${CMAKE_CURRENT_LIST_DIR}/msgq/*.c
        ${CMAKE_CURRENT_LIST_DIR}/msgq/*.cpp
    )
    add_library(msgq STATIC ${MSGQ_SRCS})

    if(BUILD_TYPE STREQUAL "Debug")
        target_compile_options(msgq PRIVATE -g)
        set_target_properties(msgq PROPERTIES
            COMPILE_OPTIONS "-g"
            INTERPROCEDURAL_OPTIMIZATION FALSE
        )
    endif()

    target_include_directories(msgq PUBLIC
        ${LIBIPC_INCLUDE_DIR}
        ${LIBIPC_INCLUDE_DIR}/msgq
    )
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    # pipe library
    file(GLOB_RECURSE PIPE_SRCS
        ${CMAKE_CURRENT_LIST_DIR}/pipe/*.c
        ${CMAKE_CURRENT_LIST_DIR}/pipe/*.cpp
    )
    add_library(pipe STATIC ${PIPE_SRCS})

    if(BUILD_TYPE STREQUAL "Debug")
        target_compile_options(pipe PRIVATE -g)
        set_target_properties(pipe PROPERTIES
            COMPILE_OPTIONS "-g"
            INTERPROCEDURAL_OPTIMIZATION FALSE
        )
    endif()

    target_include_directories(pipe PUBLIC
        ${LIBIPC_INCLUDE_DIR}
        ${LIBIPC_INCLUDE_DIR}/pipe
    )
endif()

# ipc library
file(GLOB_RECURSE IPC_SRCS
    ${CMAKE_CURRENT_LIST_DIR}/ipc.cpp
)
add_library(ipc STATIC ${IPC_SRCS})

if(BUILD_TYPE STREQUAL "Debug")
    target_compile_options(ipc PRIVATE -g)
    set_target_properties(ipc PROPERTIES
        COMPILE_OPTIONS "-g"
        INTERPROCEDURAL_OPTIMIZATION FALSE
    )
endif()

set_property(TARGET ipc PROPERTY POSITION_INDEPENDENT_CODE ON)

target_include_directories(ipc PUBLIC
    ${LIBIPC_INCLUDE_DIR}
)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_link_libraries(ipc PRIVATE msgq)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    target_link_libraries(ipc PRIVATE pipe)
endif()

install(TARGETS ipc
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)