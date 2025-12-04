/**
 * Copyright (c) 2020 ~ 2021 KylinSec Co., Ltd.
 * kiran-control-panel is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 *
 * Author:     yuanxing <yuanxing@kylinsec.com.cn>
 */
#include "cursor-theme-page.h"
#include <kiran-message-box.h>
#include <kiran-session-daemon/appearance-i.h>
#include <QDir>
#include <QIcon>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QLabel>
#include <QVBoxLayout>
#include "appearance-global-info.h"
#include "cursor-image-loader.h"
#include "exclusion-group.h"
#include "logging-category.h"
#include "theme-preview.h"

// 兼容不同开源版本光标名称，若光标名不存在，则使用默认光标
static const std::vector<const char*> cursorNames[] = {
    {"left_ptr", "default", "top_left_arrow", "left_arrow"},
    {"pointing_hand", "pointer", "hand1", "e29285e634086352946a0e7090d73106", "default"},
    {"size_ver", "ns-resize", "v_double_arrow", "00008160000006810000408080010102", "default"},
    {"size_hor", "ew-resize", "h_double_arrow", "028006030e0e7ebffc7f7070c0600140", "default"},
    {"size_bdiag", "nesw-resize", "50585d75b494802d0151028115016902", "fcf1c3c7cd4491d801f1e1c78f100000", "default"},
    {"size_fdiag", "nwse-resize", "38c5dff7c7b8962045400281044508d2", "c7088f0f3e6c8088236ef8e1e3e70000", "default"}};

CursorThemePage::CursorThemePage(QWidget* parent) : QWidget(parent)
{
    init();
}

CursorThemePage::~CursorThemePage()
{
}

void CursorThemePage::init()
{
    initUI();
    loadCurosrThemes();
}

void CursorThemePage::initUI()
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setMargin(0);
    layout->setSpacing(10);

    QLabel* label_text = new QLabel(this);
    label_text->setText(tr("Cursor Themes Settings"));
    layout->addWidget(label_text);

    m_exclusionGroup = new ExclusionGroup(this);
    connect(m_exclusionGroup, &ExclusionGroup::currentItemChanged, this, &CursorThemePage::onCurrentItemChanged);

    QWidget* cursorsContainer = new QWidget(this);
    m_cursorVlayout = new QVBoxLayout(cursorsContainer);
    m_cursorVlayout->setSpacing(16);
    m_cursorVlayout->setMargin(0);
    layout->addWidget(cursorsContainer);

    layout->addStretch();
}

void CursorThemePage::loadCurosrThemes()
{
    auto themeInfos = AppearanceGlobalInfo::instance()->getAllThemes(APPEARANCE_THEME_TYPE_CURSOR);

    QString currentCursorTheme;
    AppearanceGlobalInfo::instance()->getTheme(APPEARANCE_THEME_TYPE_CURSOR, currentCursorTheme);

    for (auto theme : themeInfos)
    {
        const QString& cursorThemePath = theme.path;
        QDir cursorThemeDir(cursorThemePath);
        const QString& themeBaseName = cursorThemeDir.dirName();

        QList<QPixmap> themeCursors;
        for (auto cursors : cursorNames)
        {
            for (auto cursorName : cursors)
            {
                QFileInfo file(cursorThemePath + "/cursors/" + cursorName);
                if (file.exists())
                {
                    auto cursorImg = CursorImageLoader::getCursorImage(themeBaseName, cursorName, 22);
                    auto curosrPixmap = QPixmap::fromImage(cursorImg);
                    themeCursors << curosrPixmap;
                    break;
                }
            }
        }

        auto previewWidget = createPreviewWidget(theme.name, themeCursors, theme.name == currentCursorTheme);
        m_exclusionGroup->addExclusionItem(previewWidget);
        m_cursorVlayout->addWidget(previewWidget);
    }
}

ThemePreview* CursorThemePage::createPreviewWidget(const QString& themeName,
                                                   const QList<QPixmap> pixmaps,
                                                   bool selected)
{
    auto previewWidget = new ThemePreview(this);
    previewWidget->setPreviewFixedHeight(40);
    previewWidget->setSpacingAndMargin(24, QMargins(24, 0, 24, 0));
    previewWidget->setSelectedIndicatorEnable(true);
    previewWidget->setThemeInfo(themeName, themeName);
    previewWidget->setPreviewPixmaps(pixmaps, QSize(22, 22));
    previewWidget->setSelected(selected);

    connect(previewWidget, &ThemePreview::pressed, this, [this]()
            { emit requestReturn(); });
    return previewWidget;
}

void CursorThemePage::onCurrentItemChanged()
{
    auto current = m_exclusionGroup->getCurrent();
    auto id = current->getID();

    KLOG_INFO(qLcAppearance) << "cursor theme ui current changed:" << id;
    if (!AppearanceGlobalInfo::instance()->setTheme(APPEARANCE_THEME_TYPE_CURSOR, id))
    {
        KLOG_WARNING(qLcAppearance) << "set current cursor theme" << id << "failed!";
    }
    else
    {
        KLOG_INFO(qLcAppearance) << "cursor theme updated:" << id;
    }
}

void CursorThemePage::updateCurrentTheme(QString newCursorTheme)
{
    QSignalBlocker blocker(m_exclusionGroup);
    m_exclusionGroup->setCurrent(newCursorTheme);
}