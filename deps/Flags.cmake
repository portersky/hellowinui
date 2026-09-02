# ==============================================================================
# Compiler and Linker Flags
# ==============================================================================
# This module sets platform-specific and compiler-specific flags, libraries,
# and definitions used throughout the build system.
#
# Variables set:
#   BASE_LIBRARIES   - Libraries to link based on target platform
#   BASE_DEFINITIONS - Preprocessor definitions based on target platform
#
# This module also configures global compile and link options for:
#   - Emscripten (WebAssembly) builds
#   - Clang/GCC warning flags
#   - MSVC warning flags and settings
#
# Requires: Platform.cmake must be included first (for IS_* variables)
# ==============================================================================

# ------------------------------------------------------------------------------
# Platform-Specific Link Libraries
# ------------------------------------------------------------------------------
# These libraries are required for OpenGL and windowing on each platform.

# detect_windows_sdk(<out_var>)
#
# Detects the latest installed Windows 10 SDK and sets out_var to its um/x64 lib path.
#
# Parameters:
#   - OUT_VAR  Name of the variable to set with the SDK lib path
function(detect_windows_sdk OUT_VAR)
    set(_ROOT "C:/Program Files (x86)/Windows Kits/10/Lib")
    file(GLOB _VERSIONS RELATIVE "${_ROOT}" "${_ROOT}/10.*")
    list(SORT _VERSIONS ORDER DESCENDING)
    list(GET _VERSIONS 0 _VERSION)
    message(STATUS "Windows SDK: ${_VERSION}")
    set(${OUT_VAR} "${_ROOT}/${_VERSION}/um/x64" PARENT_SCOPE)
endfunction()

# find_and_link(<OUT_VAR> <LIB_NAME> <DISPLAY_NAME>)
#
# Searches for a library and, if found, appends it to the output list.
#
# Parameters:
#   - OUT_VAR      Name of the list variable to append the library to
#   - LIB_NAME     The library filename to search for (e.g. "d3d12.lib")
#   - DISPLAY_NAME Human-readable name for status messages (e.g. "Direct3D 12")
function(find_and_link OUT_VAR LIB_NAME DISPLAY_NAME)
    find_library(_FOUND_LIB ${LIB_NAME} HINTS "${_WINSDK_LIB_PATH}")
    if (_FOUND_LIB)
        message(STATUS "  [x] ${DISPLAY_NAME} (${LIB_NAME})")
        list(APPEND ${OUT_VAR} "${LIB_NAME}")
        set(${OUT_VAR} "${${OUT_VAR}}" PARENT_SCOPE)
    else()
        message(STATUS "  [ ] ${DISPLAY_NAME} (${LIB_NAME})")
    endif()
    unset(_FOUND_LIB CACHE)
endfunction()

# find_and_link_framework(<OUT_VAR> <FRAMEWORK_NAME> <DISPLAY_NAME>)
#
# Searches for an Apple framework and, if found, appends -framework <name> to the output list.
#
# Parameters:
#   - OUT_VAR         Name of the list variable to append the flag to
#   - FRAMEWORK_NAME  The framework name to search for (e.g. "Cocoa")
#   - DISPLAY_NAME    Human-readable name for status messages
function(find_and_link_framework OUT_VAR FRAMEWORK_NAME DISPLAY_NAME)
    find_library(_FOUND_LIB ${FRAMEWORK_NAME})
    if (_FOUND_LIB)
        message(STATUS "  [x] ${DISPLAY_NAME} (-framework ${FRAMEWORK_NAME})")
        list(APPEND ${OUT_VAR} "-framework ${FRAMEWORK_NAME}")
        set(${OUT_VAR} "${${OUT_VAR}}" PARENT_SCOPE)
    else()
        message(STATUS "  [ ] ${DISPLAY_NAME} (-framework ${FRAMEWORK_NAME})")
    endif()
    unset(_FOUND_LIB CACHE)
endfunction()

set(BASE_LIBRARIES "")
if (IS_MACOS)
    # TODO: Support macOS
    message(FATAL_ERROR "macOS not supported!")
    message(STATUS "macOS link libraries:")
    find_and_link_framework(BASE_LIBRARIES "Cocoa"     "Cocoa")
    find_and_link_framework(BASE_LIBRARIES "IOKit"     "IOKit")
    find_and_link_framework(BASE_LIBRARIES "CoreVideo" "Core Video")
    find_and_link_framework(BASE_LIBRARIES "OpenGL"    "OpenGL")
elseif (IS_LINUX)  # Linux, BSD, Solaris, Minix
    message(STATUS "Linux link libraries:")
    find_and_link(BASE_LIBRARIES "dl"  "Dynamic Linking")
    find_and_link(BASE_LIBRARIES "m"   "Math")
    find_and_link(BASE_LIBRARIES "GL"  "OpenGL")
    find_and_link(BASE_LIBRARIES "X11" "X11")
elseif (IS_WINDOWS)
    detect_windows_sdk(_WINSDK_LIB_PATH)
    message(STATUS "Windows link libraries:")
    find_and_link(BASE_LIBRARIES "OpenGL32.lib" "OpenGL")
    find_and_link(BASE_LIBRARIES "winmm.lib"    "WinMM")
    find_and_link(BASE_LIBRARIES "dwmapi.lib"   "DWM API")
elseif(IS_EMSCRIPTEN)
    include(Emscripten)
endif()

# ------------------------------------------------------------------------------
# Platform-Specific Definitions
# ------------------------------------------------------------------------------
set(BASE_DEFINITIONS
    "ASIO_STANDALONE"
)
if (IS_WINDOWS)
    set(BASE_DEFINITIONS
        ${BASE_DEFINITIONS}
        # See _WIN32_WINNT version constants
        # https://learn.microsoft.com/en-us/cpp/porting/modifying-winver-and-win32-winnt
        "_WIN32_WINNT=0x0A00"
    )
endif()

# ------------------------------------------------------------------------------
# Compiler Warning Flags
# ------------------------------------------------------------------------------
# These are stored in BASE_OPTIONS and applied per-target via
# target_compile_options() to avoid polluting external dependencies.
set(BASE_OPTIONS "")
if (IS_CLANG_OR_GCC)
    set(BASE_OPTIONS
        "-Wall"                          # Enable all common warnings
        "-Wextra"                        # Enable extra warnings
        "-Werror"                        # Treat warnings as errors
    )
elseif (IS_MSVC)
    set(BASE_OPTIONS
        "/W4"              # Warning level 4 (high)
        "/WX"              # Treat warnings as errors
        "/utf-8"           # Set source and execution character set to UTF-8
        "/Zc:__cplusplus"  # Report correct C++ standard version in __cplusplus
    )
endif()
