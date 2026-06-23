add_library(fml_graphics STATIC
    graphics.cpp
)

target_include_directories(fml_graphics PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)
