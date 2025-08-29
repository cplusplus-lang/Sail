#pragma once

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <map>
#include <string>
#include <sstream>
#include <fmt/format.h>
#include <spdlog/spdlog.h>

namespace fs = std::filesystem;

namespace sail::commands {

// Shared utility functions
class Utils {
public:
    static std::string project_root;
    
    static bool find_project_root();
    static std::map<std::string, std::string> parse_dependencies_from_toml(const std::string& toml_content);
    static std::string generate_dependency_cpm_calls(const std::map<std::string, std::string>& dependencies);
    static std::string generate_system_dependencies(const std::map<std::string, std::string>& dependencies);
    static void ensure_target_cmake_dir();
    static void generate_cmakelists();
    static void generate_cpm_cmake();
};

} // namespace sail::commands