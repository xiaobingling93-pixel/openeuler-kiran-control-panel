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

#include "upgrade-page.h"
#include <kiran-log/qt5-log-i.h>
#include <kiran-message-box.h>
#include <kiran-push-button.h>

#include <palette.h>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QDBusMetaType>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include "deps-dialog.h"
#include "history-dialog.h"
#include "logging-category.h"
#include "ui_upgrade-page.h"
#include "upgrade-interface.h"

using namespace Kiran::Theme;

#define NOTIFICATION_SERVICE "org.freedesktop.Notifications"
#define NOTIFICATION_PATH "/org/freedesktop/Notifications"
#define NOTIFICATION_INTERFACE "org.freedesktop.Notifications"

#define ACTION_BUTTON_TEXT_SCAN QObject::tr("Scan")
#define ACTION_BUTTON_TEXT_UPGRADE QObject::tr("Upgrade")
#define ACTION_BUTTON_TEXT_RETRY QObject::tr("Retry")

UpgradePage::UpgradePage(QWidget *parent)
    : QWidget(parent),
      ui(new Ui::UpgradePage),
      m_upgradeInterface(nullptr),
      m_depsDialog(nullptr),
      m_historyDialog(nullptr),
      m_upgradeStatus(UPGRADE_STATUS_LAST),
      m_reminderInterval(DEFAULT_REMINDER_INTERVAL)
{
    qRegisterMetaType<UpgradeHistory>("UpgradeHistory");
    qDBusRegisterMetaType<UpgradeHistory>();

    ui->setupUi(this);
    m_upgradeInterface = new UpgradeInterface(this);
    m_depsDialog = new DepsDialog(this);
    m_historyDialog = new HistoryDialog(this);

    initUI();

    connect(m_upgradeInterface, &UpgradeInterface::scanCompleted, this, &UpgradePage::handleScanCompleted);
    connect(m_upgradeInterface, &UpgradeInterface::solveDepsCompleted, this, &UpgradePage::handleSolveDepsCompleted);
    connect(m_upgradeInterface, &UpgradeInterface::upgradeCompleted, this, &UpgradePage::handleUpgradeCompleted);
    connect(m_upgradeInterface, &UpgradeInterface::upgradeActionChanged, this, &UpgradePage::updateUpgradeAction);
    connect(m_upgradeInterface, &UpgradeInterface::upgradePercentageChanged, this, &UpgradePage::upgradePercentage);
    connect(m_upgradeInterface, &UpgradeInterface::reminderIntervalChanged, this, &UpgradePage::updateReminderInterval);
    connect(m_upgradeInterface, &UpgradeInterface::upgradeHistoryAdded, this, &UpgradePage::prependHistoryToDialog);

    connect(m_depsDialog, &DepsDialog::confirmed, this, &UpgradePage::upgrade);
    connect(ui->table_pkgs, &PackageTable::packageSelectedChanged, this, &UpgradePage::updatePkgNumText);
}

UpgradePage::~UpgradePage()
{
    delete ui;
}

void UpgradePage::setUpgradeStatus(UpgradeStatus upgradeStatus)
{
    if (m_upgradeStatus != upgradeStatus)
    {
        m_upgradeStatus = upgradeStatus;
        updateUI(m_upgradeStatus);
    }
}

UpgradeStatus UpgradePage::getUpgradeStatus()
{
    return m_upgradeStatus;
}

