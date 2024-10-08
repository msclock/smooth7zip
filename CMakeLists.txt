cmake_minimum_required(VERSION 3.25)
message(STATUS "CMAKE VERSION:${CMAKE_VERSION}")

# Project settings
project(
  smooth7zip
  LANGUAGES CXX C
  HOMEPAGE_URL "https://github.com/msclock/smooth7zip"
  DESCRIPTION "A wrapper for 7zip")

set(CMAKE_CXX_STANDARD
    20
    CACHE STRING "C++ standard")
set(CMAKE_CXX_STANDARD_REQUIRED
    ON
    CACHE BOOL "C++ standard required")
set(CMAKE_CXX_EXTENSIONS
    OFF
    CACHE BOOL "C++ extensions")

# Source includes from vcpkg
include_directories(${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/include)

# From https://github.com/msclock/cmake-registry.
find_package(cmake-modules REQUIRED)
include(cmake-modules/ProjectDefault)

# Project custom modules
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(ConfigureWarningsAndHardening)
include(ConfigureVersion)
include(ConfigureDocs)
include(ConfigureCoverage)

if(test IN_LIST VCPKG_MANIFEST_FEATURES)
  include(ConfigureTest)
endif()

add_subdirectory(src)
