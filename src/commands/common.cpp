#include "common.hpp"
#include <cstddef>
#include <fstream>
#include <sstream>
#include <string>
#include <filesystem>
#include <map>
#include <spdlog/spdlog.h>
#include <fmt/format.h>

namespace fs = std::filesystem;

namespace sail::commands {

std::string Utils::project_root;

bool Utils::find_project_root() {
    fs::path current = fs::current_path();
    while (current != current.parent_path()) {
        if (fs::exists(current / "Sail.toml")) {
            project_root = current.string();
            return true;
        }
        current = current.parent_path();
    }
    return false;
}

std::map<std::string, std::string> Utils::parse_dependencies_from_toml(const std::string& toml_content) {
    std::map<std::string, std::string> dependencies;
    std::istringstream stream(toml_content);
    std::string line;
    bool in_dependencies_section = false;
    
    while (std::getline(stream, line)) {
        line.erase(0, line.find_first_not_of(" \t"));
        line.erase(line.find_last_not_of(" \t") + 1);
        
        if (line == "[dependencies]") {
            in_dependencies_section = true;
            continue;
        }
        
        if (line.starts_with("[") && line.ends_with("]") && line != "[dependencies]") {
            in_dependencies_section = false;
            continue;
        }
        
        if (in_dependencies_section && !line.empty() && line.find('=') != std::string::npos) {
            const size_t eq_pos = line.find('=');
            std::string key = line.substr(0, eq_pos);
            std::string value = line.substr(eq_pos + 1);
            
            key.erase(0, key.find_first_not_of(" \t"));
            key.erase(key.find_last_not_of(" \t") + 1);
            value.erase(0, value.find_first_not_of(" \t\""));
            value.erase(value.find_last_not_of(" \t\"") + 1);
            
            dependencies[key] = value;
        }
    }
    
    return dependencies;
}

std::string Utils::generate_dependency_cpm_calls(const std::map<std::string, std::string>& dependencies) {
    std::string cpm_calls;
    for (const auto& [name, version] : dependencies) {
        if (name == "fmt") {
            cpm_calls += fmt::format("CPMAddPackage(\"gh:fmtlib/fmt#{}\")\n", version);
        } else if (name == "spdlog") {
            cpm_calls += fmt::format("CPMAddPackage(\"gh:gabime/spdlog#v{}\")\n", version);
        } else if (name == "catch2") {
            cpm_calls += fmt::format("CPMAddPackage(\"gh:catchorg/Catch2#v{}\")\n", version);
        } else if (name == "cli11") {
            cpm_calls += fmt::format("CPMAddPackage(\"gh:CLIUtils/CLI11#v{}\")\n", version);
        } else if (name == "nlohmann_json") {
            cpm_calls += fmt::format("CPMAddPackage(\"gh:nlohmann/json#v{}\")\n", version);
        } else if (name == "eigen3") {
            cpm_calls += fmt::format("CPMAddPackage(\"gh:libeigen/eigen#{}\")\n", version);
        } else if (name == "boost") {
            cpm_calls += fmt::format("CPMAddPackage(\"gh:boostorg/boost#boost-{}\")\n", version);
        } else if (name == "opencv") {
            cpm_calls += fmt::format("CPMAddPackage(\"gh:opencv/opencv#{}\")\n", version);
        } else {
            cpm_calls += fmt::format("# CPMAddPackage(\"{} version {}\")\n", name, version);
        }
    }
    return cpm_calls;
}

std::string Utils::generate_system_dependencies(const std::map<std::string, std::string>& dependencies) {
    std::string system_deps;
    
    for (const auto& [name, version] : dependencies) {
        if (name == "qt5") {
            system_deps += fmt::format(R"(# Qt5 Setup
find_package(Qt5 {} REQUIRED COMPONENTS Core Widgets)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

)", version);
        } else if (name == "qt6") {
            system_deps += fmt::format(R"(# Qt6 Setup
find_package(Qt6 {} REQUIRED COMPONENTS Core Widgets)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
qt_standard_project_setup()

)", version);
        } else if (name == "opengl") {
            system_deps += "find_package(OpenGL REQUIRED)\n";
        } else if (name == "threads") {
            system_deps += "find_package(Threads REQUIRED)\n";
        } else if (name == "zlib") {
            system_deps += "find_package(ZLIB REQUIRED)\n";
        } else if (name == "curl") {
            system_deps += "find_package(CURL REQUIRED)\n";
        } else if (name == "pkg-config") {
            system_deps += "find_package(PkgConfig REQUIRED)\n";
        }
    }
    
    return system_deps;
}

void Utils::ensure_target_cmake_dir() {
    const fs::path target_dir = fs::path(project_root) / "target" / "cmake";
    if (!fs::exists(target_dir)) {
        fs::create_directories(target_dir);
    }
}

void Utils::generate_cmakelists() {
    ensure_target_cmake_dir();
    const fs::path cmake_file = fs::path(project_root) / "target" / "cmake" / "CMakeLists.txt";
    
    // Read existing Sail.toml to get dependencies
    std::string toml_content;
    std::ifstream toml_file(fs::path(project_root) / "Sail.toml");
    if (toml_file.is_open()) {
        std::string line;
        while (std::getline(toml_file, line)) {
            toml_content += line + "\n";
        }
        toml_file.close();
    }
    
    auto dependencies = parse_dependencies_from_toml(toml_content);
    const std::string system_deps = generate_system_dependencies(dependencies);
    const std::string cpm_calls = generate_dependency_cpm_calls(dependencies);
    
    std::ofstream file(cmake_file);
    file << R"(cmake_minimum_required(VERSION 3.15)

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
)";
    file << system_deps;
    file << R"(
# CPM Package Manager
include(cmake/CPM.cmake)

# CPM Dependencies from Sail.toml
)";
    file << cpm_calls;
    file << R"(
