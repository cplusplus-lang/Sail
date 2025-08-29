#include <array>
#include <atomic>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <map>
#include <fmt/base.h>
#include <fmt/format.h>
#include <functional>
#include <optional>

#include <CLI/CLI.hpp>
#include <spdlog/spdlog.h>

#include <lefticus/tools/non_promoting_ints.hpp>

// This file will be generated automatically when cur_you run the CMake
// configuration step. It creates a namespace called `Sail`. You can modify
// the source template at `configured_files/config.hpp.in`.
#include <internal_use_only/config.hpp>
#include <string>
#include <thread>
#include <utility>
#include <vector>

namespace fs = std::filesystem;

class SailTool {
private:
    std::string project_root;
    
    std::map<std::string, std::string> parse_dependencies_from_toml(const std::string& toml_content) {
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
                size_t eq_pos = line.find('=');
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
    
    std::string generate_dependency_cpm_calls(const std::map<std::string, std::string>& dependencies) {
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
            } else {
                cpm_calls += fmt::format("# CPMAddPackage(\"{} version {}\")\n", name, version);
            }
        }
        return cpm_calls;
    }
    
    bool find_project_root() {
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
    
    void ensure_target_cmake_dir() {
        fs::path target_dir = fs::path(project_root) / "target" / "cmake";
        if (!fs::exists(target_dir)) {
            fs::create_directories(target_dir);
        }
    }
    
    void generate_cmakelists() {
        ensure_target_cmake_dir();
        fs::path cmake_file = fs::path(project_root) / "target" / "cmake" / "CMakeLists.txt";
        
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
        std::string cpm_calls = generate_dependency_cpm_calls(dependencies);
        
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

# CPM Package Manager
include(cmake/CPM.cmake)

# Dependencies from Sail.toml
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
            }
        }
        
        file.close();
        spdlog::info("Generated CMakeLists.txt in target/cmake/");
    }
    
    void generate_cpm_cmake() {
        fs::path cpm_file = fs::path(project_root) / "target" / "cmake" / "cmake" / "CPM.cmake";
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
    
public:
    int cmd_new(const std::string& name, const std::string& path = ".") {
        fs::path project_path = fs::path(path) / name;
        
        if (fs::exists(project_path)) {
            spdlog::error("Directory {} already exists", project_path.string());
            return 1;
        }
        
        fs::create_directories(project_path);
        fs::create_directories(project_path / "src");
        
        std::ofstream toml_file(project_path / "Sail.toml");
        toml_file << fmt::format(R"([package]
name = "{}"
version = "0.1.0"
authors = ["Your Name <your.email@example.com>"]
edition = "2021"

[dependencies]
)", name);
        toml_file.close();
        
        std::ofstream main_file(project_path / "src" / "main.cpp");
        main_file << R"(#include <iostream>

int main() {
    std::cout << "Hello, World!" << std::endl;
    return 0;
}
)";
        main_file.close();
        
        spdlog::info("Created package `{}` at {}", name, project_path.string());
        return 0;
    }
    
    int cmd_init() {
        if (fs::exists("Sail.toml")) {
            spdlog::error("Sail.toml already exists in current directory");
            return 1;
        }
        
        std::string name = fs::current_path().filename().string();
        
        std::ofstream toml_file("Sail.toml");
        toml_file << fmt::format(R"([package]
name = "{}"
version = "0.1.0"
authors = ["Your Name <your.email@example.com>"]

[dependencies]
)", name);
        toml_file.close();
        
        if (!fs::exists("src")) {
            fs::create_directory("src");
            std::ofstream main_file("src/main.cpp");
            main_file << R"(#include <iostream>

int main() {
    std::cout << "Hello, World!" << std::endl;
    return 0;
}
)";
            main_file.close();
        }
        
        spdlog::info("Initialized package `{}` in current directory", name);
        return 0;
    }
    
    int cmd_build() {
        if (!find_project_root()) {
            spdlog::error("Could not find Sail.toml in current directory or any parent directory");
            return 1;
        }
        
        generate_cmakelists();
        generate_cpm_cmake();
        
        fs::path build_dir = fs::path(project_root) / "target" / "cmake" / "build";
        fs::create_directories(build_dir);
        
        std::string cmake_cmd;
        #ifdef _WIN32
            cmake_cmd = fmt::format("cd /d {} && cmake .. && cmake --build .", build_dir.string());
        #else
            cmake_cmd = fmt::format("cd {} && cmake .. && make", build_dir.string());
        #endif
        int result = std::system(cmake_cmd.c_str());
        
        if (result == 0) {
            spdlog::info("Build completed successfully");
        } else {
            spdlog::error("Build failed");
        }
        
        return result;
    }
    
    int cmd_run() {
        if (!find_project_root()) {
            spdlog::error("Could not find Sail.toml in current directory or any parent directory");
            return 1;
        }
        
        cmd_build();
        
        fs::path executable;
        #ifdef _WIN32
            executable = fs::path(project_root) / "target" / "cmake" / "build" / (fs::path(project_root).filename().string() + ".exe");
        #else
            executable = fs::path(project_root) / "target" / "cmake" / "build" / fs::path(project_root).filename();
        #endif
        
        if (!fs::exists(executable)) {
            spdlog::error("Executable not found. Build may have failed.");
            return 1;
        }
        
        std::string run_cmd = executable.string();
        return std::system(run_cmd.c_str());
    }
    
    int cmd_clean() {
        if (!find_project_root()) {
            spdlog::error("Could not find Sail.toml in current directory or any parent directory");
            return 1;
        }
        
        fs::path target_dir = fs::path(project_root) / "target";
        if (fs::exists(target_dir)) {
            fs::remove_all(target_dir);
            spdlog::info("Cleaned target directory");
        } else {
            spdlog::info("Nothing to clean");
        }
        
        return 0;
    }
    
    int cmd_test() {
        if (!find_project_root()) {
            spdlog::error("Could not find Sail.toml in current directory or any parent directory");
            return 1;
        }
        
        cmd_build();
        
        spdlog::info("Running tests...");
        std::string test_cmd = fmt::format("cd {}/target/cmake/build && ctest", project_root);
        return std::system(test_cmd.c_str());
    }
    
    int cmd_add(const std::string& dependency_spec) {
        if (!find_project_root()) {
            spdlog::error("Could not find Sail.toml in current directory or any parent directory");
            return 1;
        }
        
        // Parse dependency specification (name@version or just name)
        std::string dep_name;
        std::string dep_version;
        
        size_t at_pos = dependency_spec.find('@');
        if (at_pos != std::string::npos) {
            dep_name = dependency_spec.substr(0, at_pos);
            dep_version = dependency_spec.substr(at_pos + 1);
        } else {
            dep_name = dependency_spec;
            // Default versions for common packages
            if (dep_name == "fmt") {
                dep_version = "10.1.1";
            } else if (dep_name == "spdlog") {
                dep_version = "1.12.0";
            } else if (dep_name == "catch2") {
                dep_version = "3.4.0";
            } else if (dep_name == "cli11") {
                dep_version = "2.3.2";
            } else if (dep_name == "nlohmann_json") {
                dep_version = "3.11.2";
            } else {
                spdlog::error("No default version available for '{}'. Please specify version with {}@<version>", dep_name, dep_name);
                return 1;
            }
        }
        
        // Read current Sail.toml
        fs::path toml_path = fs::path(project_root) / "Sail.toml";
        std::string toml_content;
        std::ifstream toml_file(toml_path);
        if (toml_file.is_open()) {
            std::string line;
            while (std::getline(toml_file, line)) {
                toml_content += line + "\n";
            }
            toml_file.close();
        } else {
            spdlog::error("Could not read Sail.toml");
            return 1;
        }
        
        // Check if dependency already exists
        auto existing_deps = parse_dependencies_from_toml(toml_content);
        if (existing_deps.find(dep_name) != existing_deps.end()) {
            spdlog::info("Updating {} from {} to {}", dep_name, existing_deps[dep_name], dep_version);
        } else {
            spdlog::info("Adding {} version {}", dep_name, dep_version);
        }
        
        // Add or update the dependency
        std::string new_toml_content;
        std::istringstream stream(toml_content);
        std::string line;
        bool in_dependencies_section = false;
        bool dependency_added = false;
        bool dependency_updated = false;
        
        while (std::getline(stream, line)) {
            if (line == "[dependencies]") {
                in_dependencies_section = true;
                new_toml_content += line + "\n";
                continue;
            }
            
            if (in_dependencies_section && line.starts_with("[") && line.ends_with("]") && line != "[dependencies]") {
                // End of dependencies section, add dependency if not yet added
                if (!dependency_added && !dependency_updated) {
                    new_toml_content += fmt::format("{} = \"{}\"\n", dep_name, dep_version);
                    dependency_added = true;
                }
                in_dependencies_section = false;
                new_toml_content += line + "\n";
                continue;
            }
            
            if (in_dependencies_section && !line.empty() && line.find('=') != std::string::npos) {
                size_t eq_pos = line.find('=');
                std::string key = line.substr(0, eq_pos);
                key.erase(0, key.find_first_not_of(" \t"));
                key.erase(key.find_last_not_of(" \t") + 1);
                
                if (key == dep_name) {
                    new_toml_content += fmt::format("{} = \"{}\"\n", dep_name, dep_version);
                    dependency_updated = true;
                    continue;
                }
            }
            
            new_toml_content += line + "\n";
        }
        
        // If we haven't added the dependency yet (no dependencies section exists or we're at the end)
        if (!dependency_added && !dependency_updated) {
            if (!in_dependencies_section) {
                new_toml_content += "\n[dependencies]\n";
            }
            new_toml_content += fmt::format("{} = \"{}\"\n", dep_name, dep_version);
        }
        
        // Write the updated TOML back
        std::ofstream output_file(toml_path);
        output_file << new_toml_content;
        output_file.close();
        
        spdlog::info("Updated Sail.toml");
        return 0;
    }
};

