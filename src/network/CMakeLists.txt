add_library(fml_network STATIC
    network.cpp
)

target_include_directories(fml_network PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)
