include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

# I know, sucks extremely hard, but it's the only way.

set(policy_gen_SRCS
    policy-gen/policy-gen.cpp
    policy-gen/kauth-policy-gen-polkit1.cpp )

kde4_add_executable(polkit-kde-helper-policy-gen NOGUI ${policy_gen_SRCS})

target_link_libraries(polkit-kde-helper-policy-gen ${QT_QTCORE_LIBRARY})

## Execute it
set(_output ${CMAKE_CURRENT_BINARY_DIR}/org.kde.polkitkde1.policy)
get_filename_component(_input polkitkde1.actions ABSOLUTE)

add_custom_command(OUTPUT ${_output} 
                   COMMAND ${CMAKE_CURRENT_BINARY_DIR}/polkit-kde-helper-policy-gen ${_input} > ${_output} 
                   MAIN_DEPENDENCY ${_input}
                   WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                   COMMENT "Generating org.kde.polkitkde1.policy"
                   DEPENDS polkit-kde-helper-policy-gen)
add_custom_target("actions for polkit-kde" ALL DEPENDS ${_output})

install(FILES ${_output} DESTINATION ${POLKITQT-1_POLICY_FILES_INSTALL_DIR})

#########

set(polkitkde1helper_SRCS
    main.cpp
    polkitkde1helper.cpp
)

qt4_add_dbus_adaptor(polkitkde1helper_SRCS
    org.kde.polkitkde1.helper.xml
    polkitkde1helper.h
    PolkitKde1Helper
)

kde4_add_executable(polkitkde1helper ${polkitkde1helper_SRCS})

target_link_libraries(polkitkde1helper ${QT_QTCORE_LIBRARY} ${QT_QTXML_LIBRARY} ${QT_QTDBUS_LIBRARY} ${POLKITQT-1_CORE_LIBRARY} polkitkdekcmodulesprivate)

install(TARGETS polkitkde1helper DESTINATION ${LIBEXEC_INSTALL_DIR})

dbus_add_activation_system_service(org.kde.polkitkde1.helper.service.in)

install(FILES org.kde.polkitkde1.helper.conf DESTINATION ${SYSCONF_INSTALL_DIR}/dbus-1/system.d)

