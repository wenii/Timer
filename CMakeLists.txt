
cmake_minimum_required(VERSION 3.16)
project(timer)

aux_source_directory(src SRC)
aux_source_directory(test SRC)

include_directories(src)

add_executable(test ${SRC})
target_link_libraries(test pthread)