include_guard(GLOBAL)

if(NOT WIN32)
    set(WinUI_FOUND FALSE)
    if(WinUI_FIND_REQUIRED)
        message(FATAL_ERROR "WinUI 3 requires Windows")
    endif()
    return()
endif()

set(WINUI_WINDOWS_APP_SDK_VERSION "2.4.0" CACHE STRING
    "Windows App SDK version")
set(WINUI_CPPWINRT_VERSION "2.0.250303.1" CACHE STRING
    "C++/WinRT compiler version")

set(_winui_root "${CMAKE_BINARY_DIR}/_deps/winui")
set(_winui_packages "${_winui_root}/packages")

function(_winui_download_package package_id package_version output_variable)
    string(TOLOWER "${package_id}" package_name)
    set(package_root "${_winui_packages}/${package_name}/${package_version}")
    set(package_archive "${package_root}/${package_name}.${package_version}.nupkg")
    set(package_stamp "${package_root}/.extracted")

    if(NOT EXISTS "${package_stamp}")
        file(MAKE_DIRECTORY "${package_root}")
        if(NOT EXISTS "${package_archive}")
            file(DOWNLOAD
                "https://api.nuget.org/v3-flatcontainer/${package_name}/${package_version}/${package_name}.${package_version}.nupkg"
                "${package_archive}"
                STATUS download_status
                TLS_VERIFY ON
            )
            list(GET download_status 0 download_code)
            if(NOT download_code EQUAL 0)
                list(GET download_status 1 download_message)
                message(FATAL_ERROR
                    "Could not download ${package_id} ${package_version}: "
                    "${download_message}")
            endif()
        endif()

        file(ARCHIVE_EXTRACT
            INPUT "${package_archive}"
            DESTINATION "${package_root}"
            PATTERNS ${ARGN}
        )
        file(WRITE "${package_stamp}" "${package_id} ${package_version}\n")
    endif()

    set(${output_variable} "${package_root}" PARENT_SCOPE)
endfunction()

_winui_download_package(
    Microsoft.WindowsAppSDK
    "${WINUI_WINDOWS_APP_SDK_VERSION}"
    _winui_app_sdk
    "Microsoft.WindowsAppSDK.nuspec"
)

file(READ "${_winui_app_sdk}/Microsoft.WindowsAppSDK.nuspec" _winui_nuspec)

foreach(component IN ITEMS Foundation InteractiveExperiences WinUI Runtime)
    string(REGEX MATCH
        "id=\"Microsoft\\.WindowsAppSDK\\.${component}\"[^>]*version=\"[^\"]+\""
        _component_match
        "${_winui_nuspec}"
    )
    if(NOT _component_match)
        message(FATAL_ERROR
            "Could not find the ${component} package version in "
            "Microsoft.WindowsAppSDK ${WINUI_WINDOWS_APP_SDK_VERSION}")
    endif()

    string(REGEX MATCH "version=\"(\\[[^]]+\\]|[^\"]+)\""
        _component_version_match
        "${_component_match}")
    set(_component_version "${CMAKE_MATCH_1}")
    string(REGEX REPLACE "^\\[" "" _component_version
        "${_component_version}")
    string(REGEX REPLACE "\\]$" "" _component_version
        "${_component_version}")
    set(_winui_${component}_version "${_component_version}")
endforeach()

_winui_download_package(
    Microsoft.WindowsAppSDK.WinUI
    "${_winui_WinUI_version}"
    _winui_winui
    "Microsoft.WindowsAppSDK.WinUI.nuspec"
    "include/*"
    "metadata/*"
)
file(READ "${_winui_winui}/Microsoft.WindowsAppSDK.WinUI.nuspec"
    _winui_winui_nuspec)

string(REGEX MATCH
    "id=\"Microsoft\\.Web\\.WebView2\"[^>]*version=\"([^\"]+)\""
    _webview_match
    "${_winui_winui_nuspec}"
)
if(NOT _webview_match)
    message(FATAL_ERROR "Could not find the WebView2 package version")
endif()
set(_winui_webview_version "${CMAKE_MATCH_1}")

if(CMAKE_GENERATOR_PLATFORM MATCHES "^(ARM64|arm64)$"
    OR CMAKE_SYSTEM_PROCESSOR MATCHES "^(ARM64|aarch64)$")
    set(_winui_architecture arm64)
