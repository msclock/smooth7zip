vcpkg_from_github(
  OUT_SOURCE_PATH
  SOURCE_PATH
  REPO
  rurban/safeclib
  REF
  "v${VERSION}"
  SHA512
  b95e95f4b64c3490f9153cfc5fb4b5314a074d67648c67c34664bec507f70fdbde4acbc5b60d47adbb6054e86f5c27a270211a55211d1d092877108fdde3d231
  HEAD_REF
  master)

file(
  COPY "${CMAKE_CURRENT_LIST_DIR}/CMakeLists.txt"
       "${CMAKE_CURRENT_LIST_DIR}/safeclib-config.cmake.in"
       "${CMAKE_CURRENT_LIST_DIR}/config.h.in"
  DESTINATION "${SOURCE_PATH}")

vcpkg_cmake_configure(SOURCE_PATH "${SOURCE_PATH}")

vcpkg_cmake_install()
vcpkg_copy_pdbs()

vcpkg_cmake_config_fixup()

vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/COPYING")

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
