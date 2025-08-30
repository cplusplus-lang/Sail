#include "cmd_build.hpp"
#include "common.hpp"
#include <filesystem>
#include <string>
#include <cstdlib>
#include <spdlog/spdlog.h>
#include <fmt/format.h>

namespace sail::commands {

int cmd_build() {
    if (!Utils::find_project_root()) {
        spdlog::error("Could not find Sail.toml in current directory or any parent directory");
        return 1;
    }
    
    Utils::generate_cmakelists();
    Utils::generate_cpm_cmake();
    
    const fs::path build_dir = fs::path(Utils::project_root) / "target" / "cmake" / "build";
    fs::create_directories(build_dir);
    
    std::string cmake_cmd;
    #ifdef _WIN32
        cmake_cmd = fmt::format("cd /d {} && cmake .. && cmake --build .", build_dir.string());
    #else
        cmake_cmd = fmt::format("cd {} && cmake .. && make", build_dir.string());
    #endif
    const int result = std::system(cmake_cmd.c_str()); // NOLINT(cert-env33-c,concurrency-mt-unsafe)
    
    if (result == 0) {
        spdlog::info("Build completed successfully");
    } else {
        spdlog::error("Build failed");
    }
    
    return result;
}

} // namespace sail::commands