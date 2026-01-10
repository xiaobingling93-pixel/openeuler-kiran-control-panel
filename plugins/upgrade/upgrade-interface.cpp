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

#include "upgrade-interface.h"
#include <kiran-log/qt5-log-i.h>
#include <qt5-log-i.h>
#include <QDBusConnection>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include "kcd_upgrade_proxy.h"
#include "logging-category.h"

UpgradeInterface::UpgradeInterface(QObject* parent)
    : QObject(parent),
      m_upgradeProxy(nullptr)
{
    init();
}

UpgradeInterface::~UpgradeInterface()
{
    if (m_upgradeProxy)
    {
        delete m_upgradeProxy;
        m_upgradeProxy = nullptr;
    }
}
void UpgradeInterface::init()
{
    m_upgradeProxy = new KCDUpgradeProxy(UPGRADE_DBUS_NAME, UPGRADE_OBJECT_PATH, QDBusConnection::systemBus(), this);
    if (!m_upgradeProxy->isValid())
    {
        KLOG_ERROR(qLcUpgrade) << "Failed to connect to upgrade service";
        return;
    }
    connect(m_upgradeProxy, &KCDUpgradeProxy::ScanCompleted, this, &UpgradeInterface::scanCompleted);
    connect(m_upgradeProxy, &KCDUpgradeProxy::SolveDepsCompleted, this, &UpgradeInterface::solveDepsCompleted);
    connect(m_upgradeProxy, &KCDUpgradeProxy::UpgradeCompleted, this, &UpgradeInterface::upgradeCompleted);
    connect(m_upgradeProxy, &KCDUpgradeProxy::UpgradePercentageChanged, this, &UpgradeInterface::upgradePercentageChanged);
    connect(m_upgradeProxy, &KCDUpgradeProxy::UpgradeActionChanged, this, &UpgradeInterface::upgradeActionChanged);

    connect(m_upgradeProxy, &KCDUpgradeProxy::dbusPropertyChanged, this, &UpgradeInterface::handleDBusPropertyChanged);
}

int UpgradeInterface::getBackendStatus()
{
    return m_upgradeProxy->backend_status();
}

int UpgradeInterface::getReminderInterval()
{
    return m_upgradeProxy->reminder_interval();
}

bool UpgradeInterface::setReminderInterval(int interval, QString& errorMessage)
{
    auto reply = m_upgradeProxy->SetReminderInterval(interval);
    reply.waitForFinished();
    if (reply.isError())
    {
        KLOG_ERROR(qLcUpgrade) << "Failed to set reminder interval: " << reply.error().message();
        errorMessage = reply.error().message();
        return false;
    }
    KLOG_INFO(qLcUpgrade) << "Set reminder interval to " << interval;
    return true;
}

bool UpgradeInterface::scan(QString& errorMessage)
{
    auto reply = m_upgradeProxy->Scan();
    reply.waitForFinished();
    if (reply.isError())
    {
        KLOG_ERROR(qLcUpgrade) << "Failed to scan: " << reply.error().message();
        errorMessage = reply.error().message();
        return false;
    }
    KLOG_INFO(qLcUpgrade) << "Begin to scan system upgrade pkgs";
    return true;
}

QList<UpgradePkgInfo> UpgradeInterface::getUpgradePkgsInfo()
{
    auto reply = m_upgradeProxy->GetUpgradePkgsInfo();
    reply.waitForFinished();
    if (reply.isError())
    {
        KLOG_ERROR(qLcUpgrade) << "Failed to get upgrade pkgs info: " << reply.error().message();
        return QList<UpgradePkgInfo>();
    }
    auto result = reply.value();
    QJsonDocument doc = QJsonDocument::fromJson(result.toUtf8());
    QJsonArray array = doc.array();
    QList<UpgradePkgInfo> pkgs;
    for (const auto& item : array)
    {
        QJsonObject obj = item.toObject();
        UpgradePkgInfo pkg;
        pkg.selected = false;
        pkg.id = obj["id"].toString();
        pkg.name = obj["name"].toString();
        pkg.currentVersion = obj["current_version"].toString();
        pkg.latestVersion = obj["latest_version"].toString();
        pkg.size = obj["size"].toString();
        if (obj.contains("advisory_info"))
        {
            QJsonObject advisoryInfo = obj["advisory_info"].toObject();
            int advisoryKindValue = advisoryInfo["advisory_kind"].toInt();
            pkg.kinds = getAdvisoryKindsString(static_cast<AdvisoryKindFlags>(advisoryKindValue));
        }
        else
        {
            pkg.kinds = getAdvisoryKindsString(ADVISORY_KIND_UNKNOWN);
        }

        pkgs.append(pkg);
    }
    return pkgs;
}

QString UpgradeInterface::getLatestScanTime()
{
    return m_upgradeProxy->latest_scan_time();
}

bool UpgradeInterface::solveDeps(const QStringList& pkgIDs, QString& errorMessage)
{
    auto reply = m_upgradeProxy->SolveDeps(pkgIDs);
    reply.waitForFinished();
    if (reply.isError())
    {
        KLOG_ERROR(qLcUpgrade) << "Failed to solve deps: " << reply.error().message();
        errorMessage = reply.error().message();
        return false;
    }
    KLOG_INFO(qLcUpgrade) << "Begin to solve deps pkgs:" << pkgIDs;
    return true;
}

bool UpgradeInterface::upgrade(const QStringList& pkgIDs, QString& errorMessage)
{
    auto reply = m_upgradeProxy->Upgrade(pkgIDs);
    reply.waitForFinished();
    if (reply.isError())
    {
        KLOG_ERROR(qLcUpgrade) << "Failed to upgrade: " << reply.error().message();
        errorMessage = reply.error().message();
        return false;
    }
    KLOG_INFO(qLcUpgrade) << "Begin to upgrade pkgs:" << pkgIDs;
    return true;
}

QString UpgradeInterface::getUpgradeLog()
{
    return m_upgradeProxy->GetUpgradeLog();
}

QString UpgradeInterface::getLatestUpgradeTime()
{
    return m_upgradeProxy->latest_upgrade_time();
}

QString UpgradeInterface::getAdvisoryKindsString(AdvisoryKindFlags kinds)
{
    QString kindsString;
    auto flagToStringMap = AdvisoryKindHelper::getFlagToStringMap();
    for (auto it = flagToStringMap.begin(); it != flagToStringMap.end(); ++it)
    {
        if (kinds & it.key())
            kindsString.append(QCoreApplication::translate("AdvisoryKindHelper", it.value().toLatin1().data()) + "\n");
    }
    //去掉结尾的回车
    return kindsString.trimmed();
}

void UpgradeInterface::handleDBusPropertyChanged(const QString& property, const QVariant& value)
{
    KLOG_DEBUG(qLcUpgrade) << "DBus property changed: " << property << " to " << value.toString();
    if (property == "reminder_interval")
    {
        emit reminderIntervalChanged(value.toInt());
    }
}