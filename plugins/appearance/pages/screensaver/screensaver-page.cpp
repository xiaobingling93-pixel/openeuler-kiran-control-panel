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
#include "screensaver-page.h"
#include <kiran-switch-button.h>
#include <QFileInfo>
#include <QGSettings>
#include <QPair>
#include <QVector>
#include "auxiliary.h"
#include "ui_screensaver-page.h"

#define XEMBED_SCREENSAVER_PREFIX "xembed-"
#define XSCREENSAVER_PATH "/usr/libexec/xscreensaver/"

#define SCHEMA_KIRAN_SCREENSAVER "com.kylinsec.kiran.screensaver"
#define KEY_IDLE_ACTIVATION_SCREENSAVER "idleActivationScreensaver"
#define KEY_SCREENSAVER_THEME "screensaverTheme"

using namespace Kiran;

ScreensaverPage::ScreensaverPage(QWidget* parent)
    : QWidget(parent), ui(new Ui::ScreensaverPage)
{
    ui->setupUi(this);
    initUI();
    init();
}

ScreensaverPage::~ScreensaverPage()
{
}

QWidget* ScreensaverPage::createPage()
{
    return new ScreensaverPage();
}

void ScreensaverPage::initUI()
{
    m_idleActivteScreenSaverSwitch = new KiranSwitchButton(this);
    ui->layout_idleActivateScreenSaver->addWidget(m_idleActivteScreenSaverSwitch, 0);

    static const QVector<QPair<QString, QString> > InternalScreensaverTheme = {
        {QT_TR_NOOP("Classical"), "classical"},
        {QT_TR_NOOP("Atlantis"), "xembed-atlantis"},
        {QT_TR_NOOP("Binaryhorizon"), "xembed-binaryhorizon"},
        {QT_TR_NOOP("Epicycle"), "xembed-epicycle"},
        {QT_TR_NOOP("Rubik"), "xembed-rubik"}};
    auto iter = InternalScreensaverTheme.begin();
    while (iter != InternalScreensaverTheme.end())
    {
        if (checkThemeAvaiable(iter->second))
        {
            ui->combo_themes->addItem(tr(iter->first.toStdString().c_str()), iter->second);
        }
        iter++;
    }
}

void ScreensaverPage::init()
{
    m_gSettings = new QGSettings(SCHEMA_KIRAN_SCREENSAVER, "", this);
    connect(m_gSettings, &QGSettings::changed, this, &ScreensaverPage::processSettingsChanged);

    auto idleActivateScreenSaver = m_gSettings->get(KEY_IDLE_ACTIVATION_SCREENSAVER).toBool();
    m_idleActivteScreenSaverSwitch->setChecked(idleActivateScreenSaver);
    connect(m_idleActivteScreenSaverSwitch, &KiranSwitchButton::toggled, this, &ScreensaverPage::processSwitcherTrigger);

    auto theme = m_gSettings->get(KEY_SCREENSAVER_THEME).toString();
    auto idx = ui->combo_themes->findData(theme);
    if (idx == -1)
    {
        idx = 0;
    }
    ui->combo_themes->setCurrentIndex(idx);
    connect(ui->combo_themes, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ScreensaverPage::processcurrentThemeChanged);
}

// 动态检查xcreensaver 内嵌风格主题是否可用
bool ScreensaverPage::checkThemeAvaiable(const QString& themeID)
{
    if (!themeID.startsWith(XEMBED_SCREENSAVER_PREFIX))
    {
        return true;
    }

    auto xscreensaverName = themeID.mid(QString(XEMBED_SCREENSAVER_PREFIX).length());
    auto embedProgramPath = QString("%1/%2").arg(XSCREENSAVER_PATH, xscreensaverName);
    if (QFileInfo::exists(embedProgramPath))
    {
        return true;
    }

    return false;
}

void ScreensaverPage::processSwitcherTrigger()
{
    m_gSettings->set(KEY_IDLE_ACTIVATION_SCREENSAVER, m_idleActivteScreenSaverSwitch->isChecked());
}

void ScreensaverPage::processcurrentThemeChanged()
{
    m_gSettings->set(KEY_SCREENSAVER_THEME, ui->combo_themes->currentData().toString());
}

void ScreensaverPage::processSettingsChanged(const QString& key)
{
    switch (shash(key.toUtf8().data()))
    {
    case CONNECT(KEY_IDLE_ACTIVATION_SCREENSAVER,_hash):
    {
        QSignalBlocker blocker(m_idleActivteScreenSaverSwitch);
        m_idleActivteScreenSaverSwitch->setChecked(m_gSettings->get(KEY_IDLE_ACTIVATION_SCREENSAVER).toBool());
        break;
    }
    case CONNECT(KEY_SCREENSAVER_THEME,_hash):
    {
        QSignalBlocker blocker(ui->combo_themes);
        auto idx = ui->combo_themes->findData(m_gSettings->get(KEY_SCREENSAVER_THEME).toString());
        if( idx == -1 )
            idx = 0;
        ui->combo_themes->setCurrentIndex(idx);
        break;
    }
    default:
        break;
    };
}