void UpgradePage::initUI()
{
    KiranPushButton::setButtonType(ui->btn_action, KiranPushButton::BUTTON_Default);
    KiranPushButton::setButtonType(ui->btn_show_history, KiranPushButton::BUTTON_Default);
    ui->label_icon->setPixmap(QPixmap(":/kcp-upgrade/images/upgrade.svg"));
    ui->label_install_process->setText(tr("System updating"));

    //设置错误提示文字颜色
    QColor textColor = DEFAULT_PALETTE()->getBaseColors().widgetWarning;
    QPalette palette;
    palette.setColor(QPalette::WindowText, textColor);
    ui->label_error->setPalette(palette);

    // 默认隐藏升级布局
    ui->widget_upgrade->hide();

    // 初始化状态栏
    auto backendStatus = m_upgradeInterface->getBackendStatus();
    switch (backendStatus)
    {
    case BACKEND_STATUS_IDLE:
        setUpgradeStatus(UPGRADE_STATUS_IDLE);
        break;
    case BACKEND_STATUS_SCANNING:
        setUpgradeStatus(UPGRADE_STATUS_SCANNING);
        break;
    case BACKEND_STATUS_SOLVING_DEPS:
        setUpgradeStatus(UPGRADE_STATUS_SOLVING_DEPS);
        break;
    case BACKEND_STATUS_UPGRADING:
        setUpgradeStatus(UPGRADE_STATUS_UPGRADING);
        updateUpgradeLogFromJson(m_upgradeInterface->getUpgradeLog());
        break;
    default:
        break;
    }

    // 更新扫描时间
    updateLatestScanTime();

    //初始化设置栏
    ui->cb_reminder->addItem(tr("Never"), REMINDER_INTERVAL_NEVER);
    ui->cb_reminder->addItem(tr("Weekly"), REMINDER_INTERVAL_WEEKLY);
    ui->cb_reminder->addItem(tr("Monthly"), REMINDER_INTERVAL_MONTHLY);
    ui->cb_reminder->addItem(tr("Quarterly"), REMINDER_INTERVAL_QUARTERLY);
    m_reminderInterval = m_upgradeInterface->getReminderInterval();
    ui->cb_reminder->setCurrentIndex(ui->cb_reminder->findData(m_reminderInterval));

    connect(ui->btn_action, &QPushButton::clicked, this, &UpgradePage::handleActionClicked);
    connect(ui->cb_reminder, QOverload<int>::of(&QComboBox::activated), this, &UpgradePage::setReminderInterval);
    connect(ui->btn_show_history, &QPushButton::clicked, this, &UpgradePage::showHistoryDialog);
}

void UpgradePage::updateUI(UpgradeStatus status)
{
    switch (status)
    {
    case UPGRADE_STATUS_IDLE:
    {
        updateStatusText(tr("Please check for updates manually."), ACTION_BUTTON_TEXT_SCAN, true, STACKED_WIDGET_INDEX_LAST);
        break;
    }
    case UPGRADE_STATUS_SCANNING:
    {
        updateStatusText(tr("Scanning for updates..."), ACTION_BUTTON_TEXT_SCAN, false, STACKED_WIDGET_INDEX_LAST);
        break;
    }
    case UPGRADE_STATUS_SCAN_FAILED:
    {
        updateStatusText(tr("Scan failed"), ACTION_BUTTON_TEXT_RETRY, true, STACKED_WIDGET_INDEX_LAST);
        break;
    }
    case UPGRADE_STATUS_SCAN_SUCCESS_HAS_UPDATE:
    case UPGRADE_STATUS_SOLVING_DEPS_SUCCESS:
    case UPGRADE_STATUS_SOLVING_DEPS_FAILED:
    {
        updateStatusText(tr("System updates are available"), ACTION_BUTTON_TEXT_UPGRADE, true, STACKED_WIDGET_INDEX_PACKAGES);
        break;
    }
    case UPGRADE_STATUS_SCAN_SUCCESS_NO_UPDATE:
    {
        updateStatusText(tr("No updates are available"), ACTION_BUTTON_TEXT_SCAN, true, STACKED_WIDGET_INDEX_LAST);
        break;
    }
    case UPGRADE_STATUS_SOLVING_DEPS:
    {
        updateStatusText(tr("Solving dependencies..."), ACTION_BUTTON_TEXT_UPGRADE, false, STACKED_WIDGET_INDEX_PACKAGES);
        break;
    }
    case UPGRADE_STATUS_UPGRADING:
    {
        updateStatusText(tr("Upgrading system..."), ACTION_BUTTON_TEXT_UPGRADE, false, STACKED_WIDGET_INDEX_INSTALL_LOG);
        break;
    }
    case UPGRADE_STATUS_UPGRADE_SUCCESS:
    {
        updateStatusText(tr("Upgrade successfully"), ACTION_BUTTON_TEXT_SCAN, true, STACKED_WIDGET_INDEX_INSTALL_LOG);
        break;
    }
    case UPGRADE_STATUS_UPGRADE_FAILED:
    {
        updateStatusText(tr("Upgrade failed"), ACTION_BUTTON_TEXT_RETRY, true, STACKED_WIDGET_INDEX_INSTALL_LOG);
        break;
    }
    }
}

