#include "cmd_init.hpp"
#include "common.hpp"

namespace sail::commands {

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
    std::cout << "Hello, World!" << "\n";
    return 0;
}
)";
        main_file.close();
    }
    
    spdlog::info("Initialized package `{}` in current directory", name);
    return 0;
}

} // namespace sail::commands