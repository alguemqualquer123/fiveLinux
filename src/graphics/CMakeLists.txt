add_library(fml_graphics STATIC
    vulkan_checker.cpp
    dxvk_checker.cpp
    vkd3d_checker.cpp
    gpu_detector.cpp
    graphics.cpp
)

target_include_directories(fml_graphics PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_BINARY_DIR}/generated
)

target_link_libraries(fml_graphics PUBLIC fml_core)
