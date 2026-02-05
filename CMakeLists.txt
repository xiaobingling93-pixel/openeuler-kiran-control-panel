cmake_minimum_required(VERSION 3.16..${CMAKE_VERSION})
project(kiran-control-panel VERSION 2.8)

set(KF5_MIN_VERSION "5.89")
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(PkgConfig REQUIRED)
find_package(ECM REQUIRED NO_MODULE)
list(APPEND CMAKE_MODULE_PATH "${ECM_MODULE_PATH}")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

# Qt Deps
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
find_package(
  Qt5
  COMPONENTS Core
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
find_package(
  Qt5
  OPTIONAL_COMPONENTS XkbCommonSupport
  QUIET)

# KF5 Deps
find_package(KF5 ${KF5_MIN_VERSION} REQUIRED
             COMPONENTS Config ConfigWidgets CoreAddons NetworkManagerQt)
# 提取KDecoration2版本号并转换为整数，用于VERSION_CHECK
find_package(KDecoration2 CONFIG REQUIRED)
message(STATUS "KDecoration2_VERSION: ${KDecoration2_VERSION}")
string(REPLACE "." ";" KDECORATION2_VERSION_LIST ${KDecoration2_VERSION})
list(GET KDECORATION2_VERSION_LIST 0 KDECORATION2_VERSION_MAJOR)
list(GET KDECORATION2_VERSION_LIST 1 KDECORATION2_VERSION_MINOR)
list(GET KDECORATION2_VERSION_LIST 2 KDECORATION2_VERSION_PATCH)
math(EXPR KDECORATION2_VERSION_INT "${KDECORATION2_VERSION_MAJOR} * 10000 + ${KDECORATION2_VERSION_MINOR} * 100 + ${KDECORATION2_VERSION_PATCH}")

# Kiran Deps
pkg_search_module(KIRAN_DESKTOP REQUIRED kiran-desktop)
pkg_search_module(KLOG REQUIRED klog-qt5)
pkg_search_module(KIRAN_WIDGETS REQUIRED kiranwidgets-qt5)
pkg_search_module(KIRAN_INTEGRATION_THEME REQUIRED kiran-integration-theme)
pkg_search_module(KIRAN_CC_DAEMON REQUIRED kiran-cc-daemon)

# X11 Deps
pkg_search_module(XCURSOR REQUIRED xcursor)
pkg_search_module(XRANDR REQUIRED xrandr)
pkg_search_module(X11 REQUIRED x11)
pkg_search_module(XKBCOMMON REQUIRED xkbcommon)

# Other Deps
pkg_search_module(PAM pam)
pkg_search_module(GLIB_2 REQUIRED glib-2.0)
pkg_search_module(LIBNOTIFY REQUIRED libnotify)
pkg_search_module(CRYPTOPP REQUIRED cryptopp)
pkg_search_module(Qt5Xdg REQUIRED Qt5Xdg)
pkg_search_module(QGSETTINGS REQUIRED gsettings-qt)
pkg_search_module(UPOWER_GLIB REQUIRED upower-glib)

# CMake Variables/Functions
include("kcp_variables")
include("kcp-qdbus-wrapper")
configure_file(./config.h.in ${CMAKE_BINARY_DIR}/config.h)

# CMake Options
include("options")

# 内部插件直接依赖该target找到开发头文件以及内部静态库
add_library(plugin-devel INTERFACE)
target_include_directories(plugin-devel INTERFACE ${PROJECT_SOURCE_DIR}/common
                                                  ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(plugin-devel INTERFACE common-widgets dbus-wrapper)

# CMake Subdirectories
add_subdirectory(lib)
add_subdirectory(src)
add_subdirectory(translations)
add_subdirectory(resources)
add_subdirectory(data)
add_subdirectory(launcher)
add_subdirectory(libexec)
add_subdirectory(plugins)
if(BUILD_EXAMPLE)
  add_subdirectory(example)
endif()

file(GLOB_RECURSE DEVELOP_HEADER include/*.h)
install(FILES ${DEVELOP_HEADER} DESTINATION ${KCP_INCLUDEDIR})