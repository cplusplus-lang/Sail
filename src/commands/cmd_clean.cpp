#include "cmd_clean.hpp"
#include "common.hpp"
#include <filesystem>

namespace fs = std::filesystem;

namespace sail::commands {

int cmd_clean() {
    if (!Utils::find_project_root()) {
        spdlog::error("Could not find Sail.toml in current directory or any parent directory");
        return 1;
    }
    
    const fs::path target_dir = fs::path(Utils::project_root) / "target";
    if (fs::exists(target_dir)) {
        fs::remove_all(target_dir);
        spdlog::info("Cleaned target directory");
    } else {
        spdlog::info("Nothing to clean");
    }
    
    return 0;
}

} // namespace sail::commands