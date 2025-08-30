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

struct DependencySpec {
    std::string name;
    std::string version;
};

std::string get_default_version(const std::string& dep_name) {
    if (dep_name == "fmt") {
        return "10.1.1";
    } else if (dep_name == "spdlog") {
        return "1.12.0";
    } else if (dep_name == "catch2") {
        return "3.4.0";
    } else if (dep_name == "cli11") {
        return "2.3.2";
    } else if (dep_name == "nlohmann_json") {
        return "3.11.2";
    } else if (dep_name == "qt5") {
        return "5.15";
    } else if (dep_name == "qt6") {
        return "6.5";
    } else if (dep_name == "opengl" || dep_name == "threads" || dep_name == "zlib" || dep_name == "curl") {
        return "system";
    } else {
        return "";  // Empty string indicates no default version
    }
}

DependencySpec parse_dependency_spec(const std::string& dependency_spec) {
    DependencySpec spec;
    
    const size_t at_pos = dependency_spec.find('@');
    if (at_pos != std::string::npos) {
        spec.name = dependency_spec.substr(0, at_pos);
        spec.version = dependency_spec.substr(at_pos + 1);
    } else {
        spec.name = dependency_spec;
        spec.version = get_default_version(spec.name);
    }
    
    return spec;
}

std::string read_toml_file(const fs::path& toml_path) {
    std::string toml_content;
    std::ifstream toml_file(toml_path);
    if (toml_file.is_open()) {
        std::string line;
        while (std::getline(toml_file, line)) {
            toml_content += line + "\n";
        }
        toml_file.close();
    }
    return toml_content;
}

std::string update_toml_with_dependency(const std::string& toml_content, const std::string& dep_name, const std::string& dep_version) {
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
    
    return new_toml_content;
}

int cmd_add(const std::string& dependency_spec) {
    if (!Utils::find_project_root()) {
        spdlog::error("Could not find Sail.toml in current directory or any parent directory");
        return 1;
    }
    
    // Parse dependency specification
    DependencySpec spec = parse_dependency_spec(dependency_spec);
    
    if (spec.version.empty()) {
        spdlog::error("No default version available for '{}'. Please specify version with {}@<version>", spec.name, spec.name);
        return 1;
    }
    
    // Read current Sail.toml
    const fs::path toml_path = fs::path(Utils::project_root) / "Sail.toml";
    std::string toml_content = read_toml_file(toml_path);
    
    if (toml_content.empty()) {
        spdlog::error("Could not read Sail.toml");
        return 1;
    }
    
    // Check if dependency already exists
    auto existing_deps = Utils::parse_dependencies_from_toml(toml_content);
    if (existing_deps.find(spec.name) != existing_deps.end()) {
        spdlog::info("Updating {} from {} to {}", spec.name, existing_deps[spec.name], spec.version);
    } else {
        spdlog::info("Adding {} version {}", spec.name, spec.version);
    }
    
    // Update TOML content
    std::string new_toml_content = update_toml_with_dependency(toml_content, spec.name, spec.version);
    
    // Write the updated TOML back
    std::ofstream output_file(toml_path);
    output_file << new_toml_content;
    output_file.close();
    
    spdlog::info("Updated Sail.toml");
    return 0;
}

} // namespace sail::commands