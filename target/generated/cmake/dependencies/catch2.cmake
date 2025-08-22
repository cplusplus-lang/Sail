# Catch2 dependency
if(NOT TARGET Catch2::Catch2WithMain)
  cpmaddpackage("gh:catchorg/Catch2@${SAIL_VERSION_CATCH2}")
endif()

# Set target variable for linking in parent scope (typically used for tests, not main executable)
set(SAIL_LINK_CATCH2 Catch2::Catch2WithMain PARENT_SCOPE)

