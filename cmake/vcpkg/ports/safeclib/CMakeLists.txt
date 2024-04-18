cmake_minimum_required(VERSION 3.19)

file(READ ${CMAKE_CURRENT_SOURCE_DIR}/configure.ac ac_init_configure)
#[[
AC_INIT([Safe C Library],
        [m4_esyscmd(build-aux/version-gen.sh .tarball-version)],
        [https://github.com/rurban/safeclib/issues],
        [safeclib],
        [http://github.com/rurban/safeclib/])
]]
set(ac_ini_match_pattern
    "AC_INIT\\(\\[([^\]]+)\\],\n[ ]+\\[([^\]]+)\\],\n[ ]+\\[([^\]]+)\\],\n[ ]+\\[([^\]]+)\\],\n[ ]+\\[([^\]]+)\\]"
)
string(REGEX MATCH "${ac_ini_match_pattern}" ac_init_info ${ac_init_configure})
set(PACKAGE_NAME ${CMAKE_MATCH_1})
set(PACKAGE_BUGREPORT ${CMAKE_MATCH_3})
set(PACKAGE_TARNAME ${CMAKE_MATCH_4})
set(PACKAGE_URL ${CMAKE_MATCH_5})

# AC_SUBST([SAFEC_API_VERSION], [3.8.0])
set(ac_version_match_pattern
    "AC_SUBST\\(\\[SAFEC_API_VERSION\\], \\[([^\]]+)\\]")
string(REGEX MATCH "${ac_version_match_pattern}" ac_version
             ${ac_init_configure})
set(PACKAGE_VERSION "${CMAKE_MATCH_1}")
set(PACKAGE_STRING "${PACKAGE_NAME} ${PACKAGE_VERSION}")

project(
  ${PACKAGE_TARNAME}
  VERSION ${PACKAGE_VERSION}
  LANGUAGES C)

if(VCPKG_TARGET_IS_WINDOWS)
  enable_language(ASM_MASM)
elseif(VCPKG_TARGET_IS_LINUX)
  enable_language(ASM)
endif()

include(CheckIncludeFile)
include(CheckSymbolExists)
include(CheckTypeSize)
include(CheckFunctionExists)
include(CheckCCompilerFlag)
include(CheckCSourceCompiles)

function(check_symbol_exists_and_add_definition)
  cmake_parse_arguments(PARSE_ARGV 0 "arg" "" "" "SYMBOLS;HEADERS")
  foreach(_sym ${arg_SYMBOLS})
    string(TOUPPER ${_sym} _SYM)
    check_symbol_exists(${_sym} "${arg_HEADERS}" HAVE_${_sym})
    if(HAVE_${_sym})
      set(HAVE_${_SYM}
          1
          PARENT_SCOPE)
    endif()
  endforeach()
endfunction()

function(check_include_file_and_add_definition)
  cmake_parse_arguments(PARSE_ARGV 0 "arg" "" "" "HEADERS")
  foreach(_header ${arg_HEADERS})
    string(REPLACE "/" "_" _header_suffix ${_header})
    string(REPLACE "." "_" _header_suffix ${_header_suffix})
    string(TOUPPER ${_header_suffix} _header_suffix)
    check_include_file(${_header} HAVE_${_header_suffix})
    if(HAVE_${_header_suffix})
      set(HAVE_${_header_suffix}
          1
          PARENT_SCOPE)
    endif()
  endforeach()
endfunction()

check_include_file_and_add_definition(
  HEADERS
  stdbool.h
  time.h
  sys/time.h
  stdlib.h
  memory.h
  ctype.h
  malloc.h
  string.h
  limits.h
  stddef.h
  unistd.h
  sys/types.h
  inttypes.h
  stdint.h
  errno.h
  wchar.h
  langinfo.h
  valgrind/valgrind.h
  intrin.h
  xmmintrin.h
  emmintrin.h
  x86intrin.h
  arm_neon.h
  arm_acle.h
  mmintrin.h
  altivec.h
  spe.h
  mbarrier.h
  stdio.h
  dlfcn.h
  libgen.h
  float.h
  wctype.h
  alloca.h
  getopt.h
  dirent.h
  strings.h
  endian.h
  sys/endian.h
  byteorder.h
  sys/byteorder.h
  byteswap.h
  iconv.h
  pcre2.h
  sys/stat.h
  direct.h
  sys/param.h
  libps/pslib.h
  mimalloc-override.h)

check_symbol_exists_and_add_definition(
  SYMBOLS
  floor
  gettimeofday
  memchr
  memmem
  memmove
  realloc
  scandir
  setenv
  strcasecmp
  strchr
  strnlen
  strrchr
  strtol
  strtoul
  strtoull
  wcscmp
  wcscpy
  wcslen
  wcsnlen
  mbstate_t
  errno_t
  HEADERS
  math.h
  sys/time.h
  string.h
  string.h
  string.h
  stdlib.h
  dirent.h
  stdlib.h
  strings.h
  string.h
  string.h
  string.h
  stdlib.h
  stdlib.h
  stdlib.h
  wchar.h
  wchar.h
  wchar.h
  wchar.h
  wchar.h
  errno.h)

check_function_exists(wcsrtombs HAVE_WCSRTOMBS)
check_function_exists(wmemchr HAVE_WMEMCHR)
check_function_exists(wmemcmp HAVE_WMEMCMP)
check_function_exists(wcscmp HAVE_WCSCMP)
check_function_exists(wcsstr HAVE_WCSSTR)
check_function_exists(vsnwprintf_s HAVE_VSNWPRINTF_S)

check_function_exists(mbstowcs HAVE_MBSTOWCS)
check_function_exists(iswdigit HAVE_ISWDIGIT)
check_function_exists(iswspace HAVE_ISWSPACE)
check_function_exists(towlower HAVE_TOWLOWER)
check_function_exists(towupper HAVE_TOWUPPER)
check_function_exists(towctrans HAVE_TOWCTRANS)
check_function_exists(__swprintf_chk HAVE___SWPRINTF_CHK)
check_function_exists(__vswscanf_chk HAVE___VSWSCANF_CHK)
check_function_exists(__memcpy_chk HAVE___MEMCPY_CHK)
check_function_exists(__memmove_chk HAVE___MEMMOVE_CHK)
check_function_exists(__memset_chk HAVE___MEMSET_CHK)
check_function_exists(__strcpy_chk HAVE___STRCPY_CHK)
check_function_exists(__strncpy_chk HAVE___STRNCPY_CHK)
check_function_exists(__strcat_chk HAVE___STRCAT_CHK)
check_function_exists(__strncat_chk HAVE___STRNCAT_CHK)
check_function_exists(__printf_chk HAVE___PRINTF_CHK)
check_function_exists(__sprintf_chk HAVE___SPRINTF_CHK)
check_function_exists(__snprintf_chk HAVE___SNPRINTF_CHK)
check_function_exists(__vfprintf_chk HAVE___VFPRINTF_CHK)
check_function_exists(__vfwprintf_chk HAVE___VFWPRINTF_CHK)
check_function_exists(__vsprintf_chk HAVE___VSPRINTF_CHK)
check_function_exists(__vsnprintf_chk HAVE___VSNPRINTF_CHK)
check_function_exists(__vsscanf_chk HAVE___VSSCANF_CHK)
check_type_size(size_t SIZEOF_SIZE_T)
check_type_size(time_t SIZEOF_TIME_T)
check_type_size(int32_t INT32_T)
check_type_size(uint8_t UINT8_T)
check_type_size(uinit16_t UINT16_T)
check_type_size(uint32_t UINT32_T)
check_type_size(uint32_t UINT32_T)
check_type_size(uint64_t UINT64_T)
check_type_size(uintptr_t UINTPTR_T)
check_type_size(wchar_t SIZEOF_WCHAR_T)
check_type_size(_Bool HAVE__BOOL)

check_c_compiler_flag(-Wrestrict HAVE_WARNING_RESTRICT)

set(HAVE_FUNC_ATTRIBUTE_RETURNS_NONNULL 1)
set(HAVE_FUNC_ATTRIBUTE_MALLOC 1)
set(HAVE_FUNC_ATTRIBUTE_FORMAT 1)
set(HAVE_C11 1)

if(MSVC)
  set(restrict __restrict)
else()
  set(restrict __restrict__)
endif()

set(ENABLE_NORM_COMPAT
    OFF
    CACHE BOOL "Enable NFKC and NFKD modes for wcsnorm.")

set(ENABLE_WARN_DMAX
    OFF
    CACHE BOOL "Enable dmax checks against __builtin_object_size(dest)")

set(ENABLE_ERROR_DMAX
    OFF
    CACHE BOOL "Make --enable-warn-dmax fatal")

set(DEFAULT_HANDLER
    ""
    CACHE STRING
          "Enforce compile-time default-handler, available: ignore, abort.")
if(DEFAULT_HANDLER STREQUAL "")
  set(INSERT_DEFAULT_HANDLER "#undef SAFECLIB_DEFAULT_HANDLER")
else()
  set(INSERT_DEFAULT_HANDLER
      "#define SAFECLIB_DEFAULT_HANDLER ${DEFAULT_HANDLER}_handler_s")
endif()

set(ENABLE_HARDENING
    ON
    CACHE BOOL "Enable hardening.")

if(CMAKE_C_COMPILER_ID STREQUAL "GNU" OR CMAKE_C_COMPILER_ID MATCHES ".*Clang")
  set(HAVE___BUILTIN_OBJECT_SIZE 1)
endif()
set(INSERT_OBJECT_SIZE "#undef HAVE___BUILTIN_OBJECT_SIZE")
set(INSERT_WARN_DMAX "#undef HAVE_WARN_DMAX")
set(INSERT_ERROR_DMAX "#undef HAVE_ERROR_DMAX")
if(HAVE___BUILTIN_OBJECT_SIZE)
  set(INSERT_OBJECT_SIZE "# define HAVE___BUILTIN_OBJECT_SIZE 1")
  if(ENABLE_WARN_DMAX)
    set(INSERT_WARN_DMAX "# define HAVE_WARN_DMAX 1")
  endif()

  if(ENABLE_ERROR_DMAX)
    set(INSERT_WARN_DMAX "#define HAVE_WARN_DMAX 1")
    set(INSERT_ERROR_DMAX "#define HAVE_ERROR_DMAX 1")
  endif()
else()
  message(STATUS "warn-dmax ignored, no __builtin_object_size")
endif()

# Check asam inline assembly support
function(check_asm_inline RESULT_VAR)
  set(asm_keywords __asm__ __asm asm)
  foreach(asm_inline_keyword IN LISTS asm_keywords)
    message(CHECK_START
            "Checking for inline assembly style: ${asm_inline_keyword}")
    set(check_c_source)
    check_c_source_compiles(
      "
#include <stdlib.h>
static void
foo(void) {
  ${asm_inline_keyword} (\"\");
  exit(0);
}
int main(void) {
  foo();
  return 0;
}"
      HAVE_INLINE_ASM_${asm_inline_keyword})

    if(HAVE_INLINE_ASM_${asm_inline_keyword})
      set(asm_inline_keyword_support "${asm_inline_keyword}")
      message(CHECK_PASS "yes")
      break()
    else()
      message(CHECK_FAIL "no")
    endif()
  endforeach()

  if(asm_inline_keyword_support)
    set(${RESULT_VAR}
        "${asm_inline_keyword_support}"
        PARENT_SCOPE)
  else()
    set(${RESULT_VAR}
        "none"
        PARENT_SCOPE)
  endif()
endfunction()

check_asm_inline(ASM_INLINE)

message(STATUS "Inline assembly style: ${ASM_INLINE}")

if(HAVE_SYS_TYPES_H)
  set(INSERT_SYS_TYPES_H "#include <sys/types.h>")
endif()

if(HAVE_INTTYPES_H)
  set(INSERT_INTTYPES_H "#include <inttypes.h>")
endif()

if(HAVE_STDINT_H)
  set(INSERT_STDINT_H "#include <stdint.h>")
endif()

check_include_file("errno.h" HAVE_ERRON_H)
if(HAVE_ERRON_H)
  set(INSERT_ERRNO_H "#include <errno.h>")
endif()

set(INSERT_BOOL_SUPPORT "#include <stdbool.h>")
check_include_file(stdbool.h HAVE_STDBOOL_H)
if(NOT HAVE_STDBOOL_H)
  set(INSERT_BOOL_SUPPORT
      "
# define bool _Bool
# define false 0
# define true 1
# define __bool_true_false_are_defined 1")
endif()

if(NOT HAVE__BOOL)
  set(INSERT_BOOL_SUPPORT
      "
#ifdef __cplusplus
typedef bool _Bool;
#else
# define _Bool unsigned char
#endif
${INSERT_BOOL_SUPPORT}")
endif()

if(NOT HAVE_ERRNO_T)
  set(FALLBACK_ERRNO_T "typedef int errno_t;")
endif()

if(CMAKE_C_COMPILER_ID STREQUAL "GNU" OR CMAKE_C_COMPILER_ID MATCHES ".*Clang")
  set(HAVE___BUILTIN_CTZ 1)
  set(HAVE___BUILTIN_CONSTANT_P 1)
endif()

set(INSERT_CONSTANT_P "#undef HAVE___BUILTIN_CONSTANT_P")
if(HAVE___BUILTIN_CONSTANT_P)
  set(INSERT_CONSTANT_P "#define HAVE___BUILTIN_CONSTANT_P 1")
endif()

# arm intrinsic barriers
check_function_exists(__dsb HAVE___DSB)
check_function_exists(__isb HAVE___ISB)
check_function_exists(__builtin___dsb HAVE___BUILTIN___DSB)
check_function_exists(__builtin___isb HAVE___BUILTIN___ISB)

set(HAVE_ATTRIBUTE_FORMAT_WPRINTF 0)
set(HAVE_ATTRIBUTE_FORMAT_WSCANF 0)

# maximum buffer length. default: 256UL << 20 (256MB)
set(RSIZE_MAX_MEM "(256UL << 20)")
# maximum string length. default: 4UL << 10 (4KB)
set(RSIZE_MAX_STR "(4U << 10)")

set(NULLSLACK
    ON
    CACHE
      BOOL
      "Null out the remaining part of a string buffer if it is not completely used."
)
if(NULLSLACK)
  set(INSERT_NULLSLACK "#define SAFECLIB_STR_NULL_SLACK 1")
else()
  set(INSERT_NULLSLACK "#undef SAFECLIB_STR_NULL_SLACK")
endif()

set(CONSTRAINT_HANDLER
    OFF
    CACHE
      BOOL
      "Enable C11 invoke_safe_{str,mem}_constraint_handler, but with performance, smaller size and less flexibility."
)

if(CONSTRAINT_HANDLER)
  set(INSERT_CONSTRAINT_HANDLER "#define SAFECLIB_DISABLE_CONSTRAINT_HANDLER 1")
else()
  set(INSERT_CONSTRAINT_HANDLER "#undef SAFECLIB_DISABLE_CONSTRAINT_HANDLER")
endif()

set(UNSAFE
    OFF
    CACHE BOOL "Include unsafe std C11 functions: tmpnam_s")

if(UNSAFE)
  set(INSERT_UNSAFE "#define SAFECLIB_ENABLE_UNSAFE 1")
else()
  set(INSERT_UNSAFE "#undef SAFECLIB_ENABLE_UNSAFE")
endif()

set(EXTENSIONS
    OFF
    CACHE
      BOOL
      "This library contains additional functions not defined in the C11 specification. Disable to omit these."
)

if(EXTENSIONS)
  set(INSERT_EXTS "#define SAFECLIB_DISABLE_EXTENSIONS 1")
else()
  set(INSERT_EXTS "#undef SAFECLIB_DISABLE_EXTENSIONS")
endif()

check_function_exists(memset HAVE_MEMSET)
check_function_exists(strcmp HAVE_STRCMP)
check_function_exists(strcasecmp HAVE_STRCASECMP)
check_function_exists(strcasestr HAVE_STRCASESTR)
check_function_exists(strcspn HAVE_STRCSPN)
check_function_exists(strpbrk HAVE_STRPBRK)
check_function_exists(strspn HAVE_STRSPN)
check_function_exists(strnstr HAVE_STRNSTR)
check_function_exists(strnlen HAVE_STRNLEN)
check_function_exists(strrchr HAVE_STRRCHR)
check_function_exists(memrchr HAVE_MEMRCHR)
check_function_exists(strstr HAVE_STRSTR)
check_function_exists(bcmp HAVE_BCMP)
check_function_exists(secure_getenv HAVE_SECURE_GETENV)
check_function_exists(timingsafe_memcmp HAVE_TIMINGSAFE_MEMCMP)
check_function_exists(timingsafe_bcmp HAVE_TIMINGSAFE_BCMP)
check_function_exists(explicit_bzero HAVE_EXPLICIT_BZERO)
check_function_exists(explicit_memset HAVE_EXPLICIT_MEMSET)
check_function_exists(asctime_r HAVE_ASCTIME_R)
check_function_exists(ctime_r HAVE_CTIME_R)
check_function_exists(gmtime_r HAVE_GMTIME_R)
check_function_exists(localtime_r HAVE_LOCALTIME_R)
check_function_exists(memccpy HAVE_MEMCCPY)
check_function_exists(stpcpy HAVE_STPCPY)
check_function_exists(stpncpy HAVE_STPNCPY)
check_function_exists(strerror HAVE_STRERROR)

check_function_exists(vswprintf HAVE_VSWPRINTF)
check_function_exists(vswscanf HAVE_VSWSCANF)
check_function_exists(mbsrtowcs HAVE_MBSRTOWCS)

# native safec shadowing functions (should be none)
check_function_exists(_memcpy_s_chk HAVE__MEMCPY_S_CHK)
check_function_exists(_memmove_s_chk HAVE__MEMMOVE_S_CHK)
check_function_exists(_memset_s_chk HAVE__MEMSET_S_CHK)
check_function_exists(_memcmp_s_chk HAVE__MEMCMP_S_CHK)
check_function_exists(_strcpy_s_chk HAVE__STRCPY_S_CHK)
check_function_exists(_strncpy_s_chk HAVE__STRNCPY_S_CHK)
check_function_exists(_strcat_s_chk HAVE__STRCAT_S_CHK)
check_function_exists(_strncat_s_chk HAVE__STRNCAT_S_CHK)
check_function_exists(_strnlen_s_chk HAVE__STRNLEN_S_CHK)
check_function_exists(_printf_s_chk HAVE__PRINTF_S_CHK)
check_function_exists(_sprintf_s_chk HAVE__SPRINTF_S_CHK)
check_function_exists(_snprintf_s_chk HAVE__SNPRINTF_S_CHK)
check_function_exists(_vfprintf_s_chk HAVE__VFPRINTF_S_CHK)
check_function_exists(_vsprintf_s_chk HAVE__VSPRINTF_S_CHK)
check_function_exists(_vsnprintf_s_chk HAVE__VSNPRINTF_S_CHK)

# native C11 Annex K functions
check_symbol_exists_and_add_definition(
  SYMBOLS
  memset_s
  memcpy_s
  memmove_s
  memzero_s
  memchr_s
  memrchr_s
  memccpy_s
  sprintf_s
  strcat_s
  strcpy_s
  strncat_s
  strncpy_s
  strnlen_s
  strtok_s
  strerror_s
  vsprintf_s
  tmpnam_s
  snprintf_s
  vsnprintf_s
  strspn_s
  strset_s
  strchr_s
  strrchr_s
  strstr_s
  strzero_s
  strnset_s
  stpcpy_s
  stpncpy_s
  sscanf_s
  fscanf_s
  scanf_s
  vfscanf_s
  vsscanf_s
  vscanf_s
  printf_s
  fprintf_s
  tmpfile_s
  vfprintf_s
  vprintf_s
  fopen_s
  freopen_s
  gets_s
  bsearch_s
  qsort_s
  gmtime_s
  localtime_s
  asctime_s
  ctime_s
  getenv_s
  HEADERS
  stdio.h
  stdlib.h
  string.h
  time.h)

if(SIZEOF_WCHAR_T
   AND HAVE_WCHAR_H
   AND HAVE_VSWPRINTF)
  set(ENABLE_WCHAR
      ON
      CACHE BOOL "Wchar support.")
else()
  message(WARNING "Wchar support disabled. wchar.h and vswprintf required.")
  set(ENABLE_WCHAR OFF)
endif()

if(ENABLE_WCHAR
   AND HAVE_WCHAR_H
   AND HAVE_VSWPRINTF
   AND HAVE_MBSRTOWCS
   AND HAVE_VSWSCANF)
  set(ENABLE_WCHAR ON)
else()
  set(ENABLE_WCHAR OFF)
endif()

if(ENABLE_WCHAR)
  set(INSERT_DISABLE_WCHAR "#undef SAFECLIB_DISABLE_WCHAR")
else()
  set(INSERT_DISABLE_WCHAR "#define SAFECLIB_DISABLE_WCHAR 1")
endif()

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/config.h.in"
               "${CMAKE_CURRENT_SOURCE_DIR}/config.h")

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/include/safe_config.h.in"
               "${CMAKE_CURRENT_SOURCE_DIR}/include/safe_config.h")

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/include/safe_lib_errno.h.in"
               "${CMAKE_CURRENT_SOURCE_DIR}/include/safe_lib_errno.h")

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/include/safe_types.h.in"
               "${CMAKE_CURRENT_SOURCE_DIR}/include/safe_types.h")

