include_directories(${CMAKE_CURRENT_SOURCE_DIR}
                    ${CMAKE_CURRENT_BINARY_DIR})

set( kcm_polkitactions_SRCS
    PolkitActionsKCM.cpp
    AuthorizationsFilterModel.cpp
    PoliciesModel.cpp
    PolicyItem.cpp
    ActionWidget.cpp
    explicitauthorizationdialog.cpp
    ../PKLAEntry.cpp
    pkitemdelegate.cpp
    ../polkitconfig/identitywidget.cpp
)

kde4_add_ui_files(kcm_polkitactions_SRCS
    mainview.ui
    explicitwidget.ui
    actionwidget.ui
    ../polkitconfig/identitywidget.ui)

kde4_add_plugin(kcm_polkitactions ${kcm_polkitactions_SRCS})

target_link_libraries(kcm_polkitactions
     ${KDE4_KDECORE_LIBS}
     ${KDE4_KDEUI_LIBRARY}
     ${POLKITQT-1_CORE_LIBRARY}
)

install(TARGETS kcm_polkitactions  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES kcm_polkitactions.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )
