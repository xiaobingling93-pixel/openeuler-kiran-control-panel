set(TARGET_NAME kiran-cpanel-launcher)

file(GLOB_RECURSE LAUNCHER_SRC ./*.cpp ./*.h)
add_executable(${TARGET_NAME} ${LAUNCHER_SRC})

target_include_directories(${TARGET_NAME} PRIVATE kcp-devel ${KIRAN_WIDGETS_INCLUDE_DIRS}
                                                  ${KLOG_INCLUDE_DIRS} ${CMAKE_BINARY_DIR})
target_link_libraries(
  ${TARGET_NAME}
  common-widgets
  plugin-framework
  Qt5::Core
  Qt5::Gui
  Qt5::Svg
  Qt5::X11Extras
  Qt5::Widgets
  ${KIRAN_WIDGETS_LIBRARIES}
  ${KLOG_LIBRARIES})

install(TARGETS ${TARGET_NAME} DESTINATION ${CMAKE_INSTALL_FULL_BINDIR})
