add_library(fml_network STATIC
    latency_test.cpp
    firewall_check.cpp
    port_manager.cpp
    network.cpp
)

target_include_directories(fml_network PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_BINARY_DIR}/generated
)

target_link_libraries(fml_network PUBLIC fml_core)
