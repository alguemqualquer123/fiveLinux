cmake_minimum_required(VERSION 3.20)

project(fml_core LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(fml_core STATIC
    logger.cpp
    system_detector.cpp
    config.cpp
    environment.cpp
)

target_include_directories(fml_core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/../../include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_features(fml_core PUBLIC cxx_std_20)

if(UNIX)
    target_link_libraries(fml_core PRIVATE pthread)
endif()