add_library(safeclib)

set(MEM_PRIM_FILES src/mem/mem_primitives_lib.h src/mem/mem_primitives_lib.c)

set(CORE_FILES src/safeclib_private.h src/abort_handler_s.c
               src/ignore_handler_s.c)

set(STD_MEM_FILES src/mem/safe_mem_constraint.h src/mem/safe_mem_constraint.c
                  src/mem/memset_s.c src/mem/memcpy_s.c src/mem/memmove_s.c)

set(EXT_MEM_FILES
    src/extmem/memset16_s.c
    src/extmem/memset32_s.c
    src/extmem/memcpy16_s.c
    src/extmem/memcpy32_s.c
    src/extmem/memmove16_s.c
    src/extmem/memmove32_s.c
    src/extmem/memcmp_s.c
    src/extmem/memcmp16_s.c
    src/extmem/memcmp32_s.c
    src/extmem/memzero_s.c
    src/extmem/memzero16_s.c
    src/extmem/memzero32_s.c
    src/extmem/memchr_s.c
    src/extmem/memrchr_s.c
    src/extmem/memccpy_s.c
    src/extmem/timingsafe_memcmp.c
    src/extmem/timingsafe_bcmp.c)

set(STD_STR_FILES
    src/str/safe_str_constraint.h
    src/str/safe_str_constraint.c
    src/str/sprintf_s.c
    src/str/strcat_s.c
    src/str/strcpy_s.c
    src/str/strncat_s.c
    src/str/strncpy_s.c
    src/str/strnlen_s.c
    src/str/strtok_s.c
    src/str/strerror_s.h
    src/str/strerror_s.c
    src/str/vsprintf_s.c
    src/str/snprintf_s.c
    src/str/vsnprintf_s.c)

