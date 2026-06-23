add_library(fml_core STATIC
    logger.cpp
    system_detector.cpp
    config.cpp
    environment.cpp
)

target_include_directories(fml_core PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_BINARY_DIR}/generated
)

target_compile_features(fml_core PUBLIC cxx_std_20)

if(UNIX)
    target_link_libraries(fml_core PRIVATE pthread)
endif()
