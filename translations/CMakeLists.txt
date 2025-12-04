file(GLOB TS_FILES *.ts)
set(PANEL_ALL_TRANSLATION_SRC_DIR
    ${PROJECT_SOURCE_DIR}/common
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/launcher
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/libexec
    ${PROJECT_SOURCE_DIR}/plugins
    ${PROJECT_SOURCE_DIR}/src)

# ts更新
execute_process(COMMAND lupdate-qt5 -no-obsolete ${PANEL_ALL_TRANSLATION_SRC_DIR} -ts ${TS_FILES})

# qm生成,默认构建依赖
qt5_add_translation(PANEL_QM_FILES ${TS_FILES})
add_custom_target(translations_qm ALL
    DEPENDS ${PANEL_QM_FILES}
    COMMENT "Generating QM translation files"
)

install(FILES ${PANEL_QM_FILES} DESTINATION ${TRANSLATION_DIR})