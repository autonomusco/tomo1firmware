cmake_minimum_required(VERSION 3.16)

# Tell ESP-IDF where the project’s main sources are
set(PROJECT_NAME tomo1firmware)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(${PROJECT_NAME})
