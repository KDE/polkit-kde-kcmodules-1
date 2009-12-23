set( kcm_polkitconfig_SRCS
     kcmpolkitconfig.cpp
     identitywidget.cpp
)

kde4_add_ui_files(kcm_polkitconfig_SRCS 
    polkitconfig.ui
    identitywidget.ui)

kde4_add_plugin(kcm_polkitconfig ${kcm_polkitconfig_SRCS})

target_link_libraries(kcm_polkitconfig  
     ${KDE4_KDECORE_LIBS}
     ${KDE4_KDEUI_LIBRARY}
)

install(TARGETS kcm_polkitconfig  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES kcm_polkitconfig.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )

