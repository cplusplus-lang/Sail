#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <utility>
#include <algorithm>

namespace fs = std::filesystem;

class SailTester {
private:
    std::string sail_executable;
    fs::path test_dir;
    fs::path original_dir;
    int tests_passed = 0;
    int tests_failed = 0;

public:
    // ANSI color codes (work on most terminals)
    static constexpr const char* RED_COLOR = "\033[0;31m";
    static constexpr const char* GREEN_COLOR = "\033[0;32m"; 
    static constexpr const char* BLUE_COLOR = "\033[0;34m";
    static constexpr const char* YELLOW_COLOR = "\033[1;33m";
    static constexpr const char* NO_COLOR = "\033[0m"; // No Color
    
private:

    static void print_header(const std::string& title) {
        std::cout << "\n" << BLUE_COLOR << "=== " << title << " ===" << NO_COLOR << "\n";
    }

    static void print_test(const std::string& test) {
        std::cout << YELLOW_COLOR << "Testing: " << test << NO_COLOR << "\n";
    }

    void print_success(const std::string& message) {
        std::cout << GREEN_COLOR << "✓ " << message << NO_COLOR << "\n";
        tests_passed++;
    }

    void print_error(const std::string& message) {
        std::cout << RED_COLOR << "✗ " << message << NO_COLOR << "\n";
        tests_failed++;
    }

    // Execute a command and return output + exit code
    struct CommandResult {
        std::string output;
        int exit_code{};
    };

    CommandResult execute_command(const std::string& command) {
        CommandResult result;
        
        // Create a temporary file for output
        const fs::path temp_file = test_dir / "temp_output.txt";
        
        std::string full_command = command;
#ifdef _WIN32
        full_command += " > \"" + temp_file.string() + "\" 2>&1";
#else
        full_command += " > '" + temp_file.string() + "' 2>&1";
#endif
        
        result.exit_code = std::system(full_command.c_str()); // NOLINT(cert-env33-c,concurrency-mt-unsafe)
        
        // Read the output
        std::ifstream file(temp_file);
        if (file.is_open()) {
            std::string line;
            while (std::getline(file, line)) {
                result.output += line + "\n";
            }
            file.close();
        }
        
        // Clean up temp file
        fs::remove(temp_file);
        
        return result;
    }

    static std::string read_file(const fs::path& file_path) {
        std::ifstream file(file_path);
        if (!file.is_open()) {
            return "";
        }
        
        std::string content;
        std::string line;
        while (std::getline(file, line)) {
            content += line + "\n";
        }
        file.close();
        return content;
    }

    static bool contains(const std::string& str, const std::string& substr) {
        return str.find(substr) != std::string::npos;
    }

public:
    explicit SailTester(std::string sail_exec) : sail_executable(std::move(sail_exec)), original_dir(fs::current_path()), test_dir(fs::temp_directory_path() / "sail-tests") {
    }

    void setup_tests() {
        print_header("Setting up test environment");
        if (fs::exists(test_dir)) {
            fs::remove_all(test_dir);
        }
        fs::create_directories(test_dir);
        fs::current_path(test_dir);
        print_success("Test environment created at " + test_dir.string());
    }

    void cleanup_tests() {
        print_header("Cleaning up test environment");
        fs::current_path(original_dir);
        if (fs::exists(test_dir)) {
            fs::remove_all(test_dir);
        }
        print_success("Test environment cleaned up");
    }

    void test_version() {
        print_header("Testing --version flag");
        print_test("sail --version");
        
        auto result = execute_command(sail_executable + " --version");
        
        if (result.exit_code == 0 && contains(result.output, "0.1.0")) {
            print_success("Version flag works correctly: " + result.output);
        } else {
            print_error("Version flag failed. Output: " + result.output);
        }
    }

    void test_help() {
        print_header("Testing --help flag");
        print_test("sail --help");
        
        auto result = execute_command(sail_executable + " --help");
        
        if (result.exit_code == 0 && 
            contains(result.output, "SUBCOMMANDS") && 
            contains(result.output, "new") && 
            contains(result.output, "add")) {
            print_success("Help flag works correctly");
        } else {
            print_error("Help flag failed or missing content");
        }
    }

