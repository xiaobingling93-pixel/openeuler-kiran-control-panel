set(TARGET_NAME kiran-cpanel-authentication)

file(GLOB_RECURSE AUTHENTICATION_SRC ./*.cpp ./*.h ./*.ui)

kiran_qt5_add_dbus_interface_ex(
  KRAN_AUTHENTICATION_SRC data/com.kylinsec.Kiran.Authentication.xml
  kas_auth_proxy KASAuthProxy)
kiran_qt5_add_dbus_interface_ex(
  KRAN_AUTHENTICATION_USER_SRC data/com.kylinsec.Kiran.Authentication.User.xml
  kas_auth_user_proxy KASAuthUserProxy)
list(APPEND AUTHENTICATION_SRC ${KRAN_AUTHENTICATION_SRC} ${KRAN_AUTHENTICATION_USER_SRC})

add_library(${TARGET_NAME} SHARED ${AUTHENTICATION_SRC})
target_include_directories(
  ${TARGET_NAME}
  PRIVATE ${PROJECT_BINARY_DIR} ${KCP_PLUGIN_INCLUDE_DIR}
          ${KIRAN_WIDGETS_INCLUDE_DIRS} ${KIRAN_AUTH_SERVICE_INCLUDE_DIRS}
          ${KLOG_INCLUDE_DIRS} ${KIRAN_INTEGRATION_THEME_INCLUDE_DIRS})

target_link_libraries(
  ${TARGET_NAME}
  plugin-devel
  Qt5::Widgets
  Qt5::DBus
  Qt5::Svg
  pam
  ${KIRAN_WIDGETS_LIBRARIES}
  ${KLOG_LIBRARIES}
  ${KIRAN_INTEGRATION_THEME_LIBRARIES}
  ${KIRAN_AUTH_SERVICE_LIBRARIES})
install(TARGETS ${TARGET_NAME} DESTINATION ${PLUGIN_LIBS_DIR}/)
