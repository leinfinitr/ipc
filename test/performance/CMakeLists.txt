file(GLOB_RECURSE TEST_PERF_SRCS
    ${CMAKE_CURRENT_LIST_DIR}/*.c
    ${CMAKE_CURRENT_LIST_DIR}/*.cpp
)
foreach(TEST_PERF_SRC ${TEST_PERF_SRCS})
    get_filename_component(TEST_TARGET ${TEST_PERF_SRC} NAME_WE)
    add_executable(${TEST_TARGET}
        ${TEST_PERF_SRC}
    )
    target_include_directories(${TEST_TARGET} PRIVATE
        ${LIBIPC_INCLUDE_DIR}
    )
    target_link_libraries(${TEST_TARGET} PRIVATE
        ipc
    )
    install(TARGETS ${TEST_TARGET} RUNTIME DESTINATION bin)
    set_target_properties(${TEST_TARGET} PROPERTIES
        INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/lib
        BUILD_WITH_INSTALL_RPATH TRUE
        INSTALL_RPATH_USE_LINK_PATH TRUE
    )
endforeach()