    void test_new_command() {
        print_header("Testing 'sail new' command");
        
        // Test basic new command
        print_test("sail new test-project");
        auto result = execute_command(sail_executable + " new test-project");
        
        if (result.exit_code == 0 &&
            fs::exists("test-project") &&
            fs::exists("test-project/Sail.toml") &&
            fs::exists("test-project/src/main.cpp")) {
            
            print_success("New project created successfully");
            
            // Check Sail.toml content
            const std::string toml_content = read_file("test-project/Sail.toml");
            if (contains(toml_content, "name = \"test-project\"")) {
                print_success("Sail.toml has correct project name");
            } else {
                print_error("Sail.toml has incorrect content");
            }
            
            // Check main.cpp content
            const std::string main_content = read_file("test-project/src/main.cpp");
            if (contains(main_content, "Hello, World!")) {
                print_success("main.cpp has correct template content");
            } else {
                print_error("main.cpp has incorrect content");
            }
            
            // Verify no include directory was created
            if (!fs::exists("test-project/include")) {
                print_success("No include directory created (as expected)");
            } else {
                print_error("Include directory was created (should not be)");
            }
        } else {
            print_error("Failed to create new project");
        }
        
        // Test new command with existing directory
        print_test("sail new test-project (should fail - directory exists)");
        result = execute_command(sail_executable + " new test-project");
        if (result.exit_code != 0) {
            print_success("Correctly failed when directory exists");
        } else {
            print_error("Should have failed when directory exists");
        }
        
        // Cleanup
        if (fs::exists("test-project")) {
            fs::remove_all("test-project");
        }
    }

    void test_init_command() {
        print_header("Testing 'sail init' command");
        
        // Create test directory
        fs::create_directory("init-test");
        fs::current_path("init-test");
        
        print_test("sail init (in empty directory)");
        auto result = execute_command(sail_executable + " init");
        
        if (result.exit_code == 0 &&
            fs::exists("Sail.toml") &&
            fs::exists("src/main.cpp")) {
            
            print_success("Init created project files successfully");
            
            // Check that project name matches directory name
            const std::string toml_content = read_file("Sail.toml");
            if (contains(toml_content, "name = \"init-test\"")) {
                print_success("Sail.toml has correct project name (directory name)");
            } else {
                print_error("Sail.toml has incorrect project name");
            }
        } else {
            print_error("Init failed to create required files");
        }
        
        // Test init when Sail.toml already exists
        print_test("sail init (should fail - Sail.toml exists)");
        result = execute_command(sail_executable + " init");
        if (result.exit_code != 0) {
            print_success("Correctly failed when Sail.toml exists");
        } else {
            print_error("Should have failed when Sail.toml exists");
        }
        
        fs::current_path("..");
        fs::remove_all("init-test");
    }

    void test_add_command() {
        print_header("Testing 'sail add' command");
        
        // Create test project
        execute_command(sail_executable + " new add-test");
        fs::current_path("add-test");
        
        // Test adding dependency with default version
        print_test("sail add fmt");
        auto result = execute_command(sail_executable + " add fmt");
        if (result.exit_code == 0) {
            const std::string toml_content = read_file("Sail.toml");
            if (contains(toml_content, "fmt = \"10.1.1\"")) {
                print_success("Added fmt with default version");
            } else {
                print_error("fmt not added correctly to Sail.toml");
            }
        } else {
            print_error("Failed to add fmt dependency");
        }
        
        // Test adding dependency with specific version
        print_test("sail add spdlog@1.13.0");
        result = execute_command(sail_executable + " add spdlog@1.13.0");
        if (result.exit_code == 0) {
            const std::string toml_content = read_file("Sail.toml");
            if (contains(toml_content, "spdlog = \"1.13.0\"")) {
                print_success("Added spdlog with specific version");
            } else {
                print_error("spdlog not added correctly to Sail.toml");
            }
        } else {
            print_error("Failed to add spdlog dependency");
        }
        
        // Test updating existing dependency
        print_test("sail add fmt@9.1.0 (update existing)");
        result = execute_command(sail_executable + " add fmt@9.1.0");
        if (result.exit_code == 0) {
            const std::string toml_content = read_file("Sail.toml");
            if (contains(toml_content, "fmt = \"9.1.0\"")) {
                print_success("Updated fmt version successfully");
            } else {
                print_error("fmt version not updated correctly");
            }
        } else {
            print_error("Failed to update fmt dependency");
        }
        
        // Test add without project (should fail)
        fs::current_path("..");
        print_test("sail add fmt (outside project - should fail)");
        result = execute_command(sail_executable + " add fmt");
        if (result.exit_code != 0) {
            print_success("Correctly failed outside of project");
        } else {
            print_error("Should have failed outside of project");
        }
        
        fs::remove_all("add-test");
    }

