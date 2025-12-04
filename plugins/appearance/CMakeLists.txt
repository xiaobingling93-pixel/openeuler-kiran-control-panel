set(TARGET_NAME kiran-cpanel-appearance)

file(GLOB_RECURSE APPEARANCE_SRC ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/*.h
     ${CMAKE_CURRENT_SOURCE_DIR}/*.ui)

add_library(${TARGET_NAME} SHARED ${APPEARANCE_SRC})

target_include_directories(
  ${TARGET_NAME}
  PRIVATE ${CMAKE_BINARY_DIR}
          ${CMAKE_CURRENT_BINARY_DIR}
          ${CMAKE_CURRENT_SOURCE_DIR}/utils
          ${KLOG_INCLUDE_DIRS}
          ${KIRAN_WIDGETS_INCLUDE_DIRS}
          ${KIRAN_CC_DAEMON_INCLUDE_DIRS}
          ${KIRAN_INTEGRATION_THEME_INCLUDE_DIRS})

target_link_libraries(
  ${TARGET_NAME}
  PRIVATE plugin-devel
          Qt5::Widgets
          Qt5::DBus
          Qt5::Svg
          Qt5::Core
          KDecoration2::KDecoration
          KDecoration2::KDecoration2Private
          KF5::CoreAddons
          KF5::ConfigCore
          KF5::ConfigWidgets
          ${XCURSOR_LIBRARIES}
          ${X11_LIBRARIES}
          ${KIRANWIDGETS_LIBRARIES}
          ${KLOG_LIBRARIES}
          ${KIRAN_INTEGRATION_THEME_LIBRARIES})

install(TARGETS ${TARGET_NAME} DESTINATION ${PLUGIN_LIBS_DIR}/)
