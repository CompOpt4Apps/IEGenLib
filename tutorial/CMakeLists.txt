# Add each example file as a separate executable
file(GLOB_RECURSE sparse_format_example_SOURCES sparse_format_example.cc)
add_executable(sparse_format_example EXCLUDE_FROM_ALL ${sparse_format_example_SOURCES})
add_dependencies(sparse_format_example iegenlib)
target_link_libraries(sparse_format_example iegenlib isl gmp)
target_include_directories(sparse_format_example PUBLIC ../install/include/iegenlib)

file(GLOB_RECURSE computation_example_SOURCES computation_example.cc)
add_executable(computation_example EXCLUDE_FROM_ALL ${computation_example_SOURCES})
add_dependencies(computation_example iegenlib)
target_link_libraries(computation_example iegenlib isl codegen omega gmp)
target_include_directories(computation_example PUBLIC ../install/include/iegenlib)


file(GLOB_RECURSE geoac_example_SOURCES geoac_example.cc)
add_executable(geoac_example EXCLUDE_FROM_ALL ${geoac_example_SOURCES})
add_dependencies(geoac_example iegenlib)
target_link_libraries(geoac_example iegenlib isl codegen omega gmp)
target_include_directories(geoac_example PUBLIC ../install/include/iegenlib)


file(GLOB_RECURSE geoac_codegen_driver_SOURCES geoac_codegen_driver.cc)
add_executable(geoac_codegen_driver EXCLUDE_FROM_ALL ${geoac_codegen_driver_SOURCES})
add_dependencies(geoac_codegen_driver iegenlib)
target_link_libraries(geoac_codegen_driver iegenlib isl codegen omega gmp)
target_include_directories(geoac_codegen_driver PUBLIC ../install/include/iegenlib)


# Set binary output directory for examples
set_target_properties(sparse_format_example computation_example geoac_example geoac_codegen_driver
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/tutorial"
)

# Create a 'tutorial' target to build all examples
add_custom_target(tutorial
    DEPENDS sparse_format_example
            computation_example
            geoac_example
            geoac_codegen_driver
    COMMENT "Building tutorial examples"
)
