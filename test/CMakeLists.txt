project(test-ipc)

file(GLOB TEST_SCRS ${LIBIPC_PROJECT_DIR}/test/*.cpp)
add_executable(${PROJECT_NAME} ${TEST_SCRS})

target_include_directories(${PROJECT_NAME} PRIVATE
    ${LIBIPC_INCLUDE_DIR}
    ${LIBIPC_PROJECT_DIR}/3rdparty/gtest/include
)

target_link_libraries(${PROJECT_NAME} 
    gtest
    ipc
)

install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
)
