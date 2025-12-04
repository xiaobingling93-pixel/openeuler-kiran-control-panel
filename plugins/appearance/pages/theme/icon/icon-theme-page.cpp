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

#include "icon-theme-page.h"

#include <kiran-session-daemon/appearance-i.h>
#include <kiranwidgets-qt5/kiran-message-box.h>

#include <QDir>
#include <QIcon>
#include <QVBoxLayout>

#include "appearance-global-info.h"
#include "exclusion-group.h"
#include "theme-preview.h"
#include "logging-category.h"
#include "ui_icon-theme-page.h"

// clang-format off
// 通用图标主题展示图标
const QStringList IconThemePage::m_fallbackIcons = {"accessories-calculator",
                                              "preferences-system-notifications",
                                              "web-browser",
                                              "preferences-desktop-screensaver",
                                              "utilities-terminal",
                                              "user-info",
                                              "preferences-desktop-wallpaper"};
// 特殊主题展示图标
const QMap<QString,QStringList> IconThemePage::m_specifyIcons = {
                                {"Spring",
                                    {"kc-calculator",
                                     "smplayer",
                                     "fcitx",
                                     "engrampa",
                                     "utilities-terminal",
                                     "brasero",
                                     "org.gnome.Software"
                                     }
                                },
				{"Summer",
                                    {"kc-calculator",
                                     "smplayer",
                                     "fcitx",
                                     "engrampa",
                                     "utilities-terminal",
                                     "brasero",
                                     "org.gnome.Software"
                                     }
                                }};
// clang-format on

IconThemePage::IconThemePage(QWidget* parent)
    : QWidget(parent),
      ui(new Ui::IconThemePage)
{
    ui->setupUi(this);
    init();
}

IconThemePage::~IconThemePage()
{
    delete ui;
}

void IconThemePage::init()
{
    initUI();
}

void IconThemePage::initUI()
{
    m_exclusionGroup = new ExclusionGroup(this);
    connect(m_exclusionGroup, &ExclusionGroup::currentItemChanged, this, &IconThemePage::onCurrentItemChanged);
    loadIconThemes();
}

void IconThemePage::loadIconThemes()
{
    auto themeInfos = AppearanceGlobalInfo::instance()->getAllThemes(APPEARANCE_THEME_TYPE_ICON);

    QString currentIconTheme;
    AppearanceGlobalInfo::instance()->getTheme(APPEARANCE_THEME_TYPE_ICON, currentIconTheme);

    // 遍历主题列表
    for (auto theme : themeInfos)
    {
        // 过滤非白名单主题
        if (!iconThemeWhiteList.contains(theme.name))
        {
            continue;
        }

        // 根据主题名，拿到需要展示的图标名称（某些主题可能不全，特殊化处理）
        QStringList iconNames;
        if (m_specifyIcons.contains(theme.name))
        {
            iconNames = m_specifyIcons.value(theme.name);
        }
        else
        {
            iconNames = m_fallbackIcons;
        }

        // 拼凑关键目录绝对路径，拿到该目录下的条目列表
        QMap<QString, QStringList> dirAndEntrysMap = {
            {QString("%1/48x48/apps/").arg(theme.path), {}},
            {QString("%1/apps/scalable/").arg(theme.path), {}}};
        for (QMap<QString, QStringList>::iterator iter = dirAndEntrysMap.begin();
             iter != dirAndEntrysMap.end();
             ++iter)
        {
            QDir dir(iter.key());
            dirAndEntrysMap[iter.key()] = dir.entryList(QDir::Files);
        }

        // 遍历展示图标名称，匹配对应的图标
        QList<QPixmap> previewPixmaps;
        for (auto iconName : iconNames)
        {
            QString iconPath;
            for (QMap<QString, QStringList>::iterator iter = dirAndEntrysMap.begin();
                 iter != dirAndEntrysMap.end();
                 ++iter)
            {
                QStringList entryList = iter.value();

                // 模糊匹配图标名称
                auto idx = entryList.indexOf(QRegExp(QString("^%1.*").arg(iconName)));
                if (idx != -1)
                {
                    iconPath = iter.key() + entryList.at(idx);
                    break;
                }
            }

            QPixmap pixmap(iconPath);
            if (iconPath.isEmpty() || pixmap.isNull())
            {
                KLOG_WARNING(qLcAppearance) << "load icon theme" << theme.name
                                            << "can't find" << iconName
                                            << "in" << dirAndEntrysMap.keys();
                continue;
            }

            // 图片加载成功，加入展示图片列表
            previewPixmaps << pixmap.scaled(QSize(40, 40), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }

        // 构造展示主题预览控件
        auto previewWidget = createPreviewWidget(theme.name, previewPixmaps, currentIconTheme == theme.name);
        m_exclusionGroup->addExclusionItem(previewWidget);
        ui->iconVLayout->addWidget(previewWidget);
    }
}

ThemePreview* IconThemePage::createPreviewWidget(const QString& themeName,
                                                       const QList<QPixmap> pixmaps,
                                                       bool selected)
{
    auto previewWidget = new ThemePreview(this);
    previewWidget->setPreviewFixedHeight(70);
    previewWidget->setSpacingAndMargin(24, QMargins(24, 0, 24, 0));
    previewWidget->setSelectedIndicatorEnable(true);
    previewWidget->setSelected(selected);
    previewWidget->setThemeInfo(iconThemeWhiteList.value(themeName, themeName), themeName);
    previewWidget->setPreviewPixmaps(pixmaps, QSize(40, 40));
    connect(previewWidget, &ThemePreview::pressed, this, [this]() { emit requestReturn(); });

    return previewWidget;
}

void IconThemePage::updateCurrentTheme(QString newIconTheme)
{
    QSignalBlocker blocker(m_exclusionGroup);
    m_exclusionGroup->setCurrent(newIconTheme);
}

void IconThemePage::onCurrentItemChanged()
{
    auto current = m_exclusionGroup->getCurrent();
    auto id = current->getID();

    KLOG_INFO(qLcAppearance) << "icon theme ui current changed:" << id;
    if (!AppearanceGlobalInfo::instance()->setTheme(APPEARANCE_THEME_TYPE_ICON, id))
    {
        KLOG_WARNING(qLcAppearance) << "set currnet icon theme" << id << "failed!";
    }
    else
    {
        KLOG_INFO(qLcAppearance) << "icon theme updated:" << id;
    }
}
