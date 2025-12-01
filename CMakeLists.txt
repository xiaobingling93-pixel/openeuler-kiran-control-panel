cmake_minimum_required(VERSION 3.5..${CMKAE_VERSION})

project(kiran-control-panel VERSION 2.5)

set(KF5_MIN_VERSION "5.89")
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
include("options")
include("kcp_variables")
include("kcp-qdbus-wrapper")

find_package(ECM REQUIRED NO_MODULE)
list(APPEND CMAKE_MODULE_PATH "${ECM_MODULE_PATH}")

configure_file(./config.h.in ${CMAKE_BINARY_DIR}/config.h)

if( SERVER_MODE )
    set(ENABLE_SERVER_MODE true)
else()
    set(ENABLE_SERVER_MODE false)
endif()

find_package(Qt5 COMPONENTS
        Core
        Gui
        Widgets
        Multimedia
        Network
        X11Extras
        Svg
        DBus
        PrintSupport
        Xml
        Concurrent
        LinguistTools)

find_package(PkgConfig REQUIRED)
pkg_search_module(KLOG REQUIRED klog-qt5)
pkg_search_module(KIRAN_WIDGETS REQUIRED kiranwidgets-qt5)
pkg_search_module(KIRAN_INTEGRATION_THEME REQUIRED kiran-integration-theme)
pkg_search_module(GLIB_2 REQUIRED glib-2.0)
pkg_search_module(KIRAN_CC_DAEMON REQUIRED kiran-cc-daemon)
pkg_search_module(QGSETTINGS REQUIRED gsettings-qt)


file(GLOB_RECURSE COMMON_SRC "./common/*.cpp" "./common/*.h")
file(GLOB_RECURSE INCLUDE_SRC "./include/*.cpp" "./include/*.h")
file(GLOB_RECURSE CONTROL_PANEL_SRC "./src/*.cpp" "./src/*.h")
file(GLOB_RECURSE LIB_SRC "./lib/*.cpp" "./lib/*.c" "./lib/*.h")
file(GLOB_RECURSE PLUGINS_SRC "./plugins/*.cpp"  "./plugins/*.h")
file(GLOB_RECURSE LAUNCHER_SRC "./launcher/*.cpp" "./launcher/*.h")
#NOTE: 资源文件名不能与插件资源名相同，否则会导致插件中部分图片无法正常加载显示。
set(RESOURCE ./resources/control-panel-resources.qrc)

## 翻译文件
file(GLOB TS_FILES "translations/*.ts")
set(PANEL_ALL_TRANSLATION_SRC_DIR
        ${PROJECT_SOURCE_DIR}/common
        ${PROJECT_SOURCE_DIR}/include
        ${PROJECT_SOURCE_DIR}/launcher
        ${PROJECT_SOURCE_DIR}/lib
        ${PROJECT_SOURCE_DIR}/libexec
        ${PROJECT_SOURCE_DIR}/plugins
        ${PROJECT_SOURCE_DIR}/src)
qt5_create_translation(PANEL_QM_FILES ${PANEL_ALL_TRANSLATION_SRC_DIR} ${TS_FILES})

add_executable(${PROJECT_NAME}
        ${CONTROL_PANEL_SRC}
        ${COMMON_SRC}
        ${RESOURCE}
        ${PANEL_QM_FILES})

target_include_directories(${PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_BINARY_DIR}
        ${PROJECT_SOURCE_DIR}/include
        src
        ${KLOG_INCLUDE_DIRS}
        ${KIRAN_WIDGETS_INCLUDE_DIRS}
        ${KIRAN_INTEGRATION_THEME_INCLUDE_DIRS}
        ${GLIB_2_INCLUDE_DIRS})

target_link_libraries(${PROJECT_NAME}
        common-widgets
        plugin-framework
        Qt5::Core
        Qt5::Gui
        Qt5::Svg
        Qt5::X11Extras
        Qt5::Widgets
        dl
        pthread
        ${KLOG_LIBRARIES}
        ${KIRAN_WIDGETS_LIBRARIES}
        ${KIRAN_INTEGRATION_THEME_LIBRARIES}
        ${GLIB_2_LIBRARIES})
        
## 更新翻译文件
add_custom_target(update_translations
    COMMAND ${Qt5_LUPDATE_EXECUTABLE} -no-obsolete ${PANEL_ALL_TRANSLATION_SRC_DIR} -ts ${TS_FILES}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Updating translations and removing obsolete entries"
)
add_dependencies(${PROJECT_NAME} update_translations)

install(FILES ${INCLUDE_SRC} DESTINATION ${KCP_INCLUDEDIR})
install(FILES ${PANEL_QM_FILES} DESTINATION ${TRANSLATION_DIR})
install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_FULL_BINDDIR})

add_subdirectory(lib)
add_subdirectory(libexec)
add_subdirectory(launcher)
add_subdirectory(plugins)
add_subdirectory(data)

if(BUILD_EXAMPLE)
        add_subdirectory(example)
endif()