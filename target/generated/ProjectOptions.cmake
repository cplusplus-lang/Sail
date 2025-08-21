include(cmake/SystemLink.cmake)
include(cmake/LibFuzzer.cmake)
include(CMakeDependentOption)
include(CheckCXXCompilerFlag)


include(CheckCXXSourceCompiles)


macro(Sail_supports_sanitizers)
  if((CMAKE_CXX_COMPILER_ID MATCHES ".*Clang.*" OR CMAKE_CXX_COMPILER_ID MATCHES ".*GNU.*") AND NOT WIN32)

    message(STATUS "Sanity checking UndefinedBehaviorSanitizer, it should be supported on this platform")
    set(TEST_PROGRAM "int main() { return 0; }")

    # Check if UndefinedBehaviorSanitizer works at link time
    set(CMAKE_REQUIRED_FLAGS "-fsanitize=undefined")
    set(CMAKE_REQUIRED_LINK_OPTIONS "-fsanitize=undefined")
    check_cxx_source_compiles("${TEST_PROGRAM}" HAS_UBSAN_LINK_SUPPORT)

    if(HAS_UBSAN_LINK_SUPPORT)
      message(STATUS "UndefinedBehaviorSanitizer is supported at both compile and link time.")
      set(SUPPORTS_UBSAN ON)
    else()
      message(WARNING "UndefinedBehaviorSanitizer is NOT supported at link time.")
      set(SUPPORTS_UBSAN OFF)
    endif()
  else()
    set(SUPPORTS_UBSAN OFF)
  endif()

  if((CMAKE_CXX_COMPILER_ID MATCHES ".*Clang.*" OR CMAKE_CXX_COMPILER_ID MATCHES ".*GNU.*") AND WIN32)
    set(SUPPORTS_ASAN OFF)
  else()
    if (NOT WIN32)
      message(STATUS "Sanity checking AddressSanitizer, it should be supported on this platform")
      set(TEST_PROGRAM "int main() { return 0; }")

      # Check if AddressSanitizer works at link time
      set(CMAKE_REQUIRED_FLAGS "-fsanitize=address")
      set(CMAKE_REQUIRED_LINK_OPTIONS "-fsanitize=address")
      check_cxx_source_compiles("${TEST_PROGRAM}" HAS_ASAN_LINK_SUPPORT)

      if(HAS_ASAN_LINK_SUPPORT)
        message(STATUS "AddressSanitizer is supported at both compile and link time.")
        set(SUPPORTS_ASAN ON)
      else()
        message(WARNING "AddressSanitizer is NOT supported at link time.")
        set(SUPPORTS_ASAN OFF)
      endif()
    else()
      set(SUPPORTS_ASAN ON)
    endif()
  endif()
endmacro()

