# msgq library
file(GLOB_RECURSE MSGQ_SRCS
    ${CMAKE_CURRENT_LIST_DIR}/msgq/*.c
    ${CMAKE_CURRENT_LIST_DIR}/msgq/*.cpp
)
add_library(msgq STATIC ${MSGQ_SRCS})

target_include_directories(msgq PUBLIC
    ${LIBIPC_INCLUDE_DIR}
    ${LIBIPC_INCLUDE_DIR}/msgq
)

install(TARGETS msgq
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
)

# ipc library
file(GLOB_RECURSE IPC_SRCS
    ${CMAKE_CURRENT_LIST_DIR}/*.c
    ${CMAKE_CURRENT_LIST_DIR}/*.cpp
)
add_library(ipc STATIC ${IPC_SRCS})
set_property(TARGET ipc PROPERTY POSITION_INDEPENDENT_CODE ON)

target_include_directories(ipc PUBLIC
    ${LIBIPC_INCLUDE_DIR}
)

target_link_libraries(ipc PRIVATE
    msgq
)

install(TARGETS ipc
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)