cmake_minimum_required(VERSION 3.15)
project(my_redis CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(server server.cpp)
add_executable(client client.cpp)

target_compile_options(server PRIVATE -Wall -Wextra -Werror -O1 -g)
target_compile_options(server PRIVATE -Wall -Wextra -Werror -O1 -g)
