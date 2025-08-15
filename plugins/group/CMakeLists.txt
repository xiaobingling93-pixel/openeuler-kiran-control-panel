set(TARGET_NAME kiran-cpanel-group)

file(GLOB_RECURSE SRC "src/*.cpp" "src/*.h" "src/*.ui")
file(GLOB GROUP_TS_FILES "translations/*.ts")
file(GLOB_RECURSE RESOURCES "resources/*.qrc")
file(GLOB_RECURSE TOOLS_SOURCES "tools/*.cpp")
file(GLOB_RECURSE TOOLS_HEADERS "tools/*.h")

qt5_create_translation(GROUP_QM_FILES ${CMAKE_CURRENT_SOURCE_DIR} "translations/kiran-cpanel-group.zh_CN.ts")
#DBus代理代码生成
set(DBUS_SRC_LIST "")
##group admin service
set(KSD_GROUP_ADMIN_XML data/com.kylinsec.Kiran.SystemDaemon.Groups.xml)
set_source_files_properties(${KSD_GROUP_ADMIN_XML}
        PROPERTIES
        CLASSNAME KSDGroupAdminProxy
        NO_NAMESPACE true)
kiran_qt5_add_dbus_interface(KSD_GROUP_ADMIN_SRC ${KSD_GROUP_ADMIN_XML} ksd_group_admin_proxy)
list(APPEND DBUS_SRC_LIST ${KSD_GROUP_ADMIN_SRC})
##group admin list
set(KSD_GROUP_ADMIN_LIST_XML data/com.kylinsec.Kiran.SystemDaemon.Groups.Group.xml)
set_source_files_properties(${KSD_GROUP_ADMIN_LIST_XML}
        PROPERTIES
        CLASSNAME KSDGroupAdminListProxy
        NO_NAMESPACE true)
kiran_qt5_add_dbus_interface(KSD_GROUP_ADMIN_LIST_SRC ${KSD_GROUP_ADMIN_LIST_XML} ksd_group_admin_list_proxy)
list(APPEND DBUS_SRC_LIST ${KSD_GROUP_ADMIN_LIST_SRC})
set(KSD_ACCOUNTS_XML data/com.kylinsec.Kiran.SystemDaemon.Accounts.xml)
set_source_files_properties(${KSD_ACCOUNTS_XML}
        PROPERTIES
        CLASSNAME KSDAccountsProxy
        NO_NAMESPACE true)
kiran_qt5_add_dbus_interface(KSD_ACCOUNTS_SRC ${KSD_ACCOUNTS_XML} ksd_accounts_proxy)
list(APPEND DBUS_SRC_LIST ${KSD_ACCOUNTS_SRC})
##kiran accounts user
set(KSD_ACCOUNTS_USER_XML data/com.kylinsec.Kiran.SystemDaemon.Accounts.User.xml)
set_source_files_properties(${KSD_ACCOUNTS_USER_XML}
        PROPERTIES
        CLASSNAME KSDAccountsUserProxy
        NO_NAMESPACE true)
kiran_qt5_add_dbus_interface(KSD_ACCOUNTS_USER_SRC ${KSD_ACCOUNTS_USER_XML} ksd_accounts_user_proxy)
list(APPEND DBUS_SRC_LIST ${KSD_ACCOUNTS_USER_SRC})


add_library(${TARGET_NAME} SHARED
        ${SRC}
        ${GROUP_QM_FILES}
        ${DBUS_SRC_LIST}
        ${RESOURCES}
        ${TOOLS_SOURCES}
        ${TOOLS_HEADERS}
        )

target_include_directories(${TARGET_NAME} PRIVATE
        ${PROJECT_BINARY_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}
        ${PROJECT_SOURCE_DIR}/include
        ${PROJECT_SOURCE_DIR}/common
        ${CMAKE_CURRENT_SOURCE_DIR}/src/
        ${CMAKE_CURRENT_SOURCE_DIR}/src/pages
        ${CMAKE_CURRENT_SOURCE_DIR}/src/tools
        ${CMAKE_CURRENT_SOURCE_DIR}/src/widgets
        ${KCP_PLUGIN_INCLUDE_DIR}
        ${GLIB_INCLUDE_DIRS}
        ${GSETTINGS_QT_INCLUDE_DIRS}
        ${KIRAN_CONTROL_PANEL_PKG_INCLUDE_DIRS}
	${KIRAN_WIDGETS_INCLUDE_DIRS}
        ${KLOG_QT5_INCLUDE_DIRS}
        ${KIRAN_CC_DAEMON_INCLUDE_DIRS}
        ${KIRAN_INTEGRATION_THEME_INCLUDE_DIRS})

target_link_libraries(${TARGET_NAME}
        common-widgets
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
        ${KLOG_QT5_LIBRARIES}
        ${KIRAN_CC_DAEMON_LIBRARIES})


#安装控制中心插件
install(TARGETS ${TARGET_NAME} DESTINATION ${PLUGIN_LIBS_DIR}/)

#安装翻译文件
install(FILES ${GROUP_QM_FILES} DESTINATION ${TRANSLATION_DIR} )


