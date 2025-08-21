set(TARGET_NAME kiran-cpanel-keybinding)

find_package(Qt5 OPTIONAL_COMPONENTS XkbCommonSupport QUIET)
pkg_search_module(XKBCOMMON REQUIRED xkbcommon)

kiran_qt5_add_dbus_interface_ex(KEYBINDING_PROXY
        data/com.kylinsec.Kiran.SessionDaemon.Keybinding.xml
        keybinding_backEnd_proxy
        KeybindingBackEndProxy)

file(GLOB_RECURSE KEYBINDING_SRC "./*.cpp" "./*.h" "./*.ui")

add_library(${TARGET_NAME} SHARED
        ${KEYBINDING_SRC}
        ${KEYBINDING_PROXY})

target_include_directories(${TARGET_NAME} PRIVATE
        ${PROJECT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_BINARY_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/utils
        ${KLOG_INCLUDE_DIRS}
        ${KIRAN_WIDGETS_INCLUDE_DIRS}
        ${KIRAN_CC_DAEMON_INCLUDE_DIRS}
        ${XKBCOMMON_INCLUDE_DIRS})

target_link_libraries(${TARGET_NAME}
        common-widgets
        Qt5::Widgets
        Qt5::DBus
        Qt5::Svg
        Qt5::Core
        Qt5::Concurrent
        X11
        ${KIRANWIDGETS_LIBRARIES}
        ${KLOG_LIBRARIES}
        ${XKBCOMMON_LIBRARIES})

if(Qt5XkbCommonSupport_FOUND)
        target_include_directories(${TARGET_NAME} PRIVATE ${Qt5XkbCommonSupport_PRIVATE_INCLUDE_DIRS})
        target_link_libraries(${TARGET_NAME}  Qt5::XkbCommonSupport)
else()
        target_include_directories(${TARGET_NAME} PRIVATE ${Qt5Gui_PRIVATE_INCLUDE_DIRS})
endif()

install(TARGETS ${TARGET_NAME}
        DESTINATION ${PLUGIN_LIBS_DIR}/)
