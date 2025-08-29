cmake_minimum_required(VERSION 3.15)

# Read project info from Sail.toml
file(READ "${CMAKE_SOURCE_DIR}/../../Sail.toml" SAIL_TOML)
string(REGEX MATCH "name = \"([^\"]+)\"" _ "${SAIL_TOML}")
set(PROJECT_NAME ${CMAKE_MATCH_1})
string(REGEX MATCH "version = \"([^\"]+)\"" _ "${SAIL_TOML}")
set(PROJECT_VERSION ${CMAKE_MATCH_1})

project(${PROJECT_NAME} VERSION ${PROJECT_VERSION})

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# System Dependencies
# Qt6 Setup
find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
qt_standard_project_setup()


# CPM Package Manager
include(cmake/CPM.cmake)

# CPM Dependencies from Sail.toml
# CPMAddPackage("qt6 version 6.5")

# Source files
file(GLOB_RECURSE SOURCES "${CMAKE_SOURCE_DIR}/../../src/*.cpp")
file(GLOB_RECURSE HEADERS "${CMAKE_SOURCE_DIR}/../../src/*.hpp" "${CMAKE_SOURCE_DIR}/../../include/*.hpp")

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})
target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/../../src" "${CMAKE_SOURCE_DIR}/../../include")

# Link dependencies
target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Core Qt6::Widgets)
