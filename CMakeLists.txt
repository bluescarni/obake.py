# NOTE: C++20 supported since CMake 3.12.0:
# https://cmake.org/cmake/help/v3.12/prop_tgt/CXX_STANDARD.html
cmake_minimum_required(VERSION 3.12.0)

# Set default build type to "Release".
# NOTE: this should be done before the project command since the latter can set
# CMAKE_BUILD_TYPE itself (it does so for nmake).
if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Release CACHE STRING
		"Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
	FORCE)
endif()

project(obake.py VERSION 0.1 LANGUAGES CXX C)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" "${CMAKE_CURRENT_SOURCE_DIR}/cmake/yacma")

message(STATUS "System name: ${CMAKE_SYSTEM_NAME}")
message(STATUS "obake.py version: ${obake.py_VERSION}")

option(OBAKE_PY_ENABLE_IPO "Enable IPO (requires compiler support)." OFF)
mark_as_advanced(OBAKE_PY_ENABLE_IPO)

# Run the YACMA compiler setup.
include(YACMACompilerLinkerSettings)

# Assemble the flags.
set(OBAKE_PY_CXX_FLAGS_DEBUG ${YACMA_CXX_FLAGS} ${YACMA_CXX_FLAGS_DEBUG})
set(OBAKE_PY_CXX_FLAGS_RELEASE ${YACMA_CXX_FLAGS})

if(YACMA_COMPILER_IS_MSVC)
    include(CheckCXXCompilerFlag)
    # Disable the idiotic minmax macros on MSVC (both cl and clang-cl).
    # Also, enable the bigobj flag and the WIN32_LEAN_AND_MEAN definitions:
    # https://stackoverflow.com/questions/11040133/what-does-defining-win32-lean-and-mean-exclude-exactly
    list(APPEND OBAKE_PY_CXX_FLAGS_DEBUG "-DNOMINMAX" "/bigobj" "-DWIN32_LEAN_AND_MEAN")
    list(APPEND OBAKE_PY_CXX_FLAGS_RELEASE "-DNOMINMAX" "/bigobj" "-DWIN32_LEAN_AND_MEAN")
    # Enable strict conformance mode, if supported.
    set(CMAKE_REQUIRED_QUIET TRUE)
    check_cxx_compiler_flag("/permissive-" _OBAKE_PY_MSVC_SUPPORTS_STRICT_CONFORMANCE)
    unset(CMAKE_REQUIRED_QUIET)
    if(_OBAKE_PY_MSVC_SUPPORTS_STRICT_CONFORMANCE)
        message(STATUS "The '/permissive-' flag is supported, enabling it.")
        list(APPEND OBAKE_PY_CXX_FLAGS_DEBUG "/permissive-")
        list(APPEND OBAKE_PY_CXX_FLAGS_RELEASE "/permissive-")
    endif()
    unset(_OBAKE_PY_MSVC_SUPPORTS_STRICT_CONFORMANCE)
    if(YACMA_COMPILER_IS_CLANGXX)
        # clang-cl emits various warnings from GMP/MPFR, let's just silence them.
        # NOTE: at one point in the recent past, MSVC added an options similar to GCC's isystem:
        # https://blogs.msdn.microsoft.com/vcblog/2017/12/13/broken-warnings-theory/
        # We probably just need to wait for this to be picked up by CMake/clang-cl. Let's
        # revisit the issue in the future.
        list(APPEND _OBAKE_PY_CLANG_CL_DISABLED_WARNINGS
            "-Wno-unused-variable"
            "-Wno-inconsistent-dllimport"
            "-Wno-unknown-pragmas"
            "-Wno-unused-parameter"
            "-Wno-sign-compare"
            "-Wno-deprecated-declarations"
            "-Wno-deprecated-dynamic-exception-spec"
            "-Wno-old-style-cast"
            "-Wno-sign-conversion"
            "-Wno-non-virtual-dtor"
            "-Wno-deprecated"
            "-Wno-shadow"
            "-Wno-shorten-64-to-32"
            "-Wno-reserved-id-macro"
            "-Wno-undef"
            "-Wno-c++98-compat-pedantic"
            "-Wno-documentation-unknown-command"
            "-Wno-zero-as-null-pointer-constant"
            "-Wno-language-extension-token"
            "-Wno-gnu-anonymous-struct"
            "-Wno-nested-anon-types"
            "-Wno-documentation"
            "-Wno-comma"
            "-Wno-nonportable-system-include-path"
            "-Wno-global-constructors"
            "-Wno-redundant-parens"
            "-Wno-exit-time-destructors"
            "-Wno-missing-noreturn"
            "-Wno-switch-enum"
            "-Wno-covered-switch-default"
            "-Wno-float-equal"
            "-Wno-double-promotion"
            "-Wno-microsoft-enum-value"
            "-Wno-missing-prototypes"
            "-Wno-implicit-fallthrough"
            "-Wno-format-nonliteral"
            "-Wno-cast-qual"
            "-Wno-disabled-macro-expansion"
            "-Wno-unused-private-field"
            "-Wno-unused-template"
            "-Wno-unused-macros"
            "-Wno-extra-semi-stmt"
            "-Wno-c++98-compat")
        list(APPEND OBAKE_PY_CXX_FLAGS_DEBUG ${_OBAKE_PY_CLANG_CL_DISABLED_WARNINGS})
        list(APPEND OBAKE_PY_CXX_FLAGS_RELEASE ${_OBAKE_PY_CLANG_CL_DISABLED_WARNINGS})
        unset(_OBAKE_PY_CLANG_CL_DISABLED_WARNINGS)
    else()
        # Problematic MSVC cl warnings.
        list(APPEND OBAKE_PY_CXX_FLAGS_DEBUG "/wd4459")
        list(APPEND OBAKE_PY_CXX_FLAGS_RELEASE "/wd4459")
    endif()
endif()
if(MINGW)
    # In MinGW some tests generate big object files.
    message(STATUS "Enabling the '-Wa,-mbig-obj' flag for MinGW.")
    list(APPEND OBAKE_PY_CXX_FLAGS_DEBUG "-Wa,-mbig-obj")
    list(APPEND OBAKE_PY_CXX_FLAGS_RELEASE "-Wa,-mbig-obj")
endif()

# Find the dependencies.

# obake.
# NOTE: put the minimum version in a variable
# so that we can re-use it below.
set (_OBAKE_PY_MIN_OBAKE_VERSION 0.7)
find_package(obake REQUIRED)
if(${obake_VERSION} VERSION_LESS ${_OBAKE_PY_MIN_OBAKE_VERSION})
    message(FATAL_ERROR "The minimum obake version required by obake.py is ${_OBAKE_PY_MIN_OBAKE_VERSION}, but version ${obake_VERSION} was found instead.")
endif()

# Boost (header only).
find_package(Boost 1.65.0 QUIET REQUIRED)

# python.
find_package(Python3 REQUIRED COMPONENTS Interpreter Development)
message(STATUS "Python3 interpreter: ${Python3_EXECUTABLE}")
message(STATUS "Python3 installation directory: ${Python3_SITEARCH}")
set(OBAKE_PY_INSTALL_PATH "" CACHE STRING "obake module installation path")
mark_as_advanced(OBAKE_PY_INSTALL_PATH)

# pybind11.
find_package(pybind11 REQUIRED)
if(${pybind11_VERSION} VERSION_LESS "2.6")
    message(FATAL_ERROR "pybind11 >= 2.6 is required, but version ${pybind11_VERSION} was found instead.")
endif()

# Add the module directory.
add_subdirectory(obake)
