add_definitions(-DTRANSLATION_DOMAIN=\"kcm_polkitconfig\")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}
                    ${CMAKE_CURRENT_BINARY_DIR})

set( kcm_polkitconfig_SRCS
     kcmpolkitconfig.cpp
)

ki18n_wrap_ui(kcm_polkitconfig_SRCS
    polkitconfig.ui)

add_library(kcm_polkitconfig ${kcm_polkitconfig_SRCS})

target_link_libraries(kcm_polkitconfig
     polkitkdekcmodulesprivate
)

install(TARGETS kcm_polkitconfig  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES kcm_polkitconfig.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )

