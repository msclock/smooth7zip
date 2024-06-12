#ifndef __VCPKG_7ZIP_7ZIP_H_
#define __VCPKG_7ZIP_7ZIP_H_

#include <7zip/7zip_export.h>
#include <7zip/C/7zTypes.h>
#include <7zip/CPP/7zip/MyVersion.h>
#include <7zip/CPP/7zip/PropID.h>
#include <7zip/CPP/Common/MyWindows.h>

#if !defined(UNICODE) || defined(_UNICODE)
#include <7zip/CPP/Windows/NtCheck.h>
#endif

#ifndef _7ZIP_STATIC_DEFINE
//
// Using 7zip as shared library requires GUIDs to be instantiated, they are
// already available with static library.
//
// More details from 7zip/CPP/Common/MyInitGuid.h:
//
// The 7zip/CPP/Common/MyInitGuid.h header file redefines the DEFINE_GUID
// macro to instantiate GUIDs (versus just declaring an EXTERN reference).
// Include this header file in the source file where the GUIDs should be
// instantiated.
//
#include <7zip/CPP/Common/MyInitGuid.h>
#endif

#include <7zip/CPP/7zip/Archive/IArchive.h>
#include <7zip/CPP/7zip/ICoder.h>
#include <7zip/CPP/7zip/IDecl.h>
#include <7zip/CPP/7zip/IPassword.h>
#include <7zip/CPP/7zip/IStream.h>

#ifdef _7ZIP_STATIC_DEFINE

//
// *** BEWARE ***
//
// Some globals are not initialized when using 7z as a static library so
// some functions will fail as format/codec seems unknown.
//
// Call 'Register' functions so proper variables will be initialized.
//
// Here are the minimal registration function calls to make for supporting
// default compression level with 7z:
//
//  ::lib7zCrcTableInit();
//  NArchive::N7z::Register();
//  NCompress::NBcj::RegisterCodecBCJ();
//  NCompress::NLzma::RegisterCodecLZMA();
//  NCompress::NLzma2::RegisterCodecLZMA2();
//
// There may/will be missing 'register' functions to call depending on what
// is done. For example maximum compression level also requires calling:
//
// NCompress::NBcj2::RegisterCodecBCJ2();
//

void* lib7zCrcTableInit();

namespace NArchive::N7z {
void* Register();
}
namespace NArchive::NApm {
void* Register();
}
namespace NArchive::NAr {
void* Register();
}
namespace NArchive::NArj {
void* Register();
}
namespace NArchive::NMub::NBe {
void* Register();
}
namespace NArchive::NBz2 {
void* Register();
}
namespace NArchive::NCab {
void* Register();
}
namespace NArchive::NCoff {
void* Register();
}
namespace NArchive::NCom {
void* Register();
}
namespace NArchive::NCpio {
void* Register();
}
namespace NArchive::NCramfs {
void* Register();
}
namespace NArchive::NDmg {
void* Register();
}
namespace NArchive::NElf {
void* Register();
}
namespace NArchive::NExt {
void* Register();
}
namespace NArchive::NFat {
void* Register();
}
namespace NArchive::NFlv {
void* Register();
}
namespace NArchive::NGpt {
void* Register();
}
namespace NArchive::NGz {
void* Register();
}
namespace NArchive::NHfs {
void* Register();
}
namespace NArchive::NChm::NHxs {
void* Register();
}
namespace NArchive::NIhex {
void* Register();
}
namespace NArchive::NIso {
void* Register();
}
namespace NArchive::NLzh {
void* Register();
}
namespace NArchive::NLzma::NLzma86Ar {
void* Register();
}
namespace NArchive::NLzma::NLzmaAr {
void* Register();
}
namespace NArchive::NMacho {
void* Register();
}
namespace NArchive::NMbr {
void* Register();
}
namespace NArchive::NMslz {
void* Register();
}
namespace NArchive::NNsis {
void* Register();
}
namespace NArchive::NPe {
void* Register();
}
namespace NArchive::NPpmd {
void* Register();
}
namespace NArchive::NQcow {
void* Register();
}
namespace NArchive::NRar5 {
void* Register();
}
namespace NArchive::NRar {
void* Register();
}
namespace NArchive::NRpm {
void* Register();
}
namespace NArchive::NSplit {
void* Register();
}
namespace NArchive::NSquashfs {
void* Register();
}
namespace NArchive::NSwf {
void* Register();
}
namespace NArchive::NSwfc {
void* Register();
}
namespace NArchive::NTar {
void* Register();
}
namespace NArchive::NTe {
void* Register();
}
namespace NArchive::NUdf {
void* Register();
}
namespace NArchive::NVdi {
void* Register();
}
namespace NArchive::NVhd {
void* Register();
}
namespace NArchive::NVmdk {
void* Register();
}
namespace NArchive::NWim {
void* Register();
}
namespace NArchive::NXar {
void* Register();
}
namespace NArchive::NXz {
void* Register();
}
namespace NArchive::NZ {
void* Register();
}
namespace NArchive::NZip {
void* Register();
}
namespace NArchive::Ntfs {
void* Register();
}
namespace NArchive::NUefi::UEFIc {
void* Register();
}
namespace NArchive::NUefi::UEFIf {
void* Register();
}
namespace NCrypto::N7z {
void* RegisterCodecSzAES();
}
namespace NCrypto {
void* RegisterCodecAES256CBC();
}
namespace NCompress::NBcj {
void* RegisterCodecBCJ();
}
namespace NCompress::NBcj2 {
void* RegisterCodecBCJ2();
}
namespace NCompress {
void* RegisterCodecCopy();
}
namespace NCompress::NByteSwap {
void* RegisterCodecsByteSwap();
}
namespace NCompress::NBranch {
void* RegisterCodecsBranch();
}
namespace NCompress::NBZip2 {
void* RegisterCodecBZip2();
}
namespace NCompress::NDelta {
void* RegisterCodecDelta();
}
namespace NCompress::NDeflate {
void* RegisterCodecDeflate();
}
namespace NCompress::NDeflate {
void* RegisterCodecDeflate64();
}
namespace NCompress::NLzma {
void* RegisterCodecLZMA();
}
namespace NCompress::NLzma2 {
void* RegisterCodecLZMA2();
}
namespace NCompress::NPpmd {
void* RegisterCodecPPMD();
}
namespace NCompress {
void* RegisterCodecsRar();
}

