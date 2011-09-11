project(polkit-kde-kcmodules-1)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")

find_package(KDE4 REQUIRED)

function(dbus_add_activation_system_service _sources)
    pkg_search_module( DBUS dbus-1 )
    foreach (_i ${_sources})
        get_filename_component(_service_file ${_i} ABSOLUTE)
        string(REGEX REPLACE "\\.service.*$" ".service" _output_file ${_i})
        set(_target ${CMAKE_CURRENT_BINARY_DIR}/${_output_file})
        configure_file(${_service_file} ${_target})
        install(FILES ${_target} DESTINATION ${DBUS_SYSTEM_SERVICES_INSTALL_DIR} )
    endforeach (_i ${ARGN})
endfunction(dbus_add_activation_system_service _sources)

set(POLKITQT-1_MIN_VERSION "0.98.1")
find_package(PolkitQt-1 REQUIRED)

include_directories(${KDE4_INCLUDES}
                    ${POLKITQT-1_INCLUDE_DIR}
                    ${CMAKE_CURRENT_SOURCE_DIR}/common)

add_subdirectory(common)
add_subdirectory(polkitactions)
add_subdirectory(polkitconfig)
add_subdirectory(helper)

install( FILES settings-system-policies.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )
