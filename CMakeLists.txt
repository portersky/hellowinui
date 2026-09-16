cmake_minimum_required(VERSION 3.21)
project(hellowinui VERSION 0.2.0)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Add the deps directory to the module path
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/deps")

# Platform and compiler detection
include(Platform)
include(Flags)

if(NOT IS_WINDOWS)
    message(FATAL_ERROR "hellowinui requires Windows")
endif()

find_package(fmt REQUIRED)
find_package(WinUI REQUIRED)

add_executable(hellowinui WIN32 "helloworld.cpp")
target_include_directories(hellowinui PRIVATE ".")
target_compile_features(hellowinui PRIVATE cxx_std_23)
target_compile_options(hellowinui PRIVATE ${BASE_OPTIONS})
target_compile_definitions(hellowinui PRIVATE
    ${BASE_DEFINITIONS}
    UNICODE
    _UNICODE
    WIN32_LEAN_AND_MEAN
    NOMINMAX
    WINDOWSAPPSDK_RELEASE_MAJORMINOR=${WINUI_RELEASE_MAJOR_MINOR}
)
target_link_libraries(hellowinui PRIVATE fmt::fmt WinUI::WinUI)

add_custom_command(TARGET hellowinui POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${WINUI_BOOTSTRAP_DLL}"
        "$<TARGET_FILE_DIR:hellowinui>"
)
