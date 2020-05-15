include_directories(${CMAKE_CURRENT_SOURCE_DIR}
                    ${CMAKE_CURRENT_BINARY_DIR})

set( kcm_polkitactions_SRCS
    PolkitActionsKCM.cpp
    AuthorizationsFilterModel.cpp
    PoliciesModel.cpp
    PolicyItem.cpp
    ActionWidget.cpp
    explicitauthorizationdialog.cpp
    pkitemdelegate.cpp
)

ki18n_wrap_ui(kcm_polkitactions_SRCS
    mainview.ui
    explicitwidget.ui
    actionwidget.ui)

add_library(kcm_polkitactions ${kcm_polkitactions_SRCS})

target_link_libraries(kcm_polkitactions
     ${KDE5_KDECORE_LIBS}
     ${KDE5_KDEUI_LIBRARY}
     ${POLKITQT-1_CORE_LIBRARY}
     polkitkdekcmodulesprivate
)

install(TARGETS kcm_polkitactions  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES kcm_polkitactions.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )
