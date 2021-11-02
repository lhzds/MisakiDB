project(GoogleTests)

add_subdirectory(lib)
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

add_executable(FileStoreTest "file_store_test.cpp")
target_link_libraries(FileStoreTest gtest gtest_main)
target_link_libraries(FileStoreTest MisakiDB)



