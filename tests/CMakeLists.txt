# SPDX-License-Identifier: BSD-3-Clause

add_executable(breezesquared_geometry_test geometrytest.cpp)
kde_target_enable_exceptions(breezesquared_geometry_test PRIVATE)
target_link_libraries(breezesquared_geometry_test PRIVATE
    Qt6::Gui
    KF6::CoreAddons
    KDecoration3::KDecoration
    KDecoration3::KDecoration3Private
)
add_dependencies(breezesquared_geometry_test breezesquareddecoration)

file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/reports")
file(GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/test-paths.json" CONTENT
    "{\"harness\":\"$<TARGET_FILE:breezesquared_geometry_test>\",\"plugin\":\"$<TARGET_FILE:breezesquareddecoration>\"}\n")

function(add_decoration_check name size)
    add_test(NAME "breezesquared.${name}"
        COMMAND breezesquared_geometry_test
            "$<TARGET_FILE:breezesquareddecoration>" "${size}" ${ARGN}
            --report "${CMAKE_CURRENT_BINARY_DIR}/reports/${name}.json")
    set_tests_properties("breezesquared.${name}" PROPERTIES TIMEOUT 120)
endfunction()

foreach(size ButtonTiny ButtonSmall ButtonDefault ButtonLarge ButtonVeryLarge)
    add_decoration_check("${size}" "${size}")
endforeach()
foreach(variant no-alpha border-on-maximized gradient alternate-layout inactive)
    add_decoration_check("${variant}" ButtonDefault "--${variant}")
endforeach()
