set(TARGET_NAME kiran-cpanel-group)

file(GLOB_RECURSE GROUP_SRC ./*.cpp ./*.h ./*.ui)

kiran_qt5_add_dbus_interface_ex(
  KCD_GROUP_ADM_SRC data/com.kylinsec.Kiran.SystemDaemon.Groups.xml
  ksd_group_admin_proxy KSDGroupAdminProxy)
kiran_qt5_add_dbus_interface_ex(
  KCD_GROUP_ADMIN_LIST_SRC
  data/com.kylinsec.Kiran.SystemDaemon.Groups.Group.xml
  ksd_group_admin_list_proxy KSDGroupAdminListProxy)
kiran_qt5_add_dbus_interface_ex(
  KCD_ACCOUNTS_SRC data/com.kylinsec.Kiran.SystemDaemon.Accounts.xml
  ksd_accounts_proxy KSDAccountsProxy)
kiran_qt5_add_dbus_interface_ex(
  KCD_ACCOUNTS_USER_SRC data/com.kylinsec.Kiran.SystemDaemon.Accounts.User.xml
  ksd_accounts_user_proxy KSDAccountsUserProxy)
list(APPEND GROUP_SRC ${KCD_GROUP_ADM_SRC} ${KCD_GROUP_ADMIN_LIST_SRC}
     ${KCD_ACCOUNTS_SRC} ${KCD_ACCOUNTS_USER_SRC})

add_library(
  ${TARGET_NAME} SHARED ${GROUP_SRC})

target_include_directories(
  ${TARGET_NAME}
  PRIVATE ${PROJECT_BINARY_DIR}
          ${CMAKE_CURRENT_BINARY_DIR}
          ${CMAKE_CURRENT_SOURCE_DIR}/src/
          ${CMAKE_CURRENT_SOURCE_DIR}/src/pages
          ${CMAKE_CURRENT_SOURCE_DIR}/src/tools
          ${CMAKE_CURRENT_SOURCE_DIR}/src/widgets
          ${KCP_PLUGIN_INCLUDE_DIR}
          ${GLIB_INCLUDE_DIRS}
          ${GSETTINGS_QT_INCLUDE_DIRS}
          ${KIRAN_CONTROL_PANEL_PKG_INCLUDE_DIRS}
          ${KIRAN_WIDGETS_INCLUDE_DIRS}
          ${KLOG_INCLUDE_DIRS}
          ${KIRAN_CC_DAEMON_INCLUDE_DIRS}
          ${KIRAN_INTEGRATION_THEME_INCLUDE_DIRS})

target_link_libraries(
  ${TARGET_NAME} PRIVATE
  plugin-devel
  Qt5::Core
  Qt5::Gui
  Qt5::Svg
  Qt5::Widgets
  Qt5::X11Extras
  Qt5::Xml
  Qt5::DBus
  X11
  ${GLIB_LIBRARIES}
  ${GSETTINGS_QT_LIBRARIES}
  ${KIRAN_INTEGRATION_THEME_LIBRARIES}
  ${KIRAN_WIDGETS_LIBRAIES}
  ${KLOG_LIBRARIES}
  ${KIRAN_CC_DAEMON_LIBRARIES})

install(TARGETS ${TARGET_NAME} DESTINATION ${PLUGIN_LIBS_DIR}/)