void UpgradePage::updateStatusText(const QString &summary, const QString &btnText, bool btnEnabled, StackedWidgetIndex index)
{
    ui->label_status->setText(summary);
    ui->btn_action->setText(btnText);
    ui->btn_action->setEnabled(btnEnabled);
    ui->stackedWidget->setCurrentIndex(index);
    ui->widget_upgrade->setVisible(index != STACKED_WIDGET_INDEX_LAST);
}

void UpgradePage::updateLatestScanTime()
{
    auto latestScanTime = m_upgradeInterface->getLatestScanTime();
    ui->label_time->setText(latestScanTime.isEmpty() ? tr("None") : latestScanTime);
}

void UpgradePage::updateUpgradeLogFromJson(const QString &upgradeLogJson)
{
    if (upgradeLogJson.isEmpty())
    {
        KLOG_WARNING(qLcUpgrade) << "Upgrade log json is empty";
        return;
    }
    KLOG_DEBUG(qLcUpgrade) << "Update upgrade log from json: " << upgradeLogJson;

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(upgradeLogJson.toUtf8(), &error);
    if (error.error == QJsonParseError::NoError && doc.isObject())
    {
        QJsonObject logObject = doc.object();

        // 更新百分比
        if (logObject.contains("percentage"))
        {
            upgradePercentage(logObject["percentage"].toInt());
        }

        // 更新操作日志
        if (logObject.contains("action"))
        {
            ui->text_install_log->clear();
            ui->text_install_log->append(logObject["action"].toString());
        }
    }
    else
    {
        KLOG_WARNING(qLcUpgrade) << "Failed to parse upgrade log json " << upgradeLogJson << error.errorString();
    }
}
void UpgradePage::updatePkgNumText(int selectedCount, int totalCount)
{
    ui->label_pkg_num->setText(tr("Selected %1/ Total %2").arg(selectedCount).arg(totalCount));
}

void UpgradePage::updateReminderInterval(int interval)
{
    if (interval == m_reminderInterval)
    {
        KLOG_INFO(qLcUpgrade) << "Reminder interval is already set to " << interval << ", no need to update.";
        return;
    }
    auto index = ui->cb_reminder->findData(interval);
    if (index < 0)
    {
        KLOG_WARNING(qLcUpgrade) << "Invalid reminder interval: " << interval << ", resetting to default: " << DEFAULT_REMINDER_INTERVAL;
        index = ui->cb_reminder->findData(DEFAULT_REMINDER_INTERVAL);
        m_reminderInterval = DEFAULT_REMINDER_INTERVAL;
    }
    else
    {
        m_reminderInterval = interval;
    }
    ui->cb_reminder->setCurrentIndex(index);
    KLOG_INFO(qLcUpgrade) << "Update reminder interval to " << m_reminderInterval << " successfully";
}
void UpgradePage::setReminderInterval(int index)
{
    auto reminderInterval = ui->cb_reminder->itemData(index).toInt();
    if (reminderInterval == m_reminderInterval)
    {
        KLOG_INFO(qLcUpgrade) << "Reminder interval is already set to " << reminderInterval;
        return;
    }
    QString errorMessage;
    if (!m_upgradeInterface->setReminderInterval(reminderInterval, errorMessage))
    {
        KiranMessageBox::message(nullptr,
                                 tr("Error"), errorMessage,
                                 KiranMessageBox::Ok);

        //恢复选中项为后台数据
        ui->cb_reminder->setCurrentIndex(ui->cb_reminder->findData(m_reminderInterval));
        return;
    }
    m_reminderInterval = reminderInterval;
    KLOG_INFO(qLcUpgrade) << "Set reminder interval successfully";
}

QStringList UpgradePage::getSelectedPkgIDs()
{
    QStringList pkgIDs;
    for (const auto &pkg : ui->table_pkgs->getSelectedUpgradePkgsInfo())
    {
        pkgIDs.append(pkg.id);
    }
    return pkgIDs;
}

QStringList UpgradePage::getPackageDeps(const QString &pkgDepsInfo)
{
    QJsonDocument doc = QJsonDocument::fromJson(pkgDepsInfo.toUtf8());
    QJsonObject obj = doc.object();
    QStringList requestPackages;
    QStringList dependencyPackages;
    for (const auto &value : obj.value("request_packages").toArray())
    {
        requestPackages.append(value.toString());
    }
    for (const auto &value : obj.value("dependency_packages").toArray())
    {
        dependencyPackages.append(value.toString());
    }

    if (m_selectedPkgIDs == requestPackages)
        return dependencyPackages;
    else
        return QStringList();
}

