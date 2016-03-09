cmake_minimum_required(VERSION 2.8.4)
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/cmake/ArduinoToolchain.cmake)
set(PROJECT_NAME BNB_Main)
set(LIB_DIR lib)
include_directories(${LIB_DIR})
project(${PROJECT_NAME})

set(${CMAKE_PROJECT_NAME}_SKETCH BNB_Main.ino)
generate_arduino_firmware(${CMAKE_PROJECT_NAME})
