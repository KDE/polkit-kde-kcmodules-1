cmake_minimum_required(VERSION 2.8.9)
project(polkit-kde-kcmodules-1)

set(QT_MIN_VERSION "5.9.0")
find_package(ECM 1.0.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

include(KDEInstallDirs)
include(KDECMakeSettings)

find_package(KF5 REQUIRED COMPONENTS KDELibs4Support I18n KCMUtils Auth)
find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS Core Gui Widgets DBus)

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

set(POLKITQT5-1_MIN_VERSION "0.103.0")
find_package(PolkitQt5-1 REQUIRED)

include(FindPkgConfig)

include_directories(${POLKITQT-1_INCLUDE_DIR}
                    ${CMAKE_CURRENT_SOURCE_DIR}/common)

add_subdirectory(common)
add_subdirectory(polkitactions)
add_subdirectory(polkitconfig)
add_subdirectory(helper)

install( FILES settings-system-policies.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )
