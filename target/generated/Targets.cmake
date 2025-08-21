add_executable(${PROJECT_NAME_FROM_TOML} ${PROJECT_SRC}/main.cpp)

target_link_libraries(
  ${PROJECT_NAME_FROM_TOML}
  PRIVATE Sail::Sail_options
          Sail::Sail_warnings)

target_link_system_libraries(
  ${PROJECT_NAME_FROM_TOML}
  PRIVATE
          CLI11::CLI11
          fmt::fmt
          spdlog::spdlog
          lefticus::tools
          ftxui::screen
          ftxui::dom
          ftxui::component)

target_include_directories(${PROJECT_NAME_FROM_TOML} PRIVATE "${CMAKE_BINARY_DIR}/configured_files/include")

add_subdirectory(${PROJECT_SRC}/sample_library sample_library)
