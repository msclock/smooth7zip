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

# Shared modules from # https://github.com/msclock/cmake-registry.
find_package(cmake-modules REQUIRED)
include(cmake-modules/configure/Default)
include(cmake-modules/configure/Common)
include(cmake-modules/configure/CheckBuildDir)
include(cmake-modules/configure/UniqueOutputBinaryDir)
include(cmake-modules/build/Default)
include(cmake-modules/build/Ccache)
include(cmake-modules/build/LinkOptimization)
include(cmake-modules/test/Default)
include(cmake-modules/install/Default)
include(cmake-modules/install/Common)
include(cmake-modules/install/InstallDependency)
include(cmake-modules/install/Runpath)

# Add _DEBUG macro
add_debug_macro()

# Disable warnings for MSVC from now
if(WIN32)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS -DNOMINMAX)
endif()

create_uninstall_target()

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

# Print project final information
show_project_version()
show_vcpkg_configuration()
show_installation()
