cmake_minimum_required(VERSION 2.8.12)

project(oled_player)
 
include_directories(include)

file(GLOB SOURCES "*.cpp" "*.c")
 
add_executable(oled_player ${SOURCES})