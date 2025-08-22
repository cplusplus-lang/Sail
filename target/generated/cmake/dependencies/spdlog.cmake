# spdlog dependency
if(NOT TARGET spdlog::spdlog)
  cpmaddpackage(
    NAME
    spdlog
    VERSION
    ${SAIL_VERSION_SPDLOG}
    GITHUB_REPOSITORY
    "gabime/spdlog"
    OPTIONS
    "SPDLOG_FMT_EXTERNAL ON")
endif()

# Set target variable for linking in parent scope
set(SAIL_LINK_SPDLOG spdlog::spdlog PARENT_SCOPE)