# Source files
file(GLOB_RECURSE SOURCES "${CMAKE_SOURCE_DIR}/../../src/*.cpp")
file(GLOB_RECURSE HEADERS "${CMAKE_SOURCE_DIR}/../../src/*.hpp" "${CMAKE_SOURCE_DIR}/../../include/*.hpp")

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})
target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/../../src" "${CMAKE_SOURCE_DIR}/../../include")

# Link dependencies
)";
    
    // Add target_link_libraries for known dependencies
    for (const auto& [name, version] : dependencies) {
        if (name == "fmt") {
            file << "target_link_libraries(${PROJECT_NAME} PRIVATE fmt::fmt)\n";
        } else if (name == "spdlog") {
            file << "target_link_libraries(${PROJECT_NAME} PRIVATE spdlog::spdlog)\n";
        } else if (name == "catch2") {
            file << "target_link_libraries(${PROJECT_NAME} PRIVATE Catch2::Catch2WithMain)\n";
        } else if (name == "cli11") {
            file << "target_link_libraries(${PROJECT_NAME} PRIVATE CLI11::CLI11)\n";
        } else if (name == "nlohmann_json") {
            file << "target_link_libraries(${PROJECT_NAME} PRIVATE nlohmann_json::nlohmann_json)\n";
        } else if (name == "eigen3") {
            file << "target_link_libraries(${PROJECT_NAME} PRIVATE Eigen3::Eigen)\n";
        } else if (name == "boost") {
            file << "target_link_libraries(${PROJECT_NAME} PRIVATE Boost::boost)\n";
        } else if (name == "opencv") {
            file << "target_link_libraries(${PROJECT_NAME} PRIVATE opencv_core opencv_imgproc opencv_imgcodecs)\n";
        } else if (name == "qt5") {
            file << "target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Core Qt5::Widgets)\n";
        } else if (name == "qt6") {
            file << "target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Core Qt6::Widgets)\n";
        } else if (name == "opengl") {
            file << "target_link_libraries(${PROJECT_NAME} PRIVATE OpenGL::GL)\n";
        } else if (name == "threads") {
            file << "target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads)\n";
        } else if (name == "zlib") {
            file << "target_link_libraries(${PROJECT_NAME} PRIVATE ZLIB::ZLIB)\n";
        } else if (name == "curl") {
            file << "target_link_libraries(${PROJECT_NAME} PRIVATE CURL::libcurl)\n";
        }
    }
    
    file.close();
    spdlog::info("Generated CMakeLists.txt in target/cmake/");
}

void Utils::generate_cpm_cmake() {
    const fs::path cpm_file = fs::path(project_root) / "target" / "cmake" / "cmake" / "CPM.cmake";
    fs::create_directories(cpm_file.parent_path());
    
    std::ofstream file(cpm_file);
    file << R"(# CPM.cmake - A simple Git-based package manager for CMake
# Download CPM.cmake from GitHub if not exists
set(CPM_DOWNLOAD_VERSION 0.38.1)
if(CPM_SOURCE_CACHE)
  set(CPM_DOWNLOAD_LOCATION "${CPM_SOURCE_CACHE}/cpm/CPM_${CPM_DOWNLOAD_VERSION}.cmake")
elseif(DEFINED ENV{CPM_SOURCE_CACHE})
  set(CPM_DOWNLOAD_LOCATION "$ENV{CPM_SOURCE_CACHE}/cpm/CPM_${CPM_DOWNLOAD_VERSION}.cmake")
else()
  set(CPM_DOWNLOAD_LOCATION "${CMAKE_BINARY_DIR}/cmake/CPM_${CPM_DOWNLOAD_VERSION}.cmake")
endif()

if(NOT (EXISTS ${CPM_DOWNLOAD_LOCATION}))
  message(STATUS "Downloading CPM.cmake to ${CPM_DOWNLOAD_LOCATION}")
  file(DOWNLOAD
       https://github.com/cpm-cmake/CPM.cmake/releases/download/v${CPM_DOWNLOAD_VERSION}/CPM.cmake
       ${CPM_DOWNLOAD_LOCATION}
  )
endif()

include(${CPM_DOWNLOAD_LOCATION})
)";
    file.close();
    spdlog::info("Generated CPM.cmake in target/cmake/cmake/");
}

} // namespace sail::commands