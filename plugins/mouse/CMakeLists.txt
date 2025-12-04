set(TARGET_NAME kiran-cpanel-mouse)
file(GLOB_RECURSE MOUSE_SRC "./*.cpp" "./*.h" "./*.ui" "./*.qrc" )

kiran_qt5_add_dbus_interface_ex(MOUSE_PROXY
        data/com.kylinsec.Kiran.SessionDaemon.Mouse.xml
        mouse_backEnd_proxy
        MouseBackEndProxy
        )

kiran_qt5_add_dbus_interface_ex(TOUCHPAD_PROXY
        data/com.kylinsec.Kiran.SessionDaemon.TouchPad.xml
        touchPad_backEnd_proxy
        TouchPadBackEndProxy
        )

add_library(${TARGET_NAME} SHARED
        ${MOUSE_SRC}
        ${MOUSE_PROXY}
        ${TOUCHPAD_PROXY})

target_include_directories(${TARGET_NAME} PRIVATE
        ${PROJECT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/pages
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}
        ${KLOG_INCLUDE_DIRS}
        ${KIRAN_WIDGETS_INCLUDE_DIRS}
        ${KIRAN_CC_DAEMON_INCLUDE_DIRS})

target_link_libraries(${TARGET_NAME} PRIVATE
        plugin-devel
        Qt5::Widgets
        Qt5::DBus
        Qt5::Svg
        Qt5::Core
        ${KIRANWIDGETS_LIBRARIES}
        ${KLOG_LIBRARIES})

if (MOUSE_WHEEL_TEST_VISIBLE)
    add_definitions(-DMOUSE_WHEEL_DIRECTION_TEST)
endif ()

install(TARGETS ${TARGET_NAME}
        DESTINATION ${PLUGIN_LIBS_DIR}/)
