include(cmake/CPM.cmake)
include(cmake/SailTomlParser.cmake)

# Done as a function so that updates to variables like
# CMAKE_CXX_FLAGS don't propagate out to other
# targets
function(Sail_setup_dependencies)
  # For each dependency, see if it's
  # already been provided to us by a parent project

  # Include individual library dependency files based on Sail.toml
  foreach(DEPENDENCY ${SAIL_DEPENDENCIES})
    set(DEPENDENCY_FILE "cmake/dependencies/${DEPENDENCY}.cmake")
    if(EXISTS ${CMAKE_CURRENT_LIST_DIR}/${DEPENDENCY_FILE})
      message(STATUS "Including dependency: ${DEPENDENCY}")
      include(${DEPENDENCY_FILE})
    else()
      message(WARNING "Dependency file not found: ${DEPENDENCY_FILE}")
    endif()
  endforeach()

endfunction()
