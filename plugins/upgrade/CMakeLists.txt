set(TARGET_NAME kiran-cpanel-upgrade)

file(GLOB_RECURSE UPGRADE_SRC ./*.cpp ./*.h ./*.ui)

kiran_qt5_add_dbus_interface_ex(
  KCD_UPGRADE_SRC data/com.kylinsec.Kiran.SystemDaemon.Upgrade.xml
  kcd_upgrade_proxy KCDUpgradeProxy)

add_library(${TARGET_NAME} SHARED ${UPGRADE_SRC} ${KCD_UPGRADE_SRC})

target_include_directories(
  ${TARGET_NAME}
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
          ${PROJECT_BINARY_DIR}
          ${CMAKE_CURRENT_BINARY_DIR}
          ${KIRAN_CC_DAEMON_INCLUDE_DIRS}
          ${KLOG_INCLUDE_DIRS}
          ${KIRAN_WIDGETS_INCLUDE_DIRS}
          ${KCP_PLUGIN_INCLUDE_DIR})

target_link_libraries(
  ${TARGET_NAME}
  plugin-devel
  Qt5::Widgets
  Qt5::DBus
  Qt5::Svg
  ${KIRAN_WIDGETS_LIBRARIES})

install(TARGETS ${TARGET_NAME} DESTINATION ${PLUGIN_LIBS_DIR}/)
