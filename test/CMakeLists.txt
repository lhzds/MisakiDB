project(GoogleTests)

add_subdirectory(lib)
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

add_executable(MisakiDBTest "file_store_test.cpp"
        "buffer_pool_manager_test.cpp")
target_link_libraries(MisakiDBTest gtest gtest_main)
target_link_libraries(MisakiDBTest MisakiDB)