    void test_build_command() {
        print_header("Testing 'sail build' command");
        
        // Create test project with dependencies
        execute_command(sail_executable + " new build-test");
        fs::current_path("build-test");
        
        // Add a dependency and modify main.cpp to use it
        execute_command(sail_executable + " add fmt");
        
        // Write a simple main.cpp that uses fmt
        std::ofstream main_file("src/main.cpp");
        main_file << "#include <fmt/core.h>\n\n";
        main_file << "int main() {\n";
        main_file << "    fmt::print(\"Hello from fmt!\\n\");\n";
        main_file << "    return 0;\n";
        main_file << "}\n";
        main_file.close();
        
        print_test("sail build (with fmt dependency)");
        auto result = execute_command(sail_executable + " build");
        
        if (result.exit_code == 0 &&
            fs::exists("target/cmake/build") &&
            fs::exists("target/cmake/CMakeLists.txt")) {
            
            print_success("Build completed and created artifacts");
            
            // Check that CMakeLists.txt contains dependency
            const std::string cmake_content = read_file("target/cmake/CMakeLists.txt");
            if (contains(cmake_content, "CPMAddPackage") && contains(cmake_content, "fmt")) {
                print_success("CMakeLists.txt contains dependency information");
            } else {
                print_error("CMakeLists.txt missing dependency information");
            }
        } else {
            print_error("Build command failed");
        }
        
        // Test build without project (should fail)
        fs::current_path("..");
        print_test("sail build (outside project - should fail)");
        result = execute_command(sail_executable + " build");
        if (result.exit_code != 0) {
            print_success("Correctly failed outside of project");
        } else {
            print_error("Should have failed outside of project");
        }
        
        fs::remove_all("build-test");
    }

    void test_run_command() {
        print_header("Testing 'sail run' command");
        
        // Create test project
        execute_command(sail_executable + " new run-test");
        fs::current_path("run-test");
        
        print_test("sail run (basic Hello World)");
        auto result = execute_command(sail_executable + " run");
        
        if (result.exit_code == 0 && contains(result.output, "Hello, World!")) {
            print_success("Run command executed successfully");
        } else {
            print_error("Run command failed or unexpected output");
        }
        
        // Test run without project (should fail)
        fs::current_path("..");
        print_test("sail run (outside project - should fail)");
        result = execute_command(sail_executable + " run");
        if (result.exit_code != 0) {
            print_success("Correctly failed outside of project");
        } else {
            print_error("Should have failed outside of project");
        }
        
        fs::remove_all("run-test");
    }

    void test_clean_command() {
        print_header("Testing 'sail clean' command");
        
        // Create test project and build it
        execute_command(sail_executable + " new clean-test");
        fs::current_path("clean-test");
        execute_command(sail_executable + " build");
        
        // Verify build artifacts exist
        if (fs::exists("target")) {
            print_test("sail clean");
            auto result = execute_command(sail_executable + " clean");
            
            if (result.exit_code == 0 && !fs::exists("target")) {
                print_success("Clean command removed target directory");
            } else {
                print_error("Clean command did not remove target directory");
            }
        } else {
            print_error("Build artifacts not found before clean test");
        }
        
        // Test clean when nothing to clean
        print_test("sail clean (nothing to clean)");
        auto result = execute_command(sail_executable + " clean");
        if (result.exit_code == 0) {
            print_success("Clean command handled empty case correctly");
        } else {
            print_error("Clean command failed when nothing to clean");
        }
        
        // Test clean without project (should fail)
        fs::current_path("..");
        print_test("sail clean (outside project - should fail)");
        result = execute_command(sail_executable + " clean");
        if (result.exit_code != 0) {
            print_success("Correctly failed outside of project");
        } else {
            print_error("Should have failed outside of project");
        }
        
        fs::remove_all("clean-test");
    }

