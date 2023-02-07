add_definitions(-DTRANSLATION_DOMAIN=\"kcm_polkitactions\")

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
     Qt::Core
     Qt::DBus
     PolkitQt${QT_MAJOR_VERSION}-1::Core
     polkitkdekcmodulesprivate
)

install(TARGETS kcm_polkitactions  DESTINATION ${KDE_INSTALL_PLUGINDIR} )


########### install files ###############

install( FILES kcm_polkitactions.desktop  DESTINATION  ${KDE_INSTALL_KSERVICESDIR} )
