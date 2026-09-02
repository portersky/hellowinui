cmake_minimum_required(VERSION 3.21)
project(hellowinui VERSION 0.1.0)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Add the deps directory to the module path
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/deps")

# Platform and compiler detection
include(Platform)
include(Flags)

find_package(fmt REQUIRED)

# Hello World executable
add_executable(hellowinui "helloworld.cpp")
target_include_directories(hellowinui PRIVATE ".")
target_compile_features(hellowinui PRIVATE cxx_std_23)
target_compile_options(hellowinui PRIVATE ${BASE_OPTIONS})
target_compile_definitions(hellowinui PRIVATE ${BASE_DEFINITIONS})
target_link_libraries(hellowinui PRIVATE fmt::fmt)
