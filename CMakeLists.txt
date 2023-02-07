cmake_minimum_required(VERSION 3.16)
project(polkit-kde-kcmodules-1)

set(QT_MIN_VERSION "5.9.0")
find_package(ECM 1.0.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDEFrameworkCompilerSettings NO_POLICY_SCOPE)

find_package(KF5 REQUIRED COMPONENTS I18n KCMUtils Auth)
find_package(Qt${QT_MAJOR_VERSION} ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS Core Gui Widgets DBus)

function(dbus_add_activation_system_service _sources)
    pkg_search_module( DBUS dbus-1 )
    foreach (_i ${_sources})
        get_filename_component(_service_file ${_i} ABSOLUTE)
        string(REGEX REPLACE "\\.service.*$" ".service" _output_file ${_i})
        set(_target ${CMAKE_CURRENT_BINARY_DIR}/${_output_file})
        configure_file(${_service_file} ${_target})
        install(FILES ${_target} DESTINATION ${KDE_INSTALL_DBUSSYSTEMSERVICEDIR} )
    endforeach (_i ${ARGN})
endfunction(dbus_add_activation_system_service _sources)

find_package(PolkitQt${QT_MAJOR_VERSION}-1 REQUIRED 0.103.0)

include(FindPkgConfig)

include_directories(${POLKITQT-1_INCLUDE_DIR}
                    ${CMAKE_CURRENT_SOURCE_DIR}/common)

add_subdirectory(common)
add_subdirectory(polkitactions)
add_subdirectory(polkitconfig)
add_subdirectory(helper)

install( FILES settings-system-policies.desktop  DESTINATION  ${KDE_INSTALL_KSERVICESDIR} )

ki18n_install(po)
