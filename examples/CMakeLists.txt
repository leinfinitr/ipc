cmake_minimum_required(VERSION 3.12)

project(example)

add_executable(sender ${CMAKE_CURRENT_SOURCE_DIR}/sender.cpp)
add_executable(receiver ${CMAKE_CURRENT_SOURCE_DIR}/receiver.cpp)

set(IPC_LIB_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../output/lib/libipc.a")
set(IPC_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../output/include")

# Add the IPC library
add_library(ipc STATIC IMPORTED)
set_target_properties(ipc PROPERTIES
    IMPORTED_LOCATION "${IPC_LIB_PATH}"
    INTERFACE_INCLUDE_DIRECTORIES "${IPC_INCLUDE_DIR}"
)

target_link_libraries(sender ipc)
target_link_libraries(receiver ipc)
target_include_directories(sender PRIVATE ${IPC_INCLUDE_DIR})
target_include_directories(receiver PRIVATE ${IPC_INCLUDE_DIR})

install(TARGETS sender receiver
    RUNTIME DESTINATION bin
)