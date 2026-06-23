add_library(fivem_manager STATIC
    installer.cpp
    updater.cpp
    launcher.cpp
    cache_manager.cpp
    server_tools.cpp
)
target_include_directories(fivem_manager PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../../include ${CMAKE_BINARY_DIR}/generated)
target_link_libraries(fivem_manager PUBLIC fml_core)
