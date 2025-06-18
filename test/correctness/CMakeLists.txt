project(ipc-test-correctness)

file(GLOB TEST_CORRECT_SCRS ${CMAKE_CURRENT_LIST_DIR}/*.cpp)
add_executable(${PROJECT_NAME} ${TEST_CORRECT_SCRS})

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(${PROJECT_NAME} PRIVATE -g)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        ENABLE_EXPORTS ON
        LINK_OPTIONS "-g"
    )
endif()

target_include_directories(${PROJECT_NAME} PRIVATE
    ${LIBIPC_INCLUDE_DIR}
    ${LIBIPC_PROJECT_DIR}/3rdparty/gtest/include
)

target_link_libraries(${PROJECT_NAME} 
    gtest
    ipc
    pthread
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    INSTALL_RPATH_USE_LINK_PATH TRUE
    POSITION_INDEPENDENT_CODE ON
)

install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
)
