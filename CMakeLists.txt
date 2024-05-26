cmake_minimum_required(VERSION 3.14)
project(Ancl)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(spdlog REQUIRED)
find_package(FLEX REQUIRED)

# TODO: unused _localctx parameter in generated CParser
# add_compile_options(-Wall -Wextra -Wpedantic -Werror -fPIC)
add_compile_options(-fPIC)

add_subdirectory(Ancl)
add_subdirectory(play)

# Install:
# sudo apt install libspdlog-dev
# sudo apt install g++-13

# Run:
# cmake -DCMAKE_CXX_COMPILER=/usr/bin/g++-13
