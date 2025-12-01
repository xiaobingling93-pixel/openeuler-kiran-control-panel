/**
 * Copyright (c) 2020 ~ 2025 KylinSec Co., Ltd.
 * kiran-control-panel is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 *
 * Author:     liuxinhao <liuxinhao@kylinsec.com.cn>
 */
#include "wm-theme-page.h"
#include <kiran-session-daemon/appearance-i.h>
#include <QDesktopServices>
#include <QDir>
#include <QElapsedTimer>
#include <QFileInfo>
#include <QProcess>
#include <QTimer>
#include "appearance-global-info.h"
#include "config.h"
#include "decoration-config.h"
#include "decoration-preview-widget.h"
#include "exclusion-group.h"
#include "theme-preview.h"
#include "ui_wm-theme-page.h"

#define WINDOW_PREVIEW_WIDTH 350
#define WINDOW_PREVIEW_HEIGHT 250

using namespace Kiran::Decoration;

WMThemePage::WMThemePage(QWidget* parent)
    : QWidget(parent),
      ui(new Ui::WMThemePage)
{
    ui->setupUi(this);
    init();
}

WMThemePage::~WMThemePage()
{
    delete ui;
}

QString WMThemePage::currentTheme() const
{
    auto currentTheme = m_decorationConfig->currentTheme();
    return currentTheme.visibleName;
}

ThemePreview* WMThemePage::createPreviewWidget(const Kiran::Decoration::ThemeEntry* themeConfig, bool selected)
{
    QElapsedTimer timer;
    timer.start();

    const QString& plugin = themeConfig->plugin;
    const QString& theme = themeConfig->theme;
    const QString& visibleName = themeConfig->visibleName;
    const QString themeID = QString("%1:%2").arg(plugin).arg(theme);
    const QMargins previewMargin(24 + 16, 0, 24, 0);  // 考虑右侧选中框，保证两边对齐

    // 创建主题预览窗口
    auto themePreview = new ThemePreview(this);
    themePreview->setSelectedIndicatorEnable(true);
    themePreview->setSelected(selected);
    themePreview->setSpacingAndMargin(0, previewMargin);
    themePreview->setThemeInfo(visibleName, themeID);

    // 创建装饰预览窗口
    const auto previewSize = QSize(WINDOW_PREVIEW_WIDTH, WINDOW_PREVIEW_HEIGHT);
    auto decorationPreviewWidget = new DecorationPreviewWidget(themePreview);
    decorationPreviewWidget->setFixedSize(previewSize);
    decorationPreviewWidget->setThemeData(plugin, theme, visibleName);
    themePreview->setPreviewWidget(decorationPreviewWidget);
    KLOG_DEBUG(qLcAppearance) << "preview" << plugin << "," << theme << "widget create time : " << timer.elapsed();

    connect(themePreview, &ThemePreview::pressed, this, [this]()
            { emit requestReturn(); });
    m_exclusionGroup->addExclusionItem(themePreview);

    return themePreview;
}

void WMThemePage::updateExclusionGroupCurrent(const QString& plugin, const QString& theme)
{
    m_exclusionGroup->setCurrent(QString("%1:%2").arg(plugin).arg(theme));
}

void WMThemePage::init()
{
    m_decorationConfig = new DecorationConfig(this);
    m_exclusionGroup = new ExclusionGroup(this);

    auto themes = m_decorationConfig->availableThemes();
    for (auto theme : themes)
    {
        auto themePreview = createPreviewWidget(&theme, false);
        ui->layout_scrollAreaWidgetContents->addWidget(themePreview);
    }
    
    auto currentTheme = m_decorationConfig->currentTheme();
    updateExclusionGroupCurrent(currentTheme.plugin, currentTheme.theme);

    connect(m_decorationConfig, &DecorationConfig::currentThemeChanged,
            this, [this](const ThemeEntry& theme)
            {
                updateExclusionGroupCurrent(theme.plugin, theme.theme);
                emit currentThemeChanged(theme.visibleName);
            });
    connect(m_exclusionGroup, &ExclusionGroup::currentItemChanged,
            this, &WMThemePage::exclusionGroupCurrentChanged);
}

void WMThemePage::exclusionGroupCurrentChanged()
{
    auto currentID = m_exclusionGroup->getCurrentID();
    if (currentID.isEmpty())
    {
        KLOG_WARNING(qLcAppearance) << "window decoration theme current is empty";
        return;
    }

    auto splits = currentID.split(":");
    if (splits.size() != 2)
    {
        KLOG_WARNING(qLcAppearance) << "window decoration theme current id format error:" << currentID;
        return;
    }

    auto plugin = splits.first();
    auto theme = splits.last();
    KLOG_DEBUG(qLcAppearance) << "window decoration theme current changed:" << plugin << "," << theme;
    m_decorationConfig->setCurrentTheme(plugin, theme);
}
