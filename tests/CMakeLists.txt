# Unit tests for range-model-lib (QTest based).
#
# Enabled by configuring the project with -DRANGE_BUILD_TESTS=ON.
# Run with `ctest` from the build directory.

find_package(Qt6 REQUIRED COMPONENTS Test)

if(WIN32 AND CMAKE_VERSION VERSION_LESS 3.22)
    message(FATAL_ERROR "Windows QTest execution requires CMake 3.22 or newer for ENVIRONMENT_MODIFICATION")
endif()

set(CMAKE_AUTOMOC ON)

set(range_model_lib_tests
    tst_node
    tst_line
    tst_element
    tst_element_descriptor
    tst_element_volume
    tst_segment
    tst_triangle
    tst_tetrahedron
    tst_sparse_vector
    tst_sparse_matrix
    tst_material
    tst_mesh_pipeline
    tst_problem_setup
    tst_surface
    tst_structural_product_profile
    tst_structural_cable_setup
)

foreach(test_name IN LISTS range_model_lib_tests)
    qt_add_executable(${test_name} ${test_name}.cpp)
    target_link_libraries(${test_name}
        PRIVATE
            range-model-lib
            Qt6::Test
    )
    add_test(NAME ${test_name} COMMAND ${test_name})
    if(WIN32)
        set_tests_properties(${test_name} PROPERTIES
            ENVIRONMENT_MODIFICATION "PATH=path_list_prepend:$<TARGET_FILE_DIR:Qt6::Core>")
    endif()
endforeach()

if(RANGE_GMSH_RUNTIME)
    foreach(test_name IN LISTS range_model_lib_tests)
        add_custom_command(TARGET ${test_name} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${RANGE_GMSH_RUNTIME}"
                    "$<TARGET_FILE_DIR:${test_name}>"
            VERBATIM)
    endforeach()
endif()
