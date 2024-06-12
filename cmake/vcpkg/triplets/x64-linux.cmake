set(VCPKG_TARGET_ARCHITECTURE x64)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE static)

set(VCPKG_CMAKE_SYSTEM_NAME Linux)

if(PORT MATCHES "7zip")
  message(STATUS "Set to link 7zip dynamically")
  set(VCPKG_LIBRARY_LINKAGE dynamic)
endif()