# but they are in the C11 standard
set(STD_UNSAFE_FILES src/io/tmpnam_s.c)

set(EXT_STR_FILES
    src/extstr/stpcpy_s.c
    src/extstr/stpncpy_s.c
    src/extstr/strcasecmp_s.c
    src/extstr/strcasestr_s.c
    src/extstr/strcmp_s.c
    src/extstr/strcmpfld_s.c
    src/extstr/strcpyfld_s.c
    src/extstr/strcpyfldin_s.c
    src/extstr/strcpyfldout_s.c
    src/extstr/strcspn_s.c
    src/extstr/strfirstchar_s.c
    src/extstr/strfirstdiff_s.c
    src/extstr/strfirstsame_s.c
    src/extstr/strisalphanumeric_s.c
    src/extstr/strisascii_s.c
    src/extstr/strisdigit_s.c
    src/extstr/strishex_s.c
    src/extstr/strislowercase_s.c
    src/extstr/strismixedcase_s.c
    src/extstr/strispassword_s.c
    src/extstr/strisuppercase_s.c
    src/extstr/strlastchar_s.c
    src/extstr/strlastdiff_s.c
    src/extstr/strlastsame_s.c
    src/extstr/strljustify_s.c
    src/extstr/strnatcmp_s.c
    src/extstr/strnterminate_s.c
    src/extstr/strpbrk_s.c
    src/extstr/strprefix_s.c
    src/extstr/strremovews_s.c
    src/extstr/strspn_s.c
    src/extstr/strchr_s.c
    src/extstr/strrchr_s.c
    src/extstr/strstr_s.c
    src/extstr/strtolowercase_s.c
    src/extstr/strtouppercase_s.c
    src/extstr/strzero_s.c
    src/extstr/strset_s.c
    src/extstr/strnset_s.c
    src/extstr/strcoll_s.c)

