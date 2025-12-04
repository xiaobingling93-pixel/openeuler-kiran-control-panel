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
#include "appearance.h"
#include "logging-category.h"

QString DBusWrapper::Appearance::serviceName()
{
    return QStringLiteral("com.kylinsec.Kiran.SessionDaemon.Appearance");
}
QString DBusWrapper::Appearance::serviceObjectName()
{
    return QStringLiteral("/com/kylinsec/Kiran/SessionDaemon/Appearance");
}
bool DBusWrapper::Appearance::isRegistered()
{
    return QDBusConnection::sessionBus().interface()->isServiceRegistered(serviceName());
}
DBusWrapper::Appearance::AppearanceInterfacePtr DBusWrapper::Appearance::interface()
{
    if (!QDBusConnection::sessionBus().interface()->isServiceRegistered(serviceName()))
    {
        KLOG_WARNING(qLcDbusWrapper) << "service:" << serviceName() << ",is not registered!";
    }

    return DBusWrapper::Appearance::AppearanceInterfacePtr(new AppearanceInterface(serviceName(), serviceObjectName(), QDBusConnection::sessionBus()));
}