cmake_minimum_required(VERSION 3.21)
project(MisakiDB)

set(CMAKE_CXX_STANDARD 20)
#file(GLOB SOURCES
#        "src/*.cpp"
#        "src/file_store/*.cpp"
#        "src/page/*.cpp"
#        "src/buffer/*.cpp")
#set(SOURCES "src/file_store/file_store.cpp"
#        "src/page/index_file_header.cpp"
#        "src/page/free_space_map_file_header.cpp"
#        "src/page/data_file_header.cpp"
#        "src/buffer/lru_replacer.cpp"
#        "src/buffer/buffer_pool_manager.cpp"
#        "src/file_manager/index_file_manager.cpp"
#        "src/page/index_file_free_page.cpp"
#        "src/buffer/index_buffer_pool_manager.cpp"
#        "src/file_manager/data_file_manager.cpp"
#        "src/page/free_space_map_file_page.cpp"
#        "src/page/data_file_page.cpp"
#        "src/buffer/data_buffer_pool_manager.cpp"
#        "src/page/b_plus_tree_internal_page.cpp"
#        "src/page/b_plus_tree_leaf_page.cpp"
#        "src/page/b_plus_tree_page.cpp"
#        "src/index/index_iterator.cpp"
#        "src/index/b_plus_tree.cpp"
#        "src/data_accessor.cpp"
#        "src/page/blob_file_header.cpp"
#        "src/page/blob_file_free_page.cpp"
#        "src/page/blob_file_page.cpp"
#        )

add_compile_options(-fconcepts)
file(GLOB SOURCES
        "src/*.cpp"
        "src/file_store/*.cpp"
        "src/file_manager/*.cpp"
        "src/page/*.cpp"
        "src/buffer/*.cpp"
        "src/index/*.cpp")

add_library(MisakiDB ${SOURCES})
add_executable(main "src/main.cpp")
target_link_libraries(main MisakiDB ws2_32)

include_directories("src/include")
add_subdirectory(test)
