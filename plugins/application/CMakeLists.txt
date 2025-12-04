set(TARGET_NAME kiran-cpanel-application)

file(GLOB_RECURSE APPLICATION_SRC ./*.cpp ./*.h ./*.ui)

add_library(${TARGET_NAME} SHARED ${APPLICATION_SRC})

target_include_directories(
  ${TARGET_NAME}
  PRIVATE ${PROJECT_BINARY_DIR}
          ${CMAKE_CURRENT_BINARY_DIR}
          ${CMAKE_CURRENT_SOURCE_DIR}
          ${KIRAN_WIDGETS_QT5_INCLUDE_DIRS}
          ${KLOG_INCLUDE_DIRS}
          ${KIRAN_WIDGETS_INCLUDE_DIRS}
          ${KIRAN_CC_DAEMON_INCLUDE_DIRS}
          ${KIRAN_INTEGRATION_THEME_INCLUDE_DIRS}
          ${DEFAULTAPP_INCLUDE_DIRS}
          ${Qt5Xdg_INCLUDE_DIRS}
          ${GLIB_2_INCLUDE_DIRS})

target_link_libraries(
  ${TARGET_NAME} PRIVATE
  plugin-devel
  Qt5::Widgets
  Qt5::DBus
  Qt5::Svg
  Qt5::PrintSupport
  KF5::ConfigCore
  KF5::ConfigCore
  ${KIRAN_WIDGETS_LIBRARIES}
  ${KLOG_LIBRARIES}
  ${KIRAN_INTEGRATION_THEME_LIBRARIES}
  ${Qt5Xdg_LIBRARIES}
  ${GLIB_2_LIBRARIES})

install(TARGETS ${TARGET_NAME} DESTINATION ${PLUGIN_LIBS_DIR}/)
