#ifndef __VCPKG_7ZIP_EXTRAS_H_
#define __VCPKG_7ZIP_EXTRAS_H_

#ifdef _WIN32
#include <comutil.h> // bstr, ... (from PropVariant.h, ...)
#endif

#include <7zip/7zip.h>

#include <7zip/CPP/7zip/Archive/IArchive.h>
#include <7zip/CPP/Common/Common.h>
#include <7zip/CPP/Common/MyCom.h>
#include <7zip/CPP/Common/MyString.h>
#include <7zip/CPP/Common/StringConvert.h>
#include <7zip/Common/FileStreams.h>
#include <7zip/Common/StreamObjects.h>
#include <7zip/IPassword.h>

#include <7zip/CPP/Windows/FileDir.h>
#include <7zip/CPP/Windows/FileFind.h>
#include <7zip/CPP/Windows/FileName.h>
#include <7zip/CPP/Windows/PropVariant.h>
#include <7zip/CPP/Windows/PropVariantConv.h>

#endif // __VCPKG_7ZIP_EXTRAS_H_
