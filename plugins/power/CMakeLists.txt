set(TARGET_NAME kiran-cpanel-power)

file(GLOB_RECURSE POWER_SRC ./*.cpp ./*.h ./*.ui)

# com.kylinsec.Kiran.SessionDaemon.Power
set(KSD_POWER_XML data/com.kylinsec.Kiran.SessionDaemon.Power.xml)
set_source_files_properties(${KSD_POWER_XML} PROPERTIES INCLUDE
                                                        dbus/idle-action.h)
kiran_qt5_add_dbus_interface_ex(KSD_POWER_SRC ${KSD_POWER_XML} ksd_power_proxy
                                KSDPowerProxy)
list(APPEND POWER_SRC ${KSD_POWER_SRC})

# org.gnome.SessionManager.xml - kiran-session-manager
set(GNOME_SESSION_MANAGER_XML data/org.gnome.SessionManager.xml)
kiran_qt5_add_dbus_interface_ex(
  GNOME_SESSION_MANAGER_SOUCE ${GNOME_SESSION_MANAGER_XML}
  gnome_session_manager_proxy GnomeSMProxy)
list(APPEND POWER_SRC ${GNOME_SESSION_MANAGER_SOUCE})

add_library(${TARGET_NAME} SHARED ${POWER_SRC})

target_include_directories(
  ${TARGET_NAME}
  PRIVATE ${CMAKE_BINARY_DIR}
          ${CMAKE_CURRENT_SOURCE_DIR}
          ${KIRAN_WIDGETS_INCLUDE_DIRS}
          ${KIRAN_CC_DAEMON_INCLUDE_DIRS}
          ${KLOG_INCLUDE_DIRS}
          ${QGSETTINGS_INCLUDE_DIRS}
          ${UPOWER_GLIB_INCLUDE_DIRS}
          ${KIRAN_INTEGRATION_THEME_INCLUDE_DIRS}
          ${KCP_PLUGIN_INCLUDE_DIR})

target_link_libraries(
  ${TARGET_NAME}
  PRIVATE plugin-devel
          Qt5::Widgets
          Qt5::DBus
          ${KIRAN_WIDGETS_LIBRARIES}
          ${KIRAN_CC_DAEMON_LIBRARIES}
          ${KIRAN_INTEGRATION_THEME_LIBRARIES}
          ${KLOG_LIBRARIES}
          ${QGSETTINGS_LIBRARIES}
          ${UPOWER_GLIB_LIBRARIES})

# 安装插件
install(TARGETS ${TARGET_NAME} DESTINATION ${PLUGIN_LIBS_DIR}/)