set(STD_WCHAR_FILES
    src/wchar/mbsrtowcs_s.c
    src/wchar/mbstowcs_s.c
    src/wchar/wcsrtombs_s.c
    src/wchar/wcstombs_s.c
    src/wchar/wcrtomb_s.c
    src/wchar/wctomb_s.c
    src/wchar/wcsnlen_s.c
    src/wchar/wcscpy_s.c
    src/wchar/wcsncpy_s.c
    src/wchar/wcscat_s.c
    src/wchar/wcsncat_s.c
    src/wchar/wmemcpy_s.c
    src/wchar/wmemmove_s.c
    src/wchar/wcstok_s.c
    src/wchar/vswprintf_s.c
    src/wchar/swprintf_s.c
    src/wchar/vfwprintf_s.c
    src/wchar/vwprintf_s.c
    src/wchar/wprintf_s.c
    src/wchar/fwprintf_s.c
    src/wchar/swscanf_s.c
    src/wchar/vswscanf_s.c
    src/wchar/wscanf_s.c
    src/wchar/vfwscanf_s.c
    src/wchar/fwscanf_s.c
    src/wchar/vwscanf_s.c
    src/wchar/snwprintf_s.c)

if(NOT HAVE_VSNWPRINTF_S)
  list(APPEND STD_WCHAR_FILES src/wchar/vsnwprintf_s.c)
