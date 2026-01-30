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
#include <QWidget>

namespace Ui
{
class UpgradePage;
}

//升级状态
enum UpgradeStatus
{
    //空闲
    UPGRADE_STATUS_IDLE = 0,
    //扫描
    UPGRADE_STATUS_SCANNING,
    UPGRADE_STATUS_SCAN_SUCCESS_HAS_UPDATE,
    UPGRADE_STATUS_SCAN_SUCCESS_NO_UPDATE,
    UPGRADE_STATUS_SCAN_FAILED,
    //解析依赖
    UPGRADE_STATUS_SOLVING_DEPS,
    UPGRADE_STATUS_SOLVING_DEPS_SUCCESS,
    UPGRADE_STATUS_SOLVING_DEPS_FAILED,
    //更新
    UPGRADE_STATUS_UPGRADING,
    UPGRADE_STATUS_UPGRADE_SUCCESS,
    UPGRADE_STATUS_UPGRADE_FAILED,
    UPGRADE_STATUS_LAST
};

enum StackedWidgetIndex
{
    STACKED_WIDGET_INDEX_PACKAGES,
    STACKED_WIDGET_INDEX_INSTALL_LOG,
    STACKED_WIDGET_INDEX_LAST
};

class UpgradeInterface;
class DepsDialog;
class HistoryDialog;
class UpgradePage : public QWidget
{
    Q_OBJECT

    Q_PROPERTY(UpgradeStatus upgrade_status READ getUpgradeStatus WRITE setUpgradeStatus)
public:
    explicit UpgradePage(QWidget *parent = nullptr);
    ~UpgradePage();

private slots:
    //处理UI交互槽函数
    void handleActionClicked();
    void showHistoryDialog();
    void setReminderInterval(int index);

    //处理操作完成槽函数
    void handleScanCompleted(bool success, const QString &errorMessage);
    void handleSolveDepsCompleted(bool success, const QString &pkgDepsInfo, const QString &errorMessage);
    void handleUpgradeCompleted(bool success, const QString &errorMessage);

    //更新日志和进度
    void updateUpgradeAction(const QString &action, const QString &actionHint);
    void upgradePercentage(uint percentage);

    // 刷新历史记录
    void prependHistoryToDialog(const UpgradeHistory &history);

private:
    //界面设置
    void initUI();
    void updateUI(UpgradeStatus status);

    void updateStatusText(const QString &summary, const QString &btnText, bool btnEnabled, StackedWidgetIndex index);
    void updateLatestScanTime();
    void updatePkgNumText(int selectedCount, int totalCount);
    void updateReminderInterval(int interval);
    // 解析并更新升级日志
    void updateUpgradeLogFromJson(const QString &upgradeLogJson);

    void setUpgradeStatus(UpgradeStatus upgradeStatus);
    UpgradeStatus getUpgradeStatus();

    //获取选中的软件包ID列表和依赖包ID列表
    QStringList getSelectedPkgIDs();
    QStringList getPackageDeps(const QString &pkgDepsInfo);

    //升级
    void upgrade();

    // 发送升级结果消息弹窗,以防在升级过程中切换tab，用户无法感知升级成功/失败
    void sendUpgradeNotify(bool success, const QString &errorMessage);

private:
    Ui::UpgradePage *ui;
    UpgradeInterface *m_upgradeInterface;
    DepsDialog *m_depsDialog;
    HistoryDialog *m_historyDialog;
    UpgradeStatus m_upgradeStatus;

    //保存用户选择更新的软件包ID列表
    QStringList m_selectedPkgIDs;

    int m_reminderInterval;
};