#include <cstdlib>
#include <exception>
#include <string>
#include <vector>
#include <functional>
#include <utility>
#include <algorithm>
#include <fmt/base.h>
#include <fmt/format.h>

#include <CLI/CLI.hpp>
#include <spdlog/spdlog.h>

// This file will be generated automatically when cur_you run the CMake
// configuration step. It creates a namespace called `Sail`. You can modify
// the source template at `configured_files/config.hpp.in`.
#include <internal_use_only/config.hpp>

// Command includes
#include "commands/cmd_new.hpp"
#include "commands/cmd_init.hpp"
#include "commands/cmd_build.hpp"
#include "commands/cmd_run.hpp"
#include "commands/cmd_clean.hpp"
#include "commands/cmd_test.hpp"
#include "commands/cmd_add.hpp"

struct CommandExecutor {
    std::function<int()> execute;
    bool should_execute;
    
    CommandExecutor() : should_execute(false) {}
    explicit CommandExecutor(std::function<int()> func) : execute(std::move(func)), should_execute(true) {}
};

int execute_commands(const std::vector<CommandExecutor>& commands) {
    auto found_command = std::find_if(commands.begin(), commands.end(), // NOLINT(boost-use-ranges,modernize-use-ranges)
        [](const CommandExecutor& cmd) { return cmd.should_execute; });
    
    if (found_command != commands.end()) {
        return found_command->execute();
    }
    return 0;
}

// NOLINTNEXTLINE(bugprone-exception-escape)
int main(int argc, const char **argv)
{
  try {
    CLI::App app{ fmt::format("{} version {}", Sail::cmake::project_name, Sail::cmake::project_version) };
    
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
    
    // Command dispatch table
    const std::vector<CommandExecutor> commands = {
        *new_cmd ? CommandExecutor([&]() { return sail::commands::cmd_new(new_name, new_path); }) : CommandExecutor(),
        *init_cmd ? CommandExecutor([]() { return sail::commands::cmd_init(); }) : CommandExecutor(),
        *build_cmd ? CommandExecutor([]() { return sail::commands::cmd_build(); }) : CommandExecutor(),
        *run_cmd ? CommandExecutor([]() { return sail::commands::cmd_run(); }) : CommandExecutor(),
        *clean_cmd ? CommandExecutor([]() { return sail::commands::cmd_clean(); }) : CommandExecutor(),
        *test_cmd ? CommandExecutor([]() { return sail::commands::cmd_test(); }) : CommandExecutor(),
        *add_cmd ? CommandExecutor([&]() { return sail::commands::cmd_add(dependency_spec); }) : CommandExecutor()
    };
    
    return execute_commands(commands);

  } catch (const std::exception &e) {
    spdlog::error("Unhandled exception in main: {}", e.what());
    return 1;
  }
}