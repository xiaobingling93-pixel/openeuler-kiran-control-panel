set(TARGET_NAME kiran-cpanel-demo)

file(GLOB_RECURSE DEMO_SRC "src/*.cpp" "src/*.h" "src/*.ui")
file(GLOB DEMO_TS_FILES "translations/*.ts")

qt5_create_translation(DEMO_QM_FILES ${CMAKE_SOURCE_DIR} ${DEMO_TS_FILES})
add_library(${TARGET_NAME} SHARED
        ${DEMO_SRC}
        ${DEMO_QM_FILES})

target_include_directories(${TARGET_NAME} PRIVATE
        ${CMAKE_BINARY_DIR}
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/subitem
        ${GLIB_2_INCLUDE_DIRS}
        ${QGSETTINGS_INCLUDE_DIRS}
        ${KIRAN_CONTROL_PANEL_PKG_INCLUDE_DIRS}
        ${KIRAN_WIDGETS_INCLUDE_DIRS}
        ${KLOG_INCLUDE_DIRS})

target_link_libraries(${TARGET_NAME}
        Qt5::Core
        Qt5::Gui
        Qt5::Svg
        Qt5::Widgets
        Qt5::X11Extras
        Qt5::Xml
        X11
        ${GLIB_2_LIBRARIES}
        ${QGSETTINGS_LIBRARIES}
        ${KIRAN_WIDGETS_LIBRARIES}
        ${KLOG_LIBRARIES})

set(PLUGIN_ICONS_DIR ${INSTALL_DATADIR}/icons/hicolor/scalable/apps )
install(TARGETS ${TARGET_NAME} DESTINATION ${PLUGIN_LIBS_DIR}/)

configure_file(data/kiran-cpanel-demo.desktop.in ${CMAKE_BINARY_DIR}/kiran-cpanel-demo.desktop @ONLY)
install(FILES ${CMAKE_BINARY_DIR}/kiran-cpanel-demo.desktop DESTINATION ${PLUGIN_DESKTOP_DIR}/ )
install(FILES ${DEMO_QM_FILES} DESTINATION ${TRANSLATION_DIR} )
set(INSTALL_ICONS
        "data/kcp-demo-subitem-1.svg"
        "data/kcp-demo-subitem-2.svg"
        "data/kcp-demo.svg")
install(FILES ${INSTALL_ICONS} DESTINATION ${CMAKE_INSTALL_FULL_DATADIR}/icons/hicolor/)