set (TARGET_NAME plugin-framework)

file(GLOB_RECURSE PLUGIN_INFO_SRC "./*.cpp" "./*.h" "./*.ui")
add_library(${TARGET_NAME} STATIC ${PLUGIN_INFO_SRC})

target_include_directories(${TARGET_NAME} PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR})

target_include_directories(${TARGET_NAME} PRIVATE
        ${PROJECT_SOURCE_DIR}/common
        ${PROJECT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_BINARY_DIR}
        ${PROJECT_BINARY_DIR}
        ${KLOG_INCLUDE_DIRS}
        ${GLIB_2_INCLUDE_DIRS}
        ${QGSETTINGS_INCLUDE_DIRS})

target_link_libraries(${TARGET_NAME} PRIVATE
        Qt5::Core
        Qt5::Widgets
        ${KLOG_LIBRARIES}
        ${GLIB_2_LIBRARIES}
        ${QGSETTINGS_LIBRARIES})