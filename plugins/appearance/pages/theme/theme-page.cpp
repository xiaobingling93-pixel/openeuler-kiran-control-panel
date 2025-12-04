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

#include "theme-page.h"
#include "appearance-global-info.h"
#include "cursor/cursor-theme-page.h"
#include "icon/icon-theme-page.h"
#include "setting-brief-widget/setting-brief-widget.h"
#include "theme-preview.h"
#include "ui_theme-page.h"
#include "wm/wm-theme-page.h"
#include "logging-category.h"

#include <kiran-log/qt5-log-i.h>
#include <kiran-session-daemon/appearance-i.h>
#include <kiranwidgets-qt5/kiran-message-box.h>
#include <QMouseEvent>
#include <QPushButton>

#define DARK_THEME "Kiran-dark"
#define LIGHT_THEME "Kiran-white"
#define THEME_AUTO_NAME "Kiran-auto"

#define SETTING_THEME_NUM 2
#define SETTING_THEME_PATH "/usr/share/themes/"

ThemePage::ThemePage(QWidget *parent)
    : QWidget(parent),
      ui(new Ui::ThemePage),
      m_iconThemePage(nullptr),
      m_cursorThemePage(nullptr)
{
    ui->setupUi(this);
    init();
}

ThemePage::~ThemePage()
{
    delete ui;
}

QSize ThemePage::sizeHint() const
{
    return QSize(670, 730);
}

QWidget *ThemePage::createPage()
{
    return new ThemePage();
}

void ThemePage::init()
{
    ui->stackedWidget->setCurrentIndex(0);
    ui->widget_effects->hide();

    m_uiThemeExclusionGroup = new ExclusionGroup(this);

    initUITheme();
    initIconTheme();
    initCursorTheme();
    initWindowTheme();

    connect(AppearanceGlobalInfo::instance(), &AppearanceGlobalInfo::AutoSwitchWindowThemeChanged, this, &ThemePage::onAutoSwitchWindowThemeChanged);
    connect(AppearanceGlobalInfo::instance(), &AppearanceGlobalInfo::themeChanged, this, &ThemePage::handleThemeChange);
}

bool ThemePage::initUITheme()
{
    m_enableAutoSwitchWindowTheme = AppearanceGlobalInfo::instance()->getAutoSwitchWindowTheme();
    createThemeWidget();
    handleThemeChange(APPEARANCE_THEME_TYPE_GTK);
    return true;
}

bool ThemePage::initIconTheme()
{
    // 创建图标选择控件
    m_chooseIconWidget = new SettingBriefWidget(tr("Choose icon Theme"));
    m_chooseIconWidget->setObjectName("chooseIconWidget");
    ui->verticalLayout_choose_widget->addWidget(m_chooseIconWidget);

    if (!AppearanceGlobalInfo::instance()->getTheme(APPEARANCE_THEME_TYPE_ICON, m_currIconTheme))
    {
        m_chooseIconWidget->setName(tr("Unknown"));
        return false;
    }

    m_chooseIconWidget->setName(iconThemeWhiteList.value(m_currIconTheme, m_currIconTheme));

    m_iconThemePage = new IconThemePage(ui->stackedWidget);
    ui->stackedWidget->addWidget(m_iconThemePage);

    connect(m_chooseIconWidget, &SettingBriefWidget::clicked, this,
            [this]
            {
                ui->stackedWidget->setCurrentWidget(m_iconThemePage);
            });

    connect(m_iconThemePage, &IconThemePage::requestReturn, this,
            [&, this]()
            {
                ui->stackedWidget->setCurrentIndex(0);
            });

    return true;
}

bool ThemePage::initCursorTheme()
{
    // 创建光标选择控件
    m_chooseCursorWidget = new SettingBriefWidget(tr("Choose cursor Themes"));
    m_chooseCursorWidget->setObjectName("chooseCursorWidget");
    ui->verticalLayout_choose_widget->addWidget(m_chooseCursorWidget);

    if (!AppearanceGlobalInfo::instance()->getTheme(APPEARANCE_THEME_TYPE_CURSOR, m_currCursorThemes))
    {
        m_chooseCursorWidget->setName(tr("Unknown"));
    }
    m_chooseCursorWidget->setName(m_currCursorThemes);

    m_cursorThemePage = new CursorThemePage(ui->stackedWidget);
    ui->stackedWidget->addWidget(m_cursorThemePage);

    connect(m_chooseCursorWidget, &SettingBriefWidget::clicked,
            [this]
            {
                ui->stackedWidget->setCurrentWidget(m_cursorThemePage);
            });

    connect(m_cursorThemePage, &CursorThemePage::requestReturn,
            [this]()
            {
                ui->stackedWidget->setCurrentIndex(0);
            });

    return true;
}

void ThemePage::initWindowTheme()
{
    m_wmThemePage = new WMThemePage(ui->stackedWidget);
    const QString currentTheme = m_wmThemePage->currentTheme();
    ui->stackedWidget->addWidget(m_wmThemePage);

    m_chooseWMThemeWidget = new SettingBriefWidget(tr("Choose window Themes"));
    m_chooseWMThemeWidget->setObjectName("chooseWindowThemeWidget");
    m_chooseWMThemeWidget->setName(currentTheme);
    ui->verticalLayout_choose_widget->addWidget(m_chooseWMThemeWidget);

    connect(m_chooseWMThemeWidget, &SettingBriefWidget::clicked, this,
            [this]
            {
                ui->stackedWidget->setCurrentWidget(m_wmThemePage);
            });

    connect(m_wmThemePage, &WMThemePage::requestReturn, this,
            [&, this]()
            {
                ui->stackedWidget->setCurrentIndex(0);
            });
    connect(m_wmThemePage, &WMThemePage::currentThemeChanged, this, [this]()
            {
                auto wmTheme = m_wmThemePage->currentTheme();
                KLOG_DEBUG(qLcAppearance) << "wm theme changed, update setting brief:" << wmTheme;
                m_chooseWMThemeWidget->setName(wmTheme);
            });
}

