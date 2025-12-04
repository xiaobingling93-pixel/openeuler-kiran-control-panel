set (TARGET_NAME dbus-wrapper)

set(DBUS_WRAPPER_GEN_CODE "")
file(GLOB_RECURSE DBUS_WRAPPER_SRC "./*.cpp" "./*.h")

# com.kylinsec.Kiran.SystemDaemon.Accounts
kiran_qt5_add_dbus_interface_ex(ACCOUNT_INTERFACE
        com.kylinsec.Kiran.SystemDaemon.Accounts.xml
        account_interface
        AccountInterface)
list(APPEND DBUS_WRAPPER_GEN_CODE ${ACCOUNT_INTERFACE})

kiran_qt5_add_dbus_interface_ex(ACCOUNT_USER_INTERFACE
        com.kylinsec.Kiran.SystemDaemon.Accounts.User.xml
        account_user_interface
        AccountUserInterface)
list(APPEND DBUS_WRAPPER_GEN_CODE ${ACCOUNT_USER_INTERFACE})

# com.kylinsec.Kiran.SessionDaemon.Appearance
kiran_qt5_add_dbus_interface_ex(APPEARANCE_INTERFACE
        com.kylinsec.Kiran.SessionDaemon.Appearance.xml
        appearance_interface
        AppearanceInterface)
list(APPEND DBUS_WRAPPER_GEN_CODE ${APPEARANCE_INTERFACE})

add_library(${TARGET_NAME} STATIC
        ${DBUS_WRAPPER_GEN_CODE}
        ${DBUS_WRAPPER_SRC} )

target_link_libraries(${TARGET_NAME} PRIVATE
        Qt5::Core
        Qt5::DBus
        ${KLOG_LIBRARIES})

target_include_directories(${TARGET_NAME} PRIVATE
        ${PROJECT_SOURCE_DIR}/common
        ${PROJECT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_BINARY_DIR}
        ${KLOG_INCLUDE_DIRS})

target_include_directories(${TARGET_NAME} INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_BINARY_DIR})