    void test_integration_workflow() {
        print_header("Testing complete integration workflow");
        
        print_test("Complete workflow: new -> add -> build -> run -> clean");
        
        // Step 1: Create project
        if (execute_command(sail_executable + " new integration-test").exit_code != 0) {
            print_error("Failed at step 1: create project");
            return;
        }
        
        fs::current_path("integration-test");
        
        // Step 2: Add dependency
        if (execute_command(sail_executable + " add fmt").exit_code != 0) {
            print_error("Failed at step 2: add dependency");
            fs::current_path("..");
            fs::remove_all("integration-test");
            return;
        }
        
        // Step 3: Modify source to use dependency
        std::ofstream main_file("src/main.cpp");
        main_file << "#include <fmt/core.h>\n\n";
        main_file << "int main() {\n";
        main_file << "    fmt::print(\"Integration test successful!\\n\");\n";
        main_file << "    return 0;\n";
        main_file << "}\n";
        main_file.close();
        
        // Step 4: Build
        if (execute_command(sail_executable + " build").exit_code != 0) {
            print_error("Failed at step 4: build project");
            fs::current_path("..");
            fs::remove_all("integration-test");
            return;
        }
        
        // Step 5: Run
        auto run_result = execute_command(sail_executable + " run");
        if (run_result.exit_code == 0 && contains(run_result.output, "Integration test successful!")) {
            print_success("Step 5 passed: run with dependency");
        } else {
            print_error("Failed at step 5: run project");
            fs::current_path("..");
            fs::remove_all("integration-test");
            return;
        }
        
        // Step 6: Clean
        if (execute_command(sail_executable + " clean").exit_code != 0) {
            print_error("Failed at step 6: clean project");
            fs::current_path("..");
            fs::remove_all("integration-test");
            return;
        }
        
        print_success("Complete integration workflow successful");
        
        fs::current_path("..");
        fs::remove_all("integration-test");
    }

    void run_all_tests() {
        print_header("Sail Comprehensive Test Suite");
        std::cout << "Testing Sail executable at: " << sail_executable << "\n";
        
        // Verify sail executable exists
        if (!fs::exists(sail_executable)) {
            std::cout << SailTester::RED_COLOR << "ERROR: Sail executable not found at " << sail_executable << SailTester::NO_COLOR << "\n";
            std::cout << "Please build Sail first or provide correct path as argument" << "\n";
            return;
        }
        
        setup_tests();
        
        test_version();
        test_help();
        test_new_command();
        test_init_command();
        test_add_command();
        test_build_command();
        test_run_command();
        test_clean_command();
        test_integration_workflow();
        
        cleanup_tests();
        
        // Summary
        print_header("Test Results Summary");
        std::cout << "Tests passed: " << SailTester::GREEN_COLOR << tests_passed << SailTester::NO_COLOR << "\n";
        std::cout << "Tests failed: " << SailTester::RED_COLOR << tests_failed << SailTester::NO_COLOR << "\n";
        
        if (tests_failed == 0) {
            std::cout << "\n" << SailTester::GREEN_COLOR << "🎉 All tests passed!" << SailTester::NO_COLOR << "\n";
        } else {
            std::cout << "\n" << SailTester::RED_COLOR << "❌ Some tests failed." << SailTester::NO_COLOR << "\n";
        }
    }
};

int main(int argc, char* argv[]) {
    try {
        std::string sail_executable;
        
        if (argc > 1) {
            sail_executable = std::string(argv[1]); // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        } else {
            // Try to find the executable in common locations
            const std::vector<std::string> possible_paths = {
                "./target/generated/build/sail",
                "../target/generated/build/sail",
                "./build/sail",
                "../build/sail",
                "sail"
            };
            
            auto found_path = std::find_if(possible_paths.begin(), possible_paths.end(), // NOLINT(boost-use-ranges,modernize-use-ranges)
                [](const std::string& path) { return fs::exists(path); });
            
            if (found_path != possible_paths.end()) {
                sail_executable = *found_path;
            }
            
            if (sail_executable.empty()) {
                std::cout << "Usage: " << std::string(argv[0]) << " <path-to-sail-executable>" << "\n"; // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                std::cout << "Or ensure sail executable is in one of these locations:" << "\n";
                for (const auto& path : possible_paths) {
                    std::cout << "  " << path << "\n";
                }
                return 1;
            }
        }
        
        SailTester tester(sail_executable);
        tester.run_all_tests();
        
        return 0;
    } catch (...) {
        std::cout << "An unexpected error occurred during test execution\n";
        return 1;
    }
}