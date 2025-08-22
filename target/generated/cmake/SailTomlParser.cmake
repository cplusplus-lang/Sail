# Parse Sail.toml file and extract package info and dependencies
function(parse_sail_toml TOML_FILE)
    if(NOT EXISTS ${TOML_FILE})
        message(FATAL_ERROR "Sail.toml file not found: ${TOML_FILE}")
    endif()

    # Read the TOML file
    file(READ ${TOML_FILE} TOML_CONTENT)
    
    # Initialize variables
    set(IN_PACKAGE_SECTION FALSE)
    set(IN_DEPENDENCIES_SECTION FALSE)
    set(SAIL_DEPENDENCIES "" PARENT_SCOPE)
    
    # Split content into lines
    string(REPLACE "\n" ";" TOML_LINES ${TOML_CONTENT})
    
    foreach(LINE ${TOML_LINES})
        # Remove leading/trailing whitespace
        string(STRIP "${LINE}" LINE)
        
        # Skip empty lines and comments
        if("${LINE}" STREQUAL "" OR "${LINE}" MATCHES "^#")
            continue()
        endif()
        
        # Check for [package] section
        if("${LINE}" STREQUAL "[package]")
            set(IN_PACKAGE_SECTION TRUE)
            set(IN_DEPENDENCIES_SECTION FALSE)
            continue()
        endif()
        
        # Check for [dependencies] section
        if("${LINE}" STREQUAL "[dependencies]")
            set(IN_DEPENDENCIES_SECTION TRUE)
            set(IN_PACKAGE_SECTION FALSE)
            continue()
        endif()
        
        # Check for other sections (exit current section)
        if("${LINE}" MATCHES "^\\[.*\\]$" AND NOT "${LINE}" STREQUAL "[dependencies]" AND NOT "${LINE}" STREQUAL "[package]")
            set(IN_DEPENDENCIES_SECTION FALSE)
            set(IN_PACKAGE_SECTION FALSE)
            continue()
        endif()
        
        # Parse package lines if we're in the package section
        if(IN_PACKAGE_SECTION AND "${LINE}" MATCHES "^([^=]+)=")
            # Extract key name
            string(REGEX REPLACE "^([^=]+)=.*" "\\1" KEY_NAME "${LINE}")
            string(STRIP "${KEY_NAME}" KEY_NAME)
            
            # Extract value (remove quotes)
            string(REGEX MATCH "\"([^\"]+)\"" VALUE_MATCH "${LINE}")
            if(VALUE_MATCH)
                string(REGEX REPLACE "\"([^\"]+)\"" "\\1" KEY_VALUE "${VALUE_MATCH}")
            else()
                set(KEY_VALUE "")
            endif()
            
            # Set package variables
            if("${KEY_NAME}" STREQUAL "name")
                set(SAIL_PACKAGE_NAME ${KEY_VALUE} PARENT_SCOPE)
            elseif("${KEY_NAME}" STREQUAL "version")
                set(SAIL_PACKAGE_VERSION ${KEY_VALUE} PARENT_SCOPE)
            endif()
        endif()
        
        # Parse dependency lines if we're in the dependencies section
        if(IN_DEPENDENCIES_SECTION AND "${LINE}" MATCHES "^([^=]+)=")
            # Extract dependency name
            string(REGEX REPLACE "^([^=]+)=.*" "\\1" DEP_NAME "${LINE}")
            string(STRIP "${DEP_NAME}" DEP_NAME)
            
            # Check if this is a simple string dependency or object dependency
            if("${LINE}" MATCHES "=\\s*\\{")
                # Object dependency with version and components
                # Extract version from version = "x.x.x"
                string(REGEX MATCH "version\\s*=\\s*\"([^\"]+)\"" VERSION_MATCH "${LINE}")
                if(VERSION_MATCH)
                    string(REGEX REPLACE "version\\s*=\\s*\"([^\"]+)\"" "\\1" DEP_VERSION "${VERSION_MATCH}")
                else()
                    set(DEP_VERSION "")
                endif()
                
                # Extract components from components = ["comp1", "comp2", ...]
                string(REGEX MATCH "components\\s*=\\s*\\[([^\\]]+)\\]" COMPONENTS_MATCH "${LINE}")
                if(COMPONENTS_MATCH)
                    string(REGEX REPLACE "components\\s*=\\s*\\[([^\\]]+)\\]" "\\1" COMPONENTS_STR "${COMPONENTS_MATCH}")
                    # Remove quotes and split by comma
                    string(REGEX REPLACE "\"" "" COMPONENTS_STR "${COMPONENTS_STR}")
                    string(REGEX REPLACE "\\s*,\\s*" ";" DEP_COMPONENTS "${COMPONENTS_STR}")
                else()
                    set(DEP_COMPONENTS "")
                endif()
            else()
                # Simple string dependency
                string(REGEX MATCH "\"([^\"]+)\"" VERSION_MATCH "${LINE}")
                if(VERSION_MATCH)
                    string(REGEX REPLACE "\"([^\"]+)\"" "\\1" DEP_VERSION "${VERSION_MATCH}")
                else()
                    set(DEP_VERSION "")
                endif()
                set(DEP_COMPONENTS "")
            endif()
            
            # Add to dependencies list
            list(APPEND SAIL_DEPS ${DEP_NAME})
            
            # Set version and components variables for this dependency
            string(TOUPPER ${DEP_NAME} DEP_NAME_UPPER)
            set(SAIL_VERSION_${DEP_NAME_UPPER} ${DEP_VERSION} PARENT_SCOPE)
            if(DEP_COMPONENTS)
                set(SAIL_COMPONENTS_${DEP_NAME_UPPER} ${DEP_COMPONENTS} PARENT_SCOPE)
            endif()
        endif()
    endforeach()
    
    # Set the result in parent scope
    set(SAIL_DEPENDENCIES ${SAIL_DEPS} PARENT_SCOPE)
    
    # Debug output
    if(SAIL_DEPS)
        message(STATUS "Found dependencies in Sail.toml: ${SAIL_DEPS}")
    else()
        message(STATUS "No dependencies found in Sail.toml")
    endif()
endfunction()