macro(Sail_setup_options)
  option(Sail_ENABLE_HARDENING "Enable hardening" ON)
  option(Sail_ENABLE_COVERAGE "Enable coverage reporting" OFF)
  cmake_dependent_option(
    Sail_ENABLE_GLOBAL_HARDENING
    "Attempt to push hardening options to built dependencies"
    ON
    Sail_ENABLE_HARDENING
    OFF)

  Sail_supports_sanitizers()

  if(NOT PROJECT_IS_TOP_LEVEL OR Sail_PACKAGING_MAINTAINER_MODE)
    option(Sail_ENABLE_IPO "Enable IPO/LTO" OFF)
    option(Sail_WARNINGS_AS_ERRORS "Treat Warnings As Errors" OFF)
    option(Sail_ENABLE_USER_LINKER "Enable user-selected linker" OFF)
    option(Sail_ENABLE_SANITIZER_ADDRESS "Enable address sanitizer" OFF)
    option(Sail_ENABLE_SANITIZER_LEAK "Enable leak sanitizer" OFF)
    option(Sail_ENABLE_SANITIZER_UNDEFINED "Enable undefined sanitizer" OFF)
    option(Sail_ENABLE_SANITIZER_THREAD "Enable thread sanitizer" OFF)
    option(Sail_ENABLE_SANITIZER_MEMORY "Enable memory sanitizer" OFF)
    option(Sail_ENABLE_UNITY_BUILD "Enable unity builds" OFF)
    option(Sail_ENABLE_CLANG_TIDY "Enable clang-tidy" OFF)
    option(Sail_ENABLE_CPPCHECK "Enable cpp-check analysis" OFF)
    option(Sail_ENABLE_PCH "Enable precompiled headers" OFF)
    option(Sail_ENABLE_CACHE "Enable ccache" OFF)
  else()
    option(Sail_ENABLE_IPO "Enable IPO/LTO" ON)
    option(Sail_WARNINGS_AS_ERRORS "Treat Warnings As Errors" ON)
    option(Sail_ENABLE_USER_LINKER "Enable user-selected linker" OFF)
    option(Sail_ENABLE_SANITIZER_ADDRESS "Enable address sanitizer" ${SUPPORTS_ASAN})
    option(Sail_ENABLE_SANITIZER_LEAK "Enable leak sanitizer" OFF)
    option(Sail_ENABLE_SANITIZER_UNDEFINED "Enable undefined sanitizer" ${SUPPORTS_UBSAN})
    option(Sail_ENABLE_SANITIZER_THREAD "Enable thread sanitizer" OFF)
    option(Sail_ENABLE_SANITIZER_MEMORY "Enable memory sanitizer" OFF)
    option(Sail_ENABLE_UNITY_BUILD "Enable unity builds" OFF)
    option(Sail_ENABLE_CLANG_TIDY "Enable clang-tidy" ON)
    option(Sail_ENABLE_CPPCHECK "Enable cpp-check analysis" ON)
    option(Sail_ENABLE_PCH "Enable precompiled headers" OFF)
    option(Sail_ENABLE_CACHE "Enable ccache" ON)
  endif()

  if(NOT PROJECT_IS_TOP_LEVEL)
    mark_as_advanced(
      Sail_ENABLE_IPO
      Sail_WARNINGS_AS_ERRORS
      Sail_ENABLE_USER_LINKER
      Sail_ENABLE_SANITIZER_ADDRESS
      Sail_ENABLE_SANITIZER_LEAK
      Sail_ENABLE_SANITIZER_UNDEFINED
      Sail_ENABLE_SANITIZER_THREAD
      Sail_ENABLE_SANITIZER_MEMORY
      Sail_ENABLE_UNITY_BUILD
      Sail_ENABLE_CLANG_TIDY
      Sail_ENABLE_CPPCHECK
      Sail_ENABLE_COVERAGE
      Sail_ENABLE_PCH
      Sail_ENABLE_CACHE)
  endif()

  Sail_check_libfuzzer_support(LIBFUZZER_SUPPORTED)
  if(LIBFUZZER_SUPPORTED AND (Sail_ENABLE_SANITIZER_ADDRESS OR Sail_ENABLE_SANITIZER_THREAD OR Sail_ENABLE_SANITIZER_UNDEFINED))
    set(DEFAULT_FUZZER ON)
  else()
    set(DEFAULT_FUZZER OFF)
  endif()

  option(Sail_BUILD_FUZZ_TESTS "Enable fuzz testing executable" ${DEFAULT_FUZZER})

endmacro()

