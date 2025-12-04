set(TARGET_NAME kiran-cpanel-keyboard)

kiran_qt5_add_dbus_interface_ex(KEYBOARD_PROXY
        data/com.kylinsec.Kiran.SessionDaemon.Keyboard.xml
        keyboard_backEnd_proxy
        KeyboardBackEndProxy)
file(GLOB_RECURSE KEYBOARD_SRC "./*.cpp" "./*.h" "./*.ui" "./*.qrc")
add_library(${TARGET_NAME} SHARED
        ${KEYBOARD_SRC}
        ${KEYBOARD_PROXY})

target_include_directories(${TARGET_NAME} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/pages
        ${CMAKE_CURRENT_SOURCE_DIR}/utils
        ${CMAKE_CURRENT_BINARY_DIR}
        ${KCP_PLUGIN_INCLUDE_DIR}
        ${KIRAN_WIDGETS_INCLUDE_DIRS}
        ${KLOG_INCLUDE_DIRS}
        ${KIRAN_CC_DAEMON_INCLUDE_DIRS}
        ${KIRAN_INTEGRATION_THEME_INCLUDE_DIRS})

target_link_libraries(${TARGET_NAME} PRIVATE
        plugin-devel
        Qt5::Widgets
        Qt5::DBus
        Qt5::Svg
        Qt5::Core
        ${KIRANWIDGETS_LIBRARIES}
        ${KLOG_LIBRARIES}
        ${KIRAN_INTEGRATION_THEME_LIBRARIES})

if (MODIFIER_LOCK_TIPS_VISIBLE)
    add_definitions(-DMODIFIER_LOCK_TIPS)
endif ()

install(TARGETS ${TARGET_NAME}
        DESTINATION ${PLUGIN_LIBS_DIR}/)