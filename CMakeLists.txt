cmake_minimum_required(VERSION 3.0.0 FATAL_ERROR)
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

include(FindGTestFix)
if(GTest_FOUND)
  #add_subdirectory(tests)
  add_custom_target(everything DEPENDS
    daqlite
    )
endif()
