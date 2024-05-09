find_package(GTest CONFIG REQUIRED)
find_package(vector-of-bool-cmrc CONFIG REQUIRED)

include(vector-of-bool-cmrc/CMakeRC)

file(GLOB_RECURSE test_data_files ${CMAKE_CURRENT_LIST_DIR}/test_data/*)
cmrc_add_resource_library(test_data_rc NAMESPACE test_data ${test_data_files})

include(GoogleTest)

file(GLOB files "*.cpp")

foreach(_file ${files})
  get_filename_component(file_basename ${_file} NAME_WE)
  add_executable(${file_basename} ${_file})
  target_link_libraries(
    ${file_basename} PRIVATE GTest::gtest_main smooth7zip 7zip::7zip
                             7zip::7zip_extras test_data_rc)
  warn_target(${file_basename} EXCLUDE_FLAGS -Wnon-virtual-dtor)
  harden_target(${file_basename})
  sanitize_target(${file_basename})
  target_code_coverage(${file_basename} ALL)
  gtest_discover_tests(${file_basename})
endforeach()
