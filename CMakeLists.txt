cmake_minimum_required(VERSION 3.20)

project(
    MicroSim
    VERSION 0.1.0
    DESCRIPTION "A modular embedded systems simulation framework"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(
    MicroSim
    src/main.cpp
    src/Memory/RAM.cpp
)

target_include_directories(
    MicroSim
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
)