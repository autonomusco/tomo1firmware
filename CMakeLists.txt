# Minimum CMake version required by ESP-IDF
cmake_minimum_required(VERSION 3.16)

# Set the project name
set(PROJECT_NAME "tomo1firmware")

# Include ESP-IDF build system
include($ENV{IDF_PATH}/tools/cmake/project.cmake)

# Declare the project
project(${PROJECT_NAME})