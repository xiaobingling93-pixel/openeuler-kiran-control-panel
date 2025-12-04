set(TARGET_NAME kiran-cpanel-account)

file(GLOB_RECURSE ACCOUNT_SRC ./*.cpp ./*.h ./*.ui)
add_library(${TARGET_NAME} SHARED ${ACCOUNT_SRC})

target_include_directories(${TARGET_NAME} PRIVATE
        ${CMAKE_BINARY_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/pages
        ${CMAKE_CURRENT_SOURCE_DIR}/utils
        ${KIRAN_WIDGETS_INCLUDE_DIRS}
        ${KIRAN_CC_DAEMON_INCLUDE_DIRS}
        ${KLOG_INCLUDE_DIRS}
        ${KIRAN_INTEGRATION_THEME_INCLUDE_DIRS}
        ${CRYPTOPP_INCLUDE_DIRS})

target_link_libraries(${TARGET_NAME}
        plugin-devel
        Qt5::Widgets
        Qt5::DBus
        Qt5::Svg
        ${KIRAN_WIDGETS_LIBRARIES}
        ${KIRAN_CC_DAEMON_LIBRARIES}
        ${KLOG_LIBRARIES}
        ${KIRAN_INTEGRATION_THEME_LIBRARIES}
        ${CRYPTOPP_LIBRARIES})

if(PAM_FOUND)
        target_include_directories(${TARGET_NAME} PRIVATE ${PAM_INCLUDE_DIRS})
        target_link_libraries(${TARGET_NAME} ${PAM_LIBRARIES})
else()
        target_include_directories(${TARGET_NAME} PRIVATE "/usr/include/security/")
        target_link_libraries(${TARGET_NAME} pam)
endif()

install(TARGETS ${TARGET_NAME} DESTINATION ${PLUGIN_LIBS_DIR}/)