// NOLINTNEXTLINE(bugprone-exception-escape)
int main(int argc, const char **argv)
{
  try {
    CLI::App app{ fmt::format("{} version {}", Sail::cmake::project_name, Sail::cmake::project_version) };
    
    SailTool sail;
    
    bool show_version = false;
    app.add_flag("--version", show_version, "Show version information");
    
    auto* new_cmd = app.add_subcommand("new", "Create a new Sail package");
    std::string new_name;
    std::string new_path = ".";
    new_cmd->add_option("name", new_name, "Name of the new package")->required();
    new_cmd->add_option("--path", new_path, "Path where to create the package");
    
    auto* init_cmd = app.add_subcommand("init", "Create a new Sail package in current directory");
    
    auto* build_cmd = app.add_subcommand("build", "Compile the current package");
    
    auto* run_cmd = app.add_subcommand("run", "Run the current package");
    
    auto* clean_cmd = app.add_subcommand("clean", "Remove the target directory");
    
    auto* test_cmd = app.add_subcommand("test", "Run tests");
    
    auto* add_cmd = app.add_subcommand("add", "Add a dependency to the current package");
    std::string dependency_spec;
    add_cmd->add_option("dependency", dependency_spec, "Dependency specification (name@version or name)")->required();

    CLI11_PARSE(app, argc, argv);

    if (show_version) {
      fmt::print("{}\n", Sail::cmake::project_version);
      return EXIT_SUCCESS;
    }
    
    if (*new_cmd) {
        return sail.cmd_new(new_name, new_path);
    }
    
    if (*init_cmd) {
        return sail.cmd_init();
    }
    
    if (*build_cmd) {
        return sail.cmd_build();
    }
    
    if (*run_cmd) {
        return sail.cmd_run();
    }
    
    if (*clean_cmd) {
        return sail.cmd_clean();
    }
    
    if (*test_cmd) {
        return sail.cmd_test();
    }
    
    if (*add_cmd) {
        return sail.cmd_add(dependency_spec);
    }

  } catch (const std::exception &e) {
    spdlog::error("Unhandled exception in main: {}", e.what());
    return 1;
  }
  
  return 0;
}
