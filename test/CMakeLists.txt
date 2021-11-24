project(GoogleTests)

add_subdirectory(lib)
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

add_executable(MisakiDBTest "file_store_test.cpp"
        "buffer_pool_manager_test.cpp"
        "index_file_manager_test.cpp"
        "data_file_manager_test.cpp"
        "b_plus_tree_test.cpp"
        "data_accessor_test.cpp")
target_link_libraries(MisakiDBTest gtest gtest_main)
target_link_libraries(MisakiDBTest MisakiDB)



