find_package(spdlog CONFIG REQUIRED)
find_package(7zip CONFIG REQUIRED)
find_package(tl-expected CONFIG REQUIRED)

set(target_name smooth7zip)

add_library(${target_name} INTERFACE)

generate_git_header(VERSION_NAMESPACE_PREFIX ${target_name})

target_include_interface_directories(
  ${target_name} ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_CURRENT_BINARY_DIR}/git_version)

target_link_libraries(${target_name} INTERFACE spdlog::spdlog 7zip::7zip
                                               7zip::7zip_extras tl::expected)

install_target(
  NAME
  ${target_name}
  VERSION
  ${CMAKE_PROJECT_VERSION}
  TARGETS
  ${target_name}
  INCLUDES
  ${CMAKE_CURRENT_SOURCE_DIR}/include/${target_name}/
  ${CMAKE_CURRENT_BINARY_DIR}/git_version/
  LICENSE_FILE_LIST
  ${CMAKE_SOURCE_DIR}/LICENSE
  DEPENDENCIES
  spdlog::spdlog
  7zip::7zip
  7zip::7zip_extras
  tl::expected)

if(BUILD_TESTING)
  add_test_subdirectory(tests)
endif()
