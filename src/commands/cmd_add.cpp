#include "cmd_add.hpp"
#include "common.hpp"
#include <sstream>
#include <string>
#include <filesystem>
#include <fstream>
#include <cstddef>
#include <spdlog/spdlog.h>
#include <fmt/format.h>

namespace fs = std::filesystem;

namespace sail::commands {

int cmd_add(const std::string& dependency_spec) { // NOLINT(readability-function-cognitive-complexity)
    if (!Utils::find_project_root()) {
        spdlog::error("Could not find Sail.toml in current directory or any parent directory");
        return 1;
    }
    
    // Parse dependency specification (name@version or just name)
    std::string dep_name;
    std::string dep_version;
    
    const size_t at_pos = dependency_spec.find('@');
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
        } else if (dep_name == "qt5") {
            dep_version = "5.15";
        } else if (dep_name == "qt6") {
            dep_version = "6.5";
        } else if (dep_name == "opengl" || dep_name == "threads" || dep_name == "zlib" || dep_name == "curl") {
            dep_version = "system";
        } else {
            spdlog::error("No default version available for '{}'. Please specify version with {}@<version>", dep_name, dep_name);
            return 1;
        }
    }
    
    // Read current Sail.toml
    const fs::path toml_path = fs::path(Utils::project_root) / "Sail.toml";
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
    auto existing_deps = Utils::parse_dependencies_from_toml(toml_content);
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
            const size_t eq_pos = line.find('=');
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

} // namespace sail::commands