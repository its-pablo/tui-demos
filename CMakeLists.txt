cmake_minimum_required(VERSION 3.28)
project(Gardener
    LANGUAGES CXX
    VERSION 0.0.1
    DESCRIPTION "Remote gardening tool for my dad!"
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

#find_program(CLANG_TIDY NAMES clang-tidy)

#if(CLANG_TIDY)
#    set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY};-checks=-*,clang-analyzer-*,-clang-analyzer-cplusplus*;-warnings-as-errors=*")
#else()
#    message(WARNING "clang-tidy not found!")
#endif()

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/*.cc)
add_executable(gardener ${SOURCES})
target_include_directories(gardener PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src/devices)

include(FetchContent)

FetchContent_Declare(
    tomlplusplus
    GIT_REPOSITORY  https://github.com/marzer/tomlplusplus.git
    GIT_TAG         v3.4.0
    GIT_PROGRESS    TRUE
    GIT_SHALLOW     TRUE
    EXCLUDE_FROM_ALL
)
FetchContent_MakeAvailable(tomlplusplus)
target_link_libraries(gardener tomlplusplus::tomlplusplus)

FetchContent_Declare(
    cxxopts
    GIT_REPOSITORY  https://github.com/jarro2783/cxxopts.git
    GIT_TAG         v3.3.1
    GIT_PROGRESS    TRUE
    GIT_SHALLOW     TRUE
    EXCLUDE_FROM_ALL
)
FetchContent_MakeAvailable(cxxopts)
target_link_libraries(gardener cxxopts::cxxopts)

FetchContent_Declare(
    ftxui
    GIT_REPOSITORY  https://github.com/arthursonzogni/ftxui.git
    GIT_TAG         v6.1.9
    GIT_PROGRESS    TRUE
    GIT_SHALLOW     TRUE
    EXCLUDE_FROM_ALL
)
FetchContent_MakeAvailable(ftxui)
target_link_libraries(gardener
    ftxui::screen
    ftxui::dom
    ftxui::component
)