endif()

if(NOT HAVE_WCSSTR)
  list(APPEND STD_WCHAR_FILES src/extwchar/wcsstr.c)
endif()

if(ENABLE_EXTS)
  set(STD_WCHAR_FILES
      ${STD_WCHAR_FILES}
      src/extwchar/wcsstr_s.c
      src/extwchar/wmemcmp_s.c
      src/extwchar/wcscmp_s.c
      src/extwchar/wcsncmp_s.c
      src/extwchar/wcsicmp_s.c
      src/extwchar/wcsnatcmp_s.c
      src/extwchar/wcsset_s.c
      src/extwchar/wcsnset_s.c
      src/extwchar/wcscoll_s.c
      src/extwchar/wcslwr_s.c
      src/extwchar/wcsupr_s.c
      src/extwchar/towfc_s.c
      src/extwchar/towctrans.c
      src/extwchar/wcsfc_s.c
      src/extwchar/hangul.h
      src/extwchar/unw16ifcan.h
      src/extwchar/unw16ifcpt.h
      src/extwchar/unw16ifcmb.h
      src/extwchar/unw16ifcmp.h
      src/extwchar/unw16ifexc.h
      src/extwchar/unwifcan.h
      src/extwchar/unwifcmb.h
      src/extwchar/unwifcmp.h
      src/extwchar/unwifcpt.h
      src/extwchar/unwifexc.h
      src/extwchar/wcsnorm_s.c)
