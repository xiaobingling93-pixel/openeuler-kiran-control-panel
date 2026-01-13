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
 * Author:     yuanxing <yuanxing@kylinsec.com.cn>
 */

#include "upgrade-plugin.h"
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QLocale>

#include <kiran-system-daemon/upgrade-i.h>

#include "logging-category.h"
#include "upgrade-subitem.h"

Q_LOGGING_CATEGORY(qLcUpgrade, "kcp.upgrade", QtMsgType::QtDebugMsg);

UpgradePlugin::UpgradePlugin(QObject *parent)
    : QObject(parent)
{
}

UpgradePlugin::~UpgradePlugin()
{
}

int UpgradePlugin::init(KiranControlPanel::PanelInterface *interface)
{
    if (!QDBusConnection::systemBus().interface()->isServiceRegistered(UPGRADE_DBUS_NAME))
    {
        KLOG_ERROR(qLcUpgrade) << "Upgrade service not registered";
        return -1;
    }

    m_subitem.reset(new UpgradeSubItem());

    return 0;
}

void UpgradePlugin::uninit()
{
}

QVector<KiranControlPanel::SubItemPtr> UpgradePlugin::getSubItems()
{
    return {m_subitem};
}
