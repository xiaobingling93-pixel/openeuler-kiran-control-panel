/**
 * Copyright (c) 2020 ~ 2022 KylinSec Co., Ltd.
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

#include "power-plugin.h"
#include "config.h"
#include "dbus/power.h"
#include "pages/battery-settings-page.h"
#include "pages/general-settings-page.h"
#include "pages/power-settings-page.h"
#include "pages/server-general-settings.h"
#include "plugin-subitem.h"
#include "upower-interface.h"

#include <kiran-log/qt5-log-i.h>
#include <QCoreApplication>
#include <QTranslator>

PowerPlugin::PowerPlugin(QObject* parent)
    : QObject(parent)
{
}

PowerPlugin::~PowerPlugin()
{
}

int PowerPlugin::init(KiranControlPanel::PanelInterface* interface)
{
    PowerInterface::globalInit();

    auto serverMode = interface->queryCofnig("serverMode",false).toBool();
    if (serverMode)
    {
        initServerPower();
    }
    else
    {
        initDesktopPower();
    }

    return 0;
}

void PowerPlugin::initDesktopPower()
{
    auto generalSettingsSubItemCreater = []() -> QWidget*
    {
        return new GeneralSettingsPage();
    };
    auto powerSettingsSubItemCreator = []() -> QWidget*
    {
        return new PowerSettingsPage();
    };
    auto batterySettingsSubItemCreator = []() -> QWidget*
    {
        return new BatterySettingsPage();
    };

    auto generalSettings = new PluginSubItem("GeneralSettings",
                                             tr("General Settings"),
                                             "power-management",
                                             "",
                                             "ksvg-kcp-power-general-settings",
                                             3,
                                             generalSettingsSubItemCreater);
    auto generalSettingsSubItem = KiranControlPanel::SubItemPtr(generalSettings);

    auto powerSettings = new PluginSubItem("PowerSettings",
                                           tr("Power Settings"),
                                           "power-management",
                                           "",
                                           "ksvg-kcp-power-power-settings",
                                           2,
                                           powerSettingsSubItemCreator);
    auto powerSettingsSubItem = KiranControlPanel::SubItemPtr(powerSettings);

    m_subitems = {
        generalSettingsSubItem,
        powerSettingsSubItem};

    if (UPowerInterface::haveBattery())
    {
        auto batterySettings = new PluginSubItem("BatterySettings",
                                                 tr("Battery Settings"),
                                                 "power-management",
                                                 "",
                                                 "ksvg-kcp-power-battery-settings",
                                                 1,
                                                 batterySettingsSubItemCreator);
        m_subitems << KiranControlPanel::SubItemPtr(batterySettings);
    }
}

void PowerPlugin::initServerPower()
{
    // 服务器类型下，只提供关于空闲锁屏以及空闲时的设置
    auto serverGeneralSettingsCreater = []() -> QWidget*
    {
        return new ServerGeneralSettings();
    };
    auto serverGeneralSettings = new PluginSubItem("ServerGeneralSettings",
                                                   tr("General Settings"),
                                                   "power-management",
                                                   "",
                                                   "ksvg-kcp-power-general-settings",
                                                   1,
                                                   serverGeneralSettingsCreater);
    m_subitems = {KiranControlPanel::SubItemPtr(serverGeneralSettings)};
}

void PowerPlugin::uninit()
{
    PowerInterface::globalDeinit();
}

QVector<KiranControlPanel::SubItemPtr> PowerPlugin::getSubItems()
{
    return m_subitems;
}
