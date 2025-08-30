#include "cmd_new.hpp"
#include <filesystem>
#include <string>
#include <fstream>
#include <spdlog/spdlog.h>
#include <fmt/format.h>

namespace sail::commands {

int cmd_new(const std::string& name, const std::string& path) {
    const fs::path project_path = fs::path(path) / name;
    
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
    std::cout << "Hello, World!" << "\n";
    return 0;
}
)";
    main_file.close();
    
    spdlog::info("Created package `{}` at {}", name, project_path.string());
    return 0;
}

} // namespace sail::commands