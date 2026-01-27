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

#include "system-info-dbus.h"
#include <kiran-log/qt5-log-i.h>
#include <kiran-system-daemon/systeminfo-i.h>
#include <QDBusArgument>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusReply>
#include <QDBusSignature>
#include <QObject>
#include <QtDBus/QDBusReply>
#include <QtDBus/QtDBus>
#include <iostream>

#define SYSTEMINFO_DBUS_INTERFACE "com.kylinsec.Kiran.SystemDaemon.SystemInfo"
#define METHOD_GET_SYSTEMINFO "GetSystemInfo"
#define METHOD_SET_HOSTNAME "SetHostName"

#define TIMEOUT_MS 5000
#define TIMEOUT_MS_ONLINE 10000

bool SystemInfoDBus::getSystemInfo(SystemInfoType infoType, QString &info)
{
    QDBusMessage msgMethodCall = QDBusMessage::createMethodCall(SYSTEMINFO_DBUS_NAME,
                                                                SYSTEMINFO_OBJECT_PATH,
                                                                SYSTEMINFO_DBUS_INTERFACE,
                                                                METHOD_GET_SYSTEMINFO);
    msgMethodCall << (int)infoType;

    QDBusMessage msgReply = QDBusConnection::systemBus().call(msgMethodCall,
                                                              QDBus::Block,
                                                              TIMEOUT_MS);

    QString errorMsg;
    if (msgReply.type() == QDBusMessage::ReplyMessage)
    {
        QList<QVariant> args = msgReply.arguments();
        if (args.size() < 1)
        {
            errorMsg = "arguments size < 1";
            goto failed;
        }
        QVariant firstArg = args.takeFirst();
        info = firstArg.toString();
        return true;
    }
    else if (msgReply.type() == QDBusMessage::ErrorMessage)
    {
        errorMsg = "";
        goto failed;
    }
failed:
    KLOG_WARNING() << SYSTEMINFO_DBUS_NAME << METHOD_GET_SYSTEMINFO
                   << msgReply.errorName() << msgReply.errorMessage() << errorMsg;
    return false;
}

bool SystemInfoDBus::setHostName(QString name,QString& errorgMsg)
{
    QDBusMessage msgMethodCall = QDBusMessage::createMethodCall(SYSTEMINFO_DBUS_NAME,
                                                                SYSTEMINFO_OBJECT_PATH,
                                                                SYSTEMINFO_DBUS_INTERFACE,
                                                                METHOD_SET_HOSTNAME);

    msgMethodCall << name;

    QDBusMessage msgReply = QDBusConnection::systemBus().call(msgMethodCall);

    if (msgReply.type() == QDBusMessage::ReplyMessage)
    {
        return true;
    }
    else if (msgReply.type() == QDBusMessage::ErrorMessage)
    {
        errorgMsg = msgReply.errorMessage();
        goto failed;
    }

failed:
    KLOG_WARNING() << SYSTEMINFO_DBUS_NAME << METHOD_SET_HOSTNAME
                   << msgReply.errorName() << msgReply.errorMessage();
    return false;
}
