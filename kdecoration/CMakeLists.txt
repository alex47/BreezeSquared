add_definitions(-DTRANSLATION_DOMAIN="breeze_kwin_deco")

################# includes #################
include_directories(${CMAKE_SOURCE_DIR}/libbreezecommon)
include_directories(${CMAKE_BINARY_DIR}/libbreezecommon)

################# newt target #################
### plugin classes
set(breezesquareddecoration_SRCS
    breezebutton.cpp
    breezedecoration.cpp
    breezesettingsprovider.cpp
    breezeexceptionlist.cpp
)

### build library
kconfig_add_kcfg_files(breezesquareddecoration_SRCS breezesettings.kcfgc)
add_library(breezesquareddecoration MODULE
    ${breezesquareddecoration_SRCS}
    ${breezesquareddecoration_config_PART_FORMS_HEADERS})

set_target_properties(breezesquareddecoration PROPERTIES
    OUTPUT_NAME org.kde.breezesquared
)

target_link_libraries(breezesquareddecoration
    PRIVATE
        breezecommon6
        Qt6::DBus
        KF6::CoreAddons
        KF6::ConfigGui
        KF6::GuiAddons
        KF6::I18n
        KF6::IconThemes
        KDecoration3::KDecoration
        KF6::ColorScheme
)

install(TARGETS breezesquareddecoration DESTINATION ${KDE_INSTALL_PLUGINDIR}/${KDECORATION_PLUGIN_DIR})

add_subdirectory(config)

