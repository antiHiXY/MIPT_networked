cmake_minimum_required(VERSION 3.20)

get_filename_component(target_name ${CMAKE_CURRENT_SOURCE_DIR} NAME)

add_library("${target_name}_full" full.cpp)
target_link_libraries("${target_name}_full" enet)

add_executable("${target_name}_client" client.cpp)
target_link_libraries("${target_name}_client" "${target_name}_full")
add_executable("${target_name}_server" server.cpp)
target_link_libraries("${target_name}_server" "${target_name}_full")
add_executable("${target_name}_lobby" lobby.cpp)
target_link_libraries("${target_name}_lobby" "${target_name}_full")
