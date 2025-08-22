add_executable(${PROJECT_NAME_FROM_TOML} ${PROJECT_SRC}/main.cpp)

target_link_libraries(
  ${PROJECT_NAME_FROM_TOML}
  PRIVATE Sail::Sail_options
          Sail::Sail_warnings)

# Parse Sail.toml to get list of dependencies
parse_sail_toml(${PROJECT_ROOT}/Sail.toml)

# Collect all system libraries to link based on available dependencies
set(SYSTEM_LIBS "")
foreach(DEPENDENCY ${SAIL_DEPENDENCIES})
  string(TOUPPER ${DEPENDENCY} DEPENDENCY_UPPER)
  set(LINK_VAR "SAIL_LINK_${DEPENDENCY_UPPER}")
  if(DEFINED ${LINK_VAR})
    list(APPEND SYSTEM_LIBS ${${LINK_VAR}})
  endif()
endforeach()

# Link system libraries if any were found
if(SYSTEM_LIBS)
  target_link_system_libraries(
    ${PROJECT_NAME_FROM_TOML}
    PRIVATE
    ${SYSTEM_LIBS})
endif()

target_include_directories(${PROJECT_NAME_FROM_TOML} PRIVATE "${CMAKE_BINARY_DIR}/configured_files/include")

add_subdirectory(${PROJECT_SRC}/sample_library sample_library)
