add_library(fml_rockstar STATIC
    launcher.cpp
    games_manager.cpp
)
target_include_directories(fml_rockstar PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../../include ${CMAKE_BINARY_DIR}/generated)
target_link_libraries(fml_rockstar PUBLIC fml_core)
