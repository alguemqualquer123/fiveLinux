add_executable(fivem-linux
    main.cpp
    command_interface.cpp
)
target_include_directories(fivem-linux PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_BINARY_DIR}/generated
)
target_link_libraries(fivem-linux PRIVATE
    fml_core
    fml_wine
    fivem_manager
    fml_gta
    fml_graphics
    fml_network
    fml_rockstar
)
target_compile_features(fivem-linux PRIVATE cxx_std_20)
install(TARGETS fivem-linux DESTINATION bin)
