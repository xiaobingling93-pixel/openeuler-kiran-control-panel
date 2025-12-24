set(TARGET_NAME kiran-cpanel-system)

file(GLOB_RECURSE SYSTEM_SRC ./*.cpp ./*.h ./*.ui)

if (ENABLE_SYSTEM_PRIVACY_POLICY)
    add_definitions(-DSYSTEM_PRIVACY_POLICY)
endif ()

kiran_qt5_add_dbus_interface_ex(LICENSE_DBUS_SRC
        data/com.kylinsec.Kiran.LicenseObject.xml
        dbus_license_dbus
        DBusLicenseObject)

add_library(${TARGET_NAME} SHARED
        ${SYSTEM_SRC}
        ${LICENSE_DBUS_SRC})

target_include_directories(${TARGET_NAME} PRIVATE
        ${CMAKE_CURRENT_BINARY_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${KCP_PLUGIN_INCLUDE_DIR}
        ${KLOG_INCLUDE_DIRS}
        ${KIRAN_WIDGETS_INCLUDE_DIRS}
        ${KIRAN_CC_DAEMON_INCLUDE_DIRS}
        ${KIRAN_INTEGRATION_THEME_INCLUDE_DIRS})

target_link_libraries(${TARGET_NAME}
        plugin-devel
        Qt5::Widgets
        Qt5::DBus
        Qt5::Svg
        Qt5::PrintSupport
        ${KIRAN_WIDGETS_LIBRARIES}
        ${KLOG_LIBRARIES}
        ${KIRAN_INTEGRATION_THEME_LIBRARIES})

install(TARGETS ${TARGET_NAME} DESTINATION ${PLUGIN_LIBS_DIR})
