# FTXUI dependency
if(NOT TARGET ftxui::screen)
  cpmaddpackage("gh:ArthurSonzogni/FTXUI@${SAIL_VERSION_FTXUI}")
endif()

# Set target variable for linking in parent scope based on components
set(LINK_TARGETS "")
if(DEFINED SAIL_COMPONENTS_FTXUI)
  foreach(COMPONENT ${SAIL_COMPONENTS_FTXUI})
    list(APPEND LINK_TARGETS ftxui::${COMPONENT})
  endforeach()
else()
  # Default components if none specified
  set(LINK_TARGETS ftxui::screen ftxui::dom ftxui::component)
endif()

set(SAIL_LINK_FTXUI ${LINK_TARGETS} PARENT_SCOPE)