macro(Sail_global_options)
  if(Sail_ENABLE_IPO)
    include(cmake/InterproceduralOptimization.cmake)
    Sail_enable_ipo()
  endif()

  Sail_supports_sanitizers()

  if(Sail_ENABLE_HARDENING AND Sail_ENABLE_GLOBAL_HARDENING)
    include(cmake/Hardening.cmake)
    if(NOT SUPPORTS_UBSAN 
       OR Sail_ENABLE_SANITIZER_UNDEFINED
       OR Sail_ENABLE_SANITIZER_ADDRESS
       OR Sail_ENABLE_SANITIZER_THREAD
       OR Sail_ENABLE_SANITIZER_LEAK)
      set(ENABLE_UBSAN_MINIMAL_RUNTIME FALSE)
    else()
      set(ENABLE_UBSAN_MINIMAL_RUNTIME TRUE)
    endif()
    message("${Sail_ENABLE_HARDENING} ${ENABLE_UBSAN_MINIMAL_RUNTIME} ${Sail_ENABLE_SANITIZER_UNDEFINED}")
    Sail_enable_hardening(Sail_options ON ${ENABLE_UBSAN_MINIMAL_RUNTIME})
  endif()
endmacro()

macro(Sail_local_options)
  if(PROJECT_IS_TOP_LEVEL)
    include(cmake/StandardProjectSettings.cmake)
  endif()

  add_library(Sail_warnings INTERFACE)
  add_library(Sail_options INTERFACE)

  include(cmake/CompilerWarnings.cmake)
  Sail_set_project_warnings(
    Sail_warnings
    ${Sail_WARNINGS_AS_ERRORS}
    ""
    ""
    ""
    "")

  if(Sail_ENABLE_USER_LINKER)
    include(cmake/Linker.cmake)
    Sail_configure_linker(Sail_options)
  endif()

  include(cmake/Sanitizers.cmake)
  Sail_enable_sanitizers(
    Sail_options
    ${Sail_ENABLE_SANITIZER_ADDRESS}
    ${Sail_ENABLE_SANITIZER_LEAK}
    ${Sail_ENABLE_SANITIZER_UNDEFINED}
    ${Sail_ENABLE_SANITIZER_THREAD}
    ${Sail_ENABLE_SANITIZER_MEMORY})

  set_target_properties(Sail_options PROPERTIES UNITY_BUILD ${Sail_ENABLE_UNITY_BUILD})

  if(Sail_ENABLE_PCH)
    target_precompile_headers(
      Sail_options
      INTERFACE
      <vector>
      <string>
      <utility>)
  endif()

  if(Sail_ENABLE_CACHE)
    include(cmake/Cache.cmake)
    Sail_enable_cache()
  endif()

  include(cmake/StaticAnalyzers.cmake)
  if(Sail_ENABLE_CLANG_TIDY)
    Sail_enable_clang_tidy(Sail_options ${Sail_WARNINGS_AS_ERRORS})
  endif()

  if(Sail_ENABLE_CPPCHECK)
    Sail_enable_cppcheck(${Sail_WARNINGS_AS_ERRORS} "" # override cppcheck options
    )
  endif()

  if(Sail_ENABLE_COVERAGE)
    include(cmake/Tests.cmake)
    Sail_enable_coverage(Sail_options)
  endif()

  if(Sail_WARNINGS_AS_ERRORS)
    check_cxx_compiler_flag("-Wl,--fatal-warnings" LINKER_FATAL_WARNINGS)
    if(LINKER_FATAL_WARNINGS)
      # This is not working consistently, so disabling for now
      # target_link_options(Sail_options INTERFACE -Wl,--fatal-warnings)
    endif()
  endif()

  if(Sail_ENABLE_HARDENING AND NOT Sail_ENABLE_GLOBAL_HARDENING)
    include(cmake/Hardening.cmake)
    if(NOT SUPPORTS_UBSAN 
       OR Sail_ENABLE_SANITIZER_UNDEFINED
       OR Sail_ENABLE_SANITIZER_ADDRESS
       OR Sail_ENABLE_SANITIZER_THREAD
       OR Sail_ENABLE_SANITIZER_LEAK)
      set(ENABLE_UBSAN_MINIMAL_RUNTIME FALSE)
    else()
      set(ENABLE_UBSAN_MINIMAL_RUNTIME TRUE)
    endif()
    Sail_enable_hardening(Sail_options OFF ${ENABLE_UBSAN_MINIMAL_RUNTIME})
  endif()

endmacro()
