add_library(fml_wine STATIC
    prefix_manager.cpp
    registry.cpp
    dll_manager.cpp
    proton_support.cpp
)
target_include_directories(fml_wine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../../include ${CMAKE_BINARY_DIR}/generated)
target_link_libraries(fml_wine PUBLIC fml_core)
