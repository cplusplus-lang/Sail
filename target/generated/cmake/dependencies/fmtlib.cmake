# fmtlib dependency
if(NOT TARGET fmtlib::fmtlib)
  cpmaddpackage("gh:fmtlib/fmt#${SAIL_VERSION_FMTLIB}")
endif()

# Set target variable for linking in parent scope
set(SAIL_LINK_FMTLIB fmt::fmt PARENT_SCOPE)