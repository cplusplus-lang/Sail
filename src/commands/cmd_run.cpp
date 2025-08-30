#include "cmd_run.hpp"
#include "cmd_build.hpp"
#include "common.hpp"
#include <filesystem>

namespace fs = std::filesystem;

namespace sail::commands {

int cmd_run() {
    if (!Utils::find_project_root()) {
        spdlog::error("Could not find Sail.toml in current directory or any parent directory");
        return 1;
    }
    
    cmd_build();
    
    fs::path executable;
    #ifdef _WIN32
        executable = fs::path(Utils::project_root) / "target" / "cmake" / "build" / (fs::path(Utils::project_root).filename().string() + ".exe");
    #else
        executable = fs::path(Utils::project_root) / "target" / "cmake" / "build" / fs::path(Utils::project_root).filename();
    #endif
    
    if (!fs::exists(executable)) {
        spdlog::error("Executable not found. Build may have failed.");
        return 1;
    }
    
    const std::string run_cmd = executable.string();
    return std::system(run_cmd.c_str()); // NOLINT(cert-env33-c,concurrency-mt-unsafe)
}

} // namespace sail::commands