class Auto7zInitializer {
    Auto7zInitializer(const Auto7zInitializer&) = delete;
    Auto7zInitializer& operator=(const Auto7zInitializer&) = delete;

public:
    Auto7zInitializer() {
        ::lib7zCrcTableInit();
        NArchive::N7z::Register();
        NArchive::NApm::Register();
        NArchive::NAr::Register();
        NArchive::NArj::Register();
        NArchive::NBz2::Register();
        NArchive::NCab::Register();
        NArchive::NChm::NHxs::Register();
        NArchive::NCoff::Register();
        NArchive::NCom::Register();
        NArchive::NCpio::Register();
        NArchive::NCramfs::Register();
        NArchive::NDmg::Register();
        NArchive::NElf::Register();
        NArchive::NExt::Register();
        NArchive::NFat::Register();
        NArchive::NFlv::Register();
        NArchive::NGpt::Register();
        NArchive::NGz::Register();
        NArchive::NHfs::Register();
        NArchive::NIhex::Register();
        NArchive::NIso::Register();
        NArchive::NLzh::Register();
        NArchive::NLzma::NLzmaAr::Register();
        NArchive::NLzma::NLzma86Ar::Register();
        NArchive::NMacho::Register();
        NArchive::NMbr::Register();
        NArchive::NMslz::Register();
        NArchive::NMub::NBe::Register();
        NArchive::NNsis::Register();
        NArchive::NPe::Register();
        NArchive::NPpmd::Register();
        NArchive::NQcow::Register();
        NArchive::NRar5::Register();
        NArchive::NRar::Register();
        NArchive::NRpm::Register();
        NArchive::NSplit::Register();
        NArchive::NSquashfs::Register();
        NArchive::NSwf::Register();
        NArchive::NSwfc::Register();
        NArchive::NTar::Register();
        NArchive::NTe::Register();
        NArchive::Ntfs::Register();
        NArchive::NUdf::Register();
        NArchive::NUefi::UEFIc::Register();
        NArchive::NUefi::UEFIf::Register();
        NArchive::NVdi::Register();
        NArchive::NVhd::Register();
        NArchive::NVmdk::Register();
        NArchive::NWim::Register();
        NArchive::NXar::Register();
        NArchive::NXz::Register();
        NArchive::NZ::Register();
        NArchive::NZip::Register();
        NCompress::RegisterCodecCopy();
        NCompress::RegisterCodecsRar();
        NCompress::NBcj::RegisterCodecBCJ();
        NCompress::NBcj2::RegisterCodecBCJ2();
        NCompress::NBranch::RegisterCodecsBranch();
        NCompress::NByteSwap::RegisterCodecsByteSwap();
        NCompress::NBZip2::RegisterCodecBZip2();
        NCompress::NDeflate::RegisterCodecDeflate();
        NCompress::NDeflate::RegisterCodecDeflate64();
        NCompress::NDelta::RegisterCodecDelta();
        NCompress::NLzma::RegisterCodecLZMA();
        NCompress::NLzma2::RegisterCodecLZMA2();
        NCompress::NPpmd::RegisterCodecPPMD();
        NCrypto::RegisterCodecAES256CBC();
        NCrypto::N7z::RegisterCodecSzAES();
    }
};

inline int g_auto_7z_load = []() {
    static Auto7zInitializer lib;
    return 0;
}();

#endif _7ZIP_STATIC_DEFINE

#ifdef __cplusplus
extern "C" {
#endif

    STDAPI CreateObject(const GUID* clsid, const GUID* iid, void** outObject);

    STDAPI GetHandlerProperty2(UInt32 formatIndex, PROPID propID, PROPVARIANT* value);
    STDAPI GetNumberOfFormats(UInt32* numFormats);
    STDAPI GetHandlerProperty2(UInt32 formatIndex, PROPID propID, PROPVARIANT* value);
    STDAPI GetIsArc(UInt32 formatIndex, Func_IsArc* isArc);

    STDAPI GetNumberOfMethods(UInt32* numMethods);
    STDAPI GetMethodProperty(UInt32 codecIndex, PROPID propID, PROPVARIANT* value);
    STDAPI CreateDecoder(UInt32 index, const GUID* iid, void** outObject);
    STDAPI CreateEncoder(UInt32 index, const GUID* iid, void** outObject);

    STDAPI GetHashers(IHashers** hashers);

    STDAPI SetCodecs(ICompressCodecsInfo* compressCodecsInfo);

    STDAPI SetLargePageMode();
    STDAPI SetCaseSensitive(Int32 caseSensitive);

    STDAPI GetModuleProp(PROPID propID, PROPVARIANT* value);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // __VCPKG_7ZIP_7ZIP_H_
