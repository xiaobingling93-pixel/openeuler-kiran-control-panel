set (TARGET_NAME common-widgets)

file(GLOB_RECURSE COMMON_WIDGETS_SRC ./*.cpp ./*.h ./*.ui)
add_library(${TARGET_NAME} STATIC ${COMMON_SRC} ${COMMON_WIDGETS_SRC})

target_include_directories(${TARGET_NAME} PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${KIRAN_INTEGRATION_THEME_INCLUDE_DIRS})

target_include_directories(${TARGET_NAME} PRIVATE
        ${CMAKE_CURRENT_BINARY_DIR}
        ${PROJECT_BINARY_DIR}
        ${PROJECT_SOURCE_DIR}/common
        ${PROJECT_SOURCE_DIR}/include
        ${KLOG_INCLUDE_DIRS}
        ${KIRAN_WIDGETS_INCLUDE_DIRS}
        ${KIRAN_INTEGRATION_THEME_INCLUDE_DIRS})

target_link_libraries(${TARGET_NAME} PRIVATE
        Qt5::Core
        Qt5::Widgets
        Qt5::Svg
        plugin-framework
        ${KIRAN_WIDGETS_LIBRARIES}
        ${KIRAN_INTEGRATION_THEME_LIBRARIES})