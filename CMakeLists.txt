cmake_minimum_required(VERSION 3.4)
project(gbc)

add_definitions(-std=c++20 -Wall -Werror -Wextra -pedantic -g)

add_executable(gbc src/main.cc)