void ThemePage::createThemeWidget()
{
    struct UiThemeInfo
    {
        QString name;
        QString id;
        QString pixmap;
    };

    const QList<UiThemeInfo> uiThemes = {
        {tr("Light Theme"), LIGHT_THEME, ":/kcp-appearance/images/theme-light.png"},
        {tr("Auto"), THEME_AUTO_NAME, ":/kcp-appearance/images/theme-auto.png"},
        {tr("Dark Theme"), DARK_THEME, ":/kcp-appearance/images/theme-dark.png"}};

    for (int i = 0; i < uiThemes.count(); i++)
    {
        auto uiTheme = uiThemes.at(i);

        auto previewWidget = new ThemePreview(this);
        previewWidget->setPreviewFixedSize(QSize(140, 80));
        previewWidget->setSpacingAndMargin(0, QMargins(2, 2, 2, 2));
        previewWidget->setSelectedIndicatorEnable(false);
        previewWidget->setSelectedBorderWidth(2);
        previewWidget->setThemeInfo(uiTheme.name, uiTheme.id);

        QList<QPixmap> pixmaps;
        pixmaps << QPixmap(uiTheme.pixmap).scaled(QSize(136, 76), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        previewWidget->setPreviewPixmaps(pixmaps, QSize(136, 76));
        m_uiThemeExclusionGroup->addExclusionItem(previewWidget);

        ui->gridLayout_themes->addWidget(previewWidget, 0, i, Qt::AlignHCenter);
    }

    connect(m_uiThemeExclusionGroup, &ExclusionGroup::currentItemChanged, this, &ThemePage::onCurrentUiThemeChanged);
}

void ThemePage::handleThemeChange(int type)
{
    switch (type)
    {
    case APPEARANCE_THEME_TYPE_GTK:
    {
        QString themeName;
        if (m_enableAutoSwitchWindowTheme)
        {
            themeName = THEME_AUTO_NAME;
        }
        else
        {
            AppearanceGlobalInfo::instance()->getTheme(APPEARANCE_THEME_TYPE_GTK, themeName);
        }
        KLOG_INFO(qLcAppearance) << "handle backend theme changed:" << APPEARANCE_THEME_TYPE_GTK << themeName;
        QSignalBlocker blocker(m_uiThemeExclusionGroup);
        m_uiThemeExclusionGroup->setCurrent(themeName);
        break;
    }
    case APPEARANCE_THEME_TYPE_CURSOR:
    {
        QString cursorName;
        AppearanceGlobalInfo::instance()->getTheme(APPEARANCE_THEME_TYPE_CURSOR, cursorName);
        m_chooseCursorWidget->setName(cursorName);
        m_cursorThemePage->updateCurrentTheme(cursorName);
        break;
    }
    case APPEARANCE_THEME_TYPE_ICON:
    {
        QString iconName;
        AppearanceGlobalInfo::instance()->getTheme(APPEARANCE_THEME_TYPE_ICON, iconName);
        m_chooseIconWidget->setName(iconThemeWhiteList.value(iconName, iconName));
        m_iconThemePage->updateCurrentTheme(iconName);
        break;
    }
    case APPEARANCE_THEME_TYPE_METACITY:
    {
        // 已废弃，前端不再从后端获取窗口管理器Metacity主题。
        // 全面支持Kwin，前端内部自己实现Kwin窗口外边框主题管理。
        break;
    }
    default:
        break;
    }
}

void ThemePage::onAutoSwitchWindowThemeChanged(bool enable)
{
    if (m_enableAutoSwitchWindowTheme == enable)
    {
        return;
    }
    KLOG_INFO(qLcAppearance) << "handle auto switch window theme changed:" << enable;
    m_enableAutoSwitchWindowTheme = enable;
    handleThemeChange(APPEARANCE_THEME_TYPE_GTK);
}

void ThemePage::onCurrentUiThemeChanged()
{
    auto item = m_uiThemeExclusionGroup->getCurrent();
    QString currentID = item->getID();

    KLOG_INFO(qLcAppearance) << "meta theme changed:" << currentID;
    if (currentID == THEME_AUTO_NAME)
    {
        m_enableAutoSwitchWindowTheme = true;
        AppearanceGlobalInfo::instance()->enableAutoSwitchWindowTheme();
        KLOG_INFO(qLcAppearance) << "enable auto switch meta theme";
    }
    else
    {
        if (!AppearanceGlobalInfo::instance()->setTheme(APPEARANCE_THEME_TYPE_META, currentID))
        {
            KLOG_WARNING(qLcAppearance) << "set theme type:"
                                        << APPEARANCE_THEME_TYPE_META
                                        << " value:" << currentID << "failed";
        }
        else
        {
            KLOG_INFO(qLcAppearance) << "updated ui theme" << currentID;
        }
    }
}
