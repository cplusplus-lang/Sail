#include "cmd_test.hpp"
#include "cmd_build.hpp"
#include "common.hpp"
#include <string>
#include <cstdlib>
#include <spdlog/spdlog.h>
#include <fmt/format.h>

namespace sail::commands {

int cmd_test() {
    if (!Utils::find_project_root()) {
        spdlog::error("Could not find Sail.toml in current directory or any parent directory");
        return 1;
    }
    
    cmd_build();
    
    spdlog::info("Running tests...");
    const std::string test_cmd = fmt::format("cd {}/target/cmake/build && ctest", Utils::project_root);
    return std::system(test_cmd.c_str()); // NOLINT(cert-env33-c,concurrency-mt-unsafe)
}

} // namespace sail::commands