endif()

set(STD_IO_FILES
    src/io/sscanf_s.c
    src/io/fscanf_s.c
    src/io/scanf_s.c
    src/io/vfscanf_s.c
    src/io/vsscanf_s.c
    src/io/vscanf_s.c
    src/io/printf_s.c
    src/io/fprintf_s.c
    src/io/tmpfile_s.c
    src/io/vfprintf_s.c
    src/io/vprintf_s.c
    src/io/fopen_s.c
    src/io/freopen_s.c
    src/io/gets_s.c)
set(STD_MISC_FILES src/misc/bsearch_s.c src/misc/qsort_s.c)

set(STD_OS_FILES src/os/gmtime_s.c src/os/localtime_s.c src/os/asctime_s.c
                 src/os/ctime_s.c src/os/getenv_s.c)

set(ALL_SRC_FILES ${STD_MEM_FILES} ${STD_STR_FILES} ${STD_IO_FILES}
                  ${STD_OS_FILES} ${STD_MISC_FILES})

target_sources(safeclib PRIVATE ${ALL_SRC_FILES})

target_sources(safeclib PRIVATE ${MEM_PRIM_FILES})

target_sources(safeclib PRIVATE ${CORE_FILES})

if(ENABLE_UNSAFE)
  target_sources(safeclib PRIVATE ${STD_UNSAFE_FILES})