void UpgradePage::handleActionClicked()
{
    QString errorMessage;
    ui->label_error->clear();

    if (ui->btn_action->text() == ACTION_BUTTON_TEXT_SCAN)
    {
        setUpgradeStatus(UPGRADE_STATUS_SCANNING);
        //调用后端接口进行扫描
        if (!m_upgradeInterface->scan(errorMessage))
        {
            setUpgradeStatus(UPGRADE_STATUS_SCAN_FAILED);
            ui->label_error->setText(errorMessage);
            return;
        }
    }
    else if (ui->btn_action->text() == ACTION_BUTTON_TEXT_UPGRADE)
    {
        m_selectedPkgIDs = getSelectedPkgIDs();
        if (m_selectedPkgIDs.isEmpty())
        {
            KiranMessageBox::message(nullptr,
                                     tr("Warning"), tr("Please select at least one package"),
                                     KiranMessageBox::Ok);
            return;
        }
        setUpgradeStatus(UPGRADE_STATUS_SOLVING_DEPS);
        //调用后端接口进行依赖解析
        if (!m_upgradeInterface->solveDeps(m_selectedPkgIDs, errorMessage))
        {
            setUpgradeStatus(UPGRADE_STATUS_SOLVING_DEPS_FAILED);
            ui->label_error->setText(errorMessage);
            return;
        }
    }
    else if (ui->btn_action->text() == ACTION_BUTTON_TEXT_RETRY)
    {
        setUpgradeStatus(UPGRADE_STATUS_IDLE);
        ui->label_error->clear();
        return;
    }
}

void UpgradePage::showHistoryDialog()
{
    QString errorMessage;
    auto historyList = m_upgradeInterface->getUpgradeHistory(errorMessage);
    if (!errorMessage.isEmpty())
    {
        KLOG_WARNING(qLcUpgrade) << "Get upgrade history failed: " << errorMessage;
        KiranMessageBox::message(nullptr,
                                 tr("Error"), errorMessage,
                                 KiranMessageBox::Ok);
        return;
    }
    m_historyDialog->setUpgradeHistory(historyList);
    m_historyDialog->show();
    KLOG_DEBUG(qLcUpgrade) << "Show upgrade history dialog successfully."
                           << "Upgrade history count: " << historyList.size();
}

void UpgradePage::prependHistoryToDialog(const UpgradeHistory &history)
{
    // 仅在历史记录窗口显示时刷新窗口
    if (!m_historyDialog->isVisible())
    {
        return;
    }

    m_historyDialog->prependHistory(history);
    KLOG_DEBUG(qLcUpgrade) << "Prepend history to dialog successfully."
                           << history.upgradeTime
                           << static_cast<int>(history.result)
                           << history.errorMessage
                           << history.successPackages.join(",").trimmed()
                           << history.failedPackages.join(",").trimmed();
}
void UpgradePage::handleScanCompleted(bool success, const QString &errorMessage)
{
    //无论是否扫描成功，都更新最新扫描时间
    updateLatestScanTime();

    if (!errorMessage.isEmpty())
    {
        setUpgradeStatus(UPGRADE_STATUS_SCAN_FAILED);
        ui->label_error->setText(errorMessage);
        KLOG_WARNING(qLcUpgrade) << "Scan failed: " << errorMessage;
        return;
    }
    ui->label_error->clear();

    //获取可更新包信息
    auto upgradePkgsInfo = m_upgradeInterface->getUpgradePkgsInfo();
    if (upgradePkgsInfo.isEmpty())
    {
        setUpgradeStatus(UPGRADE_STATUS_SCAN_SUCCESS_NO_UPDATE);
        return;
    }
    KLOG_INFO(qLcUpgrade) << "Scan successfully";
    for (const auto &pkg : upgradePkgsInfo)
    {
        KLOG_INFO(qLcUpgrade) << "Upgrade available pkg: "
                              << pkg.id << ", "
                              << pkg.name << ", "
                              << pkg.currentVersion << ", "
                              << pkg.latestVersion << ", "
                              << pkg.kinds << ", "
                              << pkg.size;
    }

    //更新界面
    setUpgradeStatus(UPGRADE_STATUS_SCAN_SUCCESS_HAS_UPDATE);
    updatePkgNumText(0, upgradePkgsInfo.size());

    //插入表格数据
    ui->table_pkgs->setUpgradePkgsInfo(upgradePkgsInfo);
}

