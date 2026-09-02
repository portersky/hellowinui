include(FetchContent)

if(NOT TARGET fmt::fmt)
    FetchContent_Declare(
        fmt
        GIT_REPOSITORY https://github.com/fmtlib/fmt.git
        GIT_TAG 11.2.0
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(fmt)
endif()

set(fmt_FOUND TRUE)
