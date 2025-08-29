add_executable(${PROJECT_NAME_FROM_TOML} 
    ${PROJECT_SRC}/main.cpp
    ${PROJECT_SRC}/commands/common.cpp
    ${PROJECT_SRC}/commands/cmd_new.cpp
    ${PROJECT_SRC}/commands/cmd_init.cpp
    ${PROJECT_SRC}/commands/cmd_build.cpp
    ${PROJECT_SRC}/commands/cmd_run.cpp
    ${PROJECT_SRC}/commands/cmd_clean.cpp
    ${PROJECT_SRC}/commands/cmd_test.cpp
    ${PROJECT_SRC}/commands/cmd_add.cpp)

target_link_libraries(
  ${PROJECT_NAME_FROM_TOML}
  PRIVATE Sail::Sail_options
          Sail::Sail_warnings)

target_link_system_libraries(
  ${PROJECT_NAME_FROM_TOML}
  PRIVATE
          CLI11::CLI11
          fmt::fmt
          spdlog::spdlog)

target_include_directories(${PROJECT_NAME_FROM_TOML} PRIVATE "${CMAKE_BINARY_DIR}/configured_files/include")

add_subdirectory(${PROJECT_SRC}/sample_library sample_library)
