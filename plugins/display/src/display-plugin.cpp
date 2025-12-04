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
 * Author:     luoqing <luoqing@kylinsec.com.cn>
 */
#include "display-plugin.h"
#include <qt5-log-i.h>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <kiran-session-daemon/display-i.h>
#include "config.h"
#include "display-subitem.h"
#include "logging-category.h"

Q_LOGGING_CATEGORY(qLcDisplay,"kcp.display",QtMsgType::QtDebugMsg)

DisplayPlugin::DisplayPlugin(QObject *parent)
    : QObject{parent}
{

}

DisplayPlugin::~DisplayPlugin()
{

}

int DisplayPlugin::init(KiranControlPanel::PanelInterface *interface)
{
    if (!QDBusConnection::sessionBus().interface()->isServiceRegistered(DISPLAY_DBUS_NAME).value())
    {
        KLOG_INFO() << "Connect display dbus service failed!";
        return -1;
    }

    auto displaySubitem = new DisplaySubitem();
    m_subitem.reset(displaySubitem);
    return 0;
}

void DisplayPlugin::uninit()
{
}

QVector<KiranControlPanel::SubItemPtr> DisplayPlugin::getSubItems()
{
    return {m_subitem};
}
