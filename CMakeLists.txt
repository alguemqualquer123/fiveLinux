cmake_minimum_required(VERSION 3.20)
project(FiveMLinuxSDK VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})

option(FML_BUILD_CLI "Build CLI executable" ON)
option(FML_BUILD_GUI "Build Qt6 GUI" OFF)
option(FML_BUILD_PYTHON "Build Python bindings" OFF)
option(FML_BUILD_LUA "Build Lua bindings" OFF)
option(FML_BUILD_NODE "Build Node.js bindings" OFF)
option(FML_BUILD_TESTS "Build tests" ON)
option(FML_ENABLE_DBUS "Enable D-Bus integration" ON)
option(FML_STATIC_LINK "Static link dependencies" OFF)

set(FML_LOG_DIR "$ENV{HOME}/.local/share/fivem-linux/logs" CACHE PATH "Log directory")
set(FML_CONFIG_DIR "$ENV{HOME}/.config/fivem-linux" CACHE PATH "Config directory")

configure_file(${CMAKE_SOURCE_DIR}/src/config.h.in ${CMAKE_BINARY_DIR}/generated/config.h @ONLY)
include_directories(${CMAKE_BINARY_DIR}/generated)

add_subdirectory(src/core)
add_subdirectory(src/wine)
add_subdirectory(src/fivem)
add_subdirectory(src/gta)
add_subdirectory(src/graphics)
add_subdirectory(src/network)
add_subdirectory(src/rockstar)

add_library(fivemlinux SHARED
    src/api.cpp
)
target_include_directories(fivemlinux PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_BINARY_DIR}/generated
)

target_link_libraries(fivemlinux
    fml_core
    fml_wine
    fivem_manager
    fml_gta
    fml_graphics
    fml_network
    fml_rockstar
    stdc++fs
    pthread
)

set_target_properties(fivemlinux PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
    PUBLIC_HEADER "${CMAKE_SOURCE_DIR}/include/fivemlinux/fivemlinux.h"
    POSITION_INDEPENDENT_CODE ON
)

target_compile_definitions(fivemlinux PRIVATE FML_EXPORT)

if(FML_BUILD_CLI)
    add_subdirectory(src/cli)
endif()

if(FML_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

install(TARGETS fivemlinux
    LIBRARY DESTINATION lib
    PUBLIC_HEADER DESTINATION include/fivemlinux
)
install(DIRECTORY ${CMAKE_BINARY_DIR}/generated/ DESTINATION include/fivemlinux
    FILES_MATCHING PATTERN "*.h"
)
