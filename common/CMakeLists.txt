include_directories(${CMAKE_CURRENT_SOURCE_DIR}
                    ${CMAKE_CURRENT_BINARY_DIR})

set( polkitkdekcmodulesprivate_SRCS
     PKLAEntry.cpp
     identitywidget.cpp
)

ki18n_wrap_ui(polkitkdekcmodulesprivate_SRCS
    identitywidget.ui)

add_library (polkitkdekcmodulesprivate SHARED ${polkitkdekcmodulesprivate_SRCS})

set_target_properties (polkitkdekcmodulesprivate PROPERTIES VERSION   0.1.0
                                                            SOVERSION 0)

target_link_libraries(polkitkdekcmodulesprivate
    KF5::KCMUtils
    KF5::KDELibs4Support
    KF5::I18n
)

install(TARGETS polkitkdekcmodulesprivate  DESTINATION ${LIB_INSTALL_DIR} )