void UpgradePage::handleSolveDepsCompleted(bool success, const QString &pkgDepsInfo, const QString &errorMessage)
{
    if (!errorMessage.isEmpty())
    {
        setUpgradeStatus(UPGRADE_STATUS_SOLVING_DEPS_FAILED);
        KLOG_WARNING(qLcUpgrade) << "Solve deps failed: " << errorMessage;
        KiranMessageBox::message(nullptr,
                                 tr("Error"), errorMessage,
                                 KiranMessageBox::Ok);
        return;
    }

    //更新界面
    setUpgradeStatus(UPGRADE_STATUS_SOLVING_DEPS_SUCCESS);

    //弹出依赖列表窗口
    auto pkgDeps = getPackageDeps(pkgDepsInfo);
    m_depsDialog->clearDeps();
    m_depsDialog->setPkgDepsInfo(pkgDeps.join("\n"));
    m_depsDialog->show();
    KLOG_INFO(qLcUpgrade) << "Solve deps completed, pkg deps info: " << pkgDeps;
}

void UpgradePage::handleUpgradeCompleted(bool success, const QString &errorMessage)
{
    if (!errorMessage.isEmpty())
    {
        ui->text_install_log->append(tr("Upgrade failed: ") + errorMessage);
        setUpgradeStatus(UPGRADE_STATUS_UPGRADE_FAILED);
        sendUpgradeNotify(false, errorMessage);
        KLOG_WARNING(qLcUpgrade) << "Upgrade failed: " << errorMessage;
        return;
    }
    KLOG_INFO(qLcUpgrade) << "Upgrade completed, upgrade pkgs:" << m_selectedPkgIDs;

    //更新界面
    setUpgradeStatus(UPGRADE_STATUS_UPGRADE_SUCCESS);

    //记录本次升级情况
    ui->text_install_log->append(tr("Upgrade completed successfully"));

    // 发送升级成功消息弹窗
    sendUpgradeNotify(true, "");
}

void UpgradePage::updateUpgradeAction(const QString &action, const QString &actionHint)
{
    KLOG_DEBUG(qLcUpgrade) << "Update upgrade action: " << action << ", action hint: " << actionHint;
    ui->text_install_log->append(action + (actionHint.isEmpty() ? "" : ": " + actionHint));
}

void UpgradePage::upgradePercentage(uint percentage)
{
    KLOG_DEBUG(qLcUpgrade) << "Update upgrade percentage: " << percentage;
    ui->label_install_process->setText(tr("System updating (%1%)").arg(percentage));
}

void UpgradePage::upgrade()
{
    // 更新界面
    setUpgradeStatus(UPGRADE_STATUS_UPGRADING);
    ui->text_install_log->clear();

    //调用后端接口进行升级
    QString errorMessage;
    if (!m_upgradeInterface->upgrade(m_selectedPkgIDs, errorMessage))
    {
        ui->label_error->setText(errorMessage);
        //回退到扫描成功且有更新包状态
        setUpgradeStatus(UPGRADE_STATUS_SCAN_SUCCESS_HAS_UPDATE);
        return;
    }
}

void UpgradePage::sendUpgradeNotify(bool success, const QString &errorMessage)
{
    QDBusInterface notifyInterface(NOTIFICATION_SERVICE,
                                   NOTIFICATION_PATH,
                                   NOTIFICATION_INTERFACE,
                                   QDBusConnection::sessionBus());
    notifyInterface.setTimeout(500);
    QString title = success ? tr("Upgrade package successfully") : tr("Upgrade package failed");

    QVariantList args;
    args << "KiranControlPanel::Upgrade";  // 应用名称
    args << (uint)0;                       // 替换现有通知的ID（0表示新通知）
    args << "";                            // 图标路径（可选）
    args << title;                         // 标题
    args << errorMessage;                  // 内容
    args << QStringList();                 // 按钮列表（可选）
    args << QVariantMap();                 // 附加属性（如紧急程度）
    args << (int)10000;                    // 超时时间（毫秒）

    auto reply = notifyInterface.callWithArgumentList(QDBus::AutoDetect, "Notify", args);
    if (reply.type() == QDBusMessage::ErrorMessage)
    {
        KLOG_WARNING(qLcUpgrade) << "send notify failed," << reply.errorMessage();
    }
}