elseif(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(_winui_architecture x64)
else()
    set(_winui_architecture x86)
endif()

_winui_download_package(
    Microsoft.WindowsAppSDK.Foundation
    "${_winui_Foundation_version}"
    _winui_foundation
    "include/*"
    "metadata/*"
    "lib/native/${_winui_architecture}/*"
    "runtimes/win-${_winui_architecture}/native/Microsoft.WindowsAppRuntime.Bootstrap.dll"
)
_winui_download_package(
    Microsoft.WindowsAppSDK.InteractiveExperiences
    "${_winui_InteractiveExperiences_version}"
    _winui_interactive
    "include/*"
    "metadata/10.0.17763.0/*"
)
_winui_download_package(
    Microsoft.Web.WebView2
    "${_winui_webview_version}"
    _winui_webview
    "lib/Microsoft.Web.WebView2.Core.winmd"
    "build/native/include-winrt/*"
)
_winui_download_package(
    Microsoft.Windows.CppWinRT
    "${WINUI_CPPWINRT_VERSION}"
    _winui_cppwinrt
    "bin/cppwinrt.exe"
)

set(_winui_cppwinrt_executable "${_winui_cppwinrt}/bin/cppwinrt.exe")
set(_winui_generated
    "${_winui_root}/generated/${WINUI_WINDOWS_APP_SDK_VERSION}-${WINUI_CPPWINRT_VERSION}"
)
if(NOT EXISTS "${_winui_generated}/winrt/Microsoft.UI.Xaml.h")
    file(MAKE_DIRECTORY "${_winui_generated}")
    execute_process(
        COMMAND "${_winui_cppwinrt_executable}"
            -input sdk
            -input "${_winui_winui}/metadata"
            -input "${_winui_interactive}/metadata/10.0.17763.0"
            -input "${_winui_foundation}/metadata"
            -input "${_winui_webview}/lib/Microsoft.Web.WebView2.Core.winmd"
            -output "${_winui_generated}"
        RESULT_VARIABLE generation_result
        OUTPUT_VARIABLE generation_output
        ERROR_VARIABLE generation_error
    )
    if(NOT generation_result EQUAL 0)
        message(FATAL_ERROR
            "C++/WinRT header generation failed:\n"
            "${generation_output}${generation_error}")
    endif()
endif()

string(REGEX MATCH "^([0-9]+)\\.([0-9]+)\\." _release_match
    "${WINUI_WINDOWS_APP_SDK_VERSION}")
if(NOT _release_match)
    message(FATAL_ERROR "Invalid Windows App SDK version")
endif()
math(EXPR WINUI_RELEASE_MAJOR_MINOR
    "(${CMAKE_MATCH_1} << 16) | ${CMAKE_MATCH_2}")

set(WINUI_GENERATED_INCLUDE_DIR "${_winui_generated}")
set(WINUI_INCLUDE_DIRS
    "${_winui_winui}/include"
    "${_winui_foundation}/include"
    "${_winui_interactive}/include"
    "${_winui_webview}/build/native/include-winrt"
)
set(WINUI_BOOTSTRAP_DLL
    "${_winui_foundation}/runtimes/win-${_winui_architecture}/native/Microsoft.WindowsAppRuntime.Bootstrap.dll"
)
set(WINUI_BOOTSTRAP_LIBRARY
    "${_winui_foundation}/lib/native/${_winui_architecture}/Microsoft.WindowsAppRuntime.Bootstrap.lib"
)

if(NOT EXISTS "${WINUI_GENERATED_INCLUDE_DIR}/winrt/Microsoft.UI.Xaml.h")
    message(FATAL_ERROR "WinUI C++/WinRT headers were not generated")
endif()
if(NOT EXISTS "${WINUI_BOOTSTRAP_LIBRARY}")
    message(FATAL_ERROR "Windows App SDK bootstrap library was not found")
endif()

if(NOT TARGET WinUI::WinUI)
    add_library(WinUI::WinUI INTERFACE IMPORTED)
    target_include_directories(WinUI::WinUI SYSTEM INTERFACE
        "${WINUI_GENERATED_INCLUDE_DIR}"
        ${WINUI_INCLUDE_DIRS}
    )
    target_link_libraries(WinUI::WinUI INTERFACE
        "${WINUI_BOOTSTRAP_LIBRARY}"
        runtimeobject
        windowsapp
    )
endif()

set(WinUI_FOUND TRUE)
