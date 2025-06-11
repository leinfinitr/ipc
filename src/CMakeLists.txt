file(GLOB_RECURSE MSGQ_SRCS
    ${CMAKE_CURRENT_LIST_DIR}/msgq/*.c
    ${CMAKE_CURRENT_LIST_DIR}/msgq/*.cpp
)
add_library(msgq STATIC ${MSGQ_SRCS})

target_include_directories(msgq PUBLIC
    ${LIBIPC_INCLUDE_DIR}
    ${LIBIPC_INCLUDE_DIR}/msgq
)
