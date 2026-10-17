cmake_minimum_required(VERSION 3.21)

project(ai_level_loader VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Geode SDK package
find_package(Geode REQUIRED)

add_geode_mod(
    ai_level_loader
    SOURCES
        src/main.cpp
        src/LevelConverter.cpp
)
