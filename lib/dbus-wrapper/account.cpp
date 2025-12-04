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
#include "account.h"
#include "logging-category.h"

namespace DBusWrapper
{
namespace Account
{
QString serviceName()
{
    return QStringLiteral("com.kylinsec.Kiran.SystemDaemon.Accounts");
}

QString serviceObjectName()
{
    return QStringLiteral("/com/kylinsec/Kiran/SystemDaemon/Accounts");
}

bool isRegistered()
{
    return QDBusConnection::systemBus().interface()->isServiceRegistered(serviceName());
}

extern AccountInterfacePtr interface()
{
    if (!QDBusConnection::systemBus().interface()->isServiceRegistered(serviceName()))
    {
        KLOG_WARNING(qLcDbusWrapper) << "service:" << serviceName() << ",is not registered!";
    }

    return AccountInterfacePtr(new AccountInterface(serviceName(), serviceObjectName(), QDBusConnection::systemBus()));
}

AccountUserInterfacePtr userInterface(const QString& userObjectPath)
{
    if (!QDBusConnection::systemBus().interface()->isServiceRegistered(serviceName()))
    {
        KLOG_WARNING(qLcDbusWrapper) << "service:" << serviceName() << ",is not registered!";
    }

    return AccountUserInterfacePtr(new AccountUserInterface(serviceName(), userObjectPath, QDBusConnection::systemBus()));
}
}  // namespace Account
}  // namespace DBusWrapper