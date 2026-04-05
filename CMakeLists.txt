cmake_minimum_required(VERSION 3.20)

project(AudioSpectrumAnalyzer)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

add_executable(${PROJECT_NAME} src/main.cpp)
