# CLI11 dependency
if(NOT TARGET CLI11::CLI11)
  cpmaddpackage("gh:CLIUtils/CLI11@${SAIL_VERSION_CLI11}")
endif()

# Set target variable for linking in parent scope
set(SAIL_LINK_CLI11 CLI11::CLI11 PARENT_SCOPE)
