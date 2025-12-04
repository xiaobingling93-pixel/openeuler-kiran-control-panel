set(TARGET_NAME kiran-cpanel-network)

file(GLOB_RECURSE NETWORK_SOURCE_FILES ./*.cpp ./*.h ./*.ui)

add_library(${TARGET_NAME} SHARED ${NETWORK_SOURCE_FILES})

target_include_directories(
  ${TARGET_NAME}
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/pages/
          ${CMAKE_CURRENT_SOURCE_DIR}/widgets/
          ${CMAKE_CURRENT_SOURCE_DIR}/utils/
          ${KCP_PLUGIN_INCLUDE_DIR}
          ${CMAKE_BINARY_DIR}
          ${KIRAN_WIDGETS_INCLUDE_DIRS}
          ${KLOG_INCLUDE_DIRS}
          ${LIBNOTIFY_INCLUDE_DIRS}
          ${KIRAN_INTEGRATION_THEME_INCLUDE_DIRS}
          ${KIRAN_DESKTOP_INCLUDE_DIRS})

target_link_libraries(
  ${TARGET_NAME}
  PRIVATE plugin-devel
          KF5::NetworkManagerQt
          ${KIRAN_WIDGETS_QT5_LIBRARIES}
          ${KLOG_LIBRARIES}
          ${LIBNOTIFY_LIBRARIES}
          ${KIRAN_INTEGRATION_THEME_LIBRARIES}
          ${KIRAN_DESKTOP_LIBRARIES})

install(TARGETS ${TARGET_NAME} DESTINATION ${PLUGIN_LIBS_DIR}/)

file(GLOB SVG_THEME_ICONS "./resources/kcp-network-images/*.svg")
install(FILES ${SVG_THEME_ICONS}
        DESTINATION ${CMAKE_INSTALL_FULL_DATADIR}/icons/hicolor/scalable/apps)
