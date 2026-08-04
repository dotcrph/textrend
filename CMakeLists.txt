cmake_minimum_required(VERSION 3.26)

project(
    textrend
    VERSION 0.1.0
    DESCRIPTION "Terminal 3D renderer"
    LANGUAGES CXX
)

if(NOT (WIN32 OR UNIX))
    message(FATAL_ERROR "Platforms other than Windows and Linux/MacOS are not supported!")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

if(CMAKE_BUILD_TYPE MATCHES "Debug")
    add_subdirectory("test")
endif()

add_subdirectory("src")
