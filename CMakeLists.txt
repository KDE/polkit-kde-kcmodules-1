add_subdirectory(polkitactions)
add_subdirectory(polkitconfig)
add_subdirectory(helper)

install( FILES settings-system-policies.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )
