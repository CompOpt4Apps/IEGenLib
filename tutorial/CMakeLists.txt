file(GLOB_RECURSE sparse_format_example_SOURCES sparse_format_example.cc)

add_executable(sparse_format_example EXCLUDE_FROM_ALL ${sparse_format_example_SOURCES})
add_dependencies(sparse_format_example iegenlib)
target_link_libraries(sparse_format_example iegenlib isl gmp)
target_include_directories(sparse_format_example PUBLIC ../iegen/include/iegenlib)

add_custom_target(tutorial
    DEPENDS sparse_format_example
    COMMENT "Building tutorial examples"
)
