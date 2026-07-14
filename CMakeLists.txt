cmake_minimum_required(VERSION 3.10.0 FATAL_ERROR)
project(daquiri
    LANGUAGES CXX C
    VERSION 0.0.1
    )

set(EXTRA_MODULES_DIR ${CMAKE_CURRENT_LIST_DIR}/cmake)
list(APPEND CMAKE_MODULE_PATH ${EXTRA_MODULES_DIR})
list(APPEND CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR})

#=============================================================================
# ensure build type first
#=============================================================================

include(EnsureBuildType)

#=============================================================================
# Conan - dependencies must be installed before running CMake
#=============================================================================

if(EXISTS "${CMAKE_BINARY_DIR}/conanbuildinfo.cmake")
  include("${CMAKE_BINARY_DIR}/conanbuildinfo.cmake")
  conan_basic_setup(NO_OUTPUT_DIRS KEEP_RPATHS)
  message(STATUS "Deleting Conan-generated Findqplot.cmake to use package-provided modules")
  file(REMOVE ${CMAKE_BINARY_DIR}/Findqplot.cmake)
else()
  message(FATAL_ERROR "conanbuildinfo.cmake not found in build directory. Please run 'conan install' before cmake. See README.md for build instructions.")
endif()

include(CompilerConfig)
include(OutputDirConfig)
include(CoverageReports)

add_subdirectory(src)

#include(FindGTestFix)
#if(GTest_FOUND)
#  message(STATUS "GTest found, adding tests subdirectory")
#  add_custom_target(everything DEPENDS
#    daqlite
#    )
#endif()
add_subdirectory(tests)

#=============================================================================
# cppcheck target — mirrors the GitLab CI invocation
#=============================================================================

find_program(CPPCHECK cppcheck)
if(CPPCHECK)
  add_custom_target(cppcheck
    # Collect C/C++ source files from src, excluding:
    #   - test files from test dirs and Test files)
    #   - fylgje
    COMMAND find src/ -type f
      \( -name "*.cpp" -o -name "*.c" -o -name "*.h" \)
      ! -path "*/test/*" ! -name "*Test*"
      ! -path "*/fylgje/*"
      -print > cppcheck_files.txt
    COMMAND ${CPPCHECK} -v
      --platform=unix64
      --force
      --xml-version=2
      --enable=all
      --inconclusive
      --std=c++17
      --language=c++
      --library=qt
      --suppressions-list=cppcheck-suppressions.txt
      "--template={file},{line},{severity},{id},{message}"
      --file-list=cppcheck_files.txt
      --output-file=cppcheck.xml
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Static analysis with cppcheck"
    VERBATIM
  )

  add_custom_target(cppcheck_html
    COMMAND cppcheck-htmlreport --file cppcheck.xml --report-dir cppcheck_html
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Generating cppcheck HTML report in cppcheck_html/"
  )
else()
  message(STATUS "cppcheck not found — 'make cppcheck' target unavailable")
endif()
