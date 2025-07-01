# 主题图标
install(DIRECTORY hicolor DESTINATION ${CMAKE_INSTALL_FULL_DATADIR}/icons/)

# 安装主分类文件
file(GLOB CATEGORY_DESKTOP "category/desktop/*.desktop")

install(FILES ${CATEGORY_DESKTOP} DESTINATION ${CATEGORY_DESKTOP_DIR})

# 安装主分类相关图标文件
file(GLOB CATEGORY_ICONS "./category/images/*")
install(FILES ${CATEGORY_ICONS} DESTINATION ${CATEGORY_ICON_DIR})

# 主面板desktop
configure_file(${CMAKE_SOURCE_DIR}/data/${PROJECT_NAME}.desktop.in ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.desktop)
install(FILES ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.desktop DESTINATION ${CMAKE_INSTALL_FULL_DATADIR}/applications/)

# 控制中心pkgconfig文件
configure_file(${CMAKE_SOURCE_DIR}/data/${PROJECT_NAME}.pc.in ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.pc @ONLY)
install(FILES ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)

install(FILES ${CMAKE_SOURCE_DIR}/data/00-server.conf DESTINATION /etc/NetworkManager/conf.d)

install(DIRECTORY ./avatars DESTINATION ${KCP_ACCOUNT_DATADIR} )

# 声音测试音频文件
file(GLOB SOUND_FILES "sounds/*.ogg")
install(FILES ${SOUND_FILES} DESTINATION ${AUDIO_SOUNDS_DIR})

# GSettings配置
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/com.kylinsec.kiran.control-panel.plugin.gschema.xml 
        DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATADIR}/glib-2.0/schemas/)
