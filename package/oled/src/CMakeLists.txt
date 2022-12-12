cmake_minimum_required(VERSION 2.8.12)

project(oled_test)
 
include_directories(include)

file(GLOB SOURCES "source/*.c" "*.c")
 
add_executable(oled_test ${SOURCES})