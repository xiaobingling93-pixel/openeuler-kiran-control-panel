file(GLOB_RECURSE PANEL_SRC ./*.cpp ./*.h ./*.ui)
add_executable(kiran-control-panel ${PANEL_SRC})
target_include_directories(
  kiran-control-panel
  PRIVATE ${CMAKE_BINARY_DIR} ${KLOG_INCLUDE_DIRS} ${KIRAN_WIDGETS_INCLUDE_DIRS}
          ${KIRAN_INTEGRATION_THEME_INCLUDE_DIRS} ${GLIB_2_INCLUDE_DIRS})
target_link_libraries(
  kiran-control-panel
  PRIVATE common-widgets
          plugin-framework
          Qt5::Core
          Qt5::Gui
          Qt5::Svg
          Qt5::X11Extras
          Qt5::Widgets
          ${KLOG_LIBRARIES}
          ${KIRAN_WIDGETS_LIBRARIES}
          ${KIRAN_INTEGRATION_THEME_LIBRARIES}
          ${GLIB_2_LIBRARIES})
install(TARGETS kiran-control-panel DESTINATION ${CMAKE_INSTALL_FULL_BINDIR})
