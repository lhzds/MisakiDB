project(GoogleTests)

# 'lib' is the folder with Google Test sources
add_subdirectory(lib)
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

# 'Google_Tests_run' is the target name
# 'test1.cpp tests2.cpp' are source files with tests
add_executable(IndexFileStoreTest "index_file_store_test.cpp")
target_link_libraries(IndexFileStoreTest gtest gtest_main)
target_link_libraries(IndexFileStoreTest MisakiDB)



