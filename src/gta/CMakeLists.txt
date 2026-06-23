add_library(fml_gta STATIC
    installation_detector.cpp
    files_checker.cpp
    repair_system.cpp
)
target_include_directories(fml_gta PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../../include ${CMAKE_BINARY_DIR}/generated)
target_link_libraries(fml_gta PUBLIC fml_core)