endif()

if(ENABLE_WCHAR)
  target_sources(safeclib PRIVATE ${STD_WCHAR_FILES})
endif()

if(ENABLE_EXTS)
  target_sources(safeclib PRIVATE ${EXT_MEM_FILES} ${EXT_STR_FILES})
endif()

if(ENABLE_HARDENING AND (CMAKE_C_COMPILER_ID STREQUAL "GNU"
                         OR CMAKE_C_COMPILER_ID MATCHES ".*Clang"))
  target_compile_options(
    safeclib
    PRIVATE -fstack-protector-strong
            -fstack-clash-protection
            -fcf-protection
            -fno-strict-aliasing
            -fno-strict-overflow
            -fno-delete-null-pointer-checks
            -fno-lifetime-dse)
  target_link_options(safeclib PRIVATE -fstack-protector-strong
                      -fstack-clash-protection -fcf-protection)
  target_compile_options(safeclib PRIVATE -Wall -Wextra)
  if(NOT MINGW)
    target_compile_options(safeclib PRIVATE -pedantic)
  endif()
endif()

target_include_directories(
  safeclib
  INTERFACE $<INSTALL_INTERFACE:include>
  PRIVATE "${CMAKE_CURRENT_BINARY_DIR}"
  PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include"
  PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src"
  PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")

include(GenerateExportHeader)
generate_export_header(safeclib EXPORT_FILE_NAME
                       ${CMAKE_CURRENT_BINARY_DIR}/safeclib/safeclib_export.h)

if(NOT BUILD_SHARED_LIBS)
  target_compile_definitions(safeclib INTERFACE -DSAFECLIB_STATIC_DEFINE)
endif()

install(
  TARGETS safeclib
  EXPORT safeclib-targets
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin)

# Headers makes relative includes so directory structure must be kept
file(GLOB HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")
install(FILES ${HEADERS} DESTINATION "include/safeclib")

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/safeclib/safeclib_export.h"
        DESTINATION "include/safeclib")

install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/config.h"
        DESTINATION "include/safeclib")

include(CMakePackageConfigHelpers)
configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/safeclib-config.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/safeclib-config.cmake"
  INSTALL_DESTINATION "share/safeclib")

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/safeclib-config.cmake"
        DESTINATION "share/safeclib")

install(
  EXPORT safeclib-targets
  DESTINATION share/safeclib
  NAMESPACE safeclib::)
