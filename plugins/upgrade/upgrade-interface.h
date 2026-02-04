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

#pragma once

#include <kiran-system-daemon/upgrade-i.h>
#include <QObject>
#include "def.h"

class KCDUpgradeProxy;
class UpgradeInterface : public QObject
{
    Q_OBJECT
public:
    UpgradeInterface(QObject* parent = nullptr);
    ~UpgradeInterface();

    //后端状态
    int getBackendStatus();

    //提醒周期
    int getReminderInterval();
    bool setReminderInterval(int interval, QString& errorMessage);

    //扫描
    bool scan(QString& errorMessage);
    QList<UpgradePkgInfo> getUpgradePkgsInfo();
    QString getLatestScanTime();

    //解析依赖
    bool solveDeps(const QStringList& pkgIDs, QString& errorMessage);

    //升级
    bool upgrade(const QStringList& pkgIDs, QString& errorMessage);
    QString getUpgradeLog();
    QString getLatestUpgradeTime();

    //获取升级历史记录
    QList<UpgradeHistory> getUpgradeHistory(QString& errorMessage);

signals:
    //操作结果
    void scanCompleted(bool success, const QString& errorMessage);
    void solveDepsCompleted(bool success, const QString& pkgDepsInfo, const QString& errorMessage);
    void upgradeCompleted(bool success, const QString& errorMessage);
    //属性变化
    void reminderIntervalChanged(int interval);
    void latestUpgradeTimeChanged(const QString& latestUpgradeTime);
    //更新进度
    void upgradePercentageChanged(uint percentage);
    void upgradeActionChanged(const QString& action, const QString& actionHint);

    //升级历史记录变化
    void upgradeHistoryAdded(const UpgradeHistory& history);

private slots:
    void handleDBusPropertyChanged(const QString& property, const QVariant& value);

private:
    void init();
    QString getAdvisoryKindsString(AdvisoryKindFlags kinds);

private:
    KCDUpgradeProxy* m_upgradeProxy;
};