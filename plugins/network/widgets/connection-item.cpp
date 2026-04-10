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
#include "connection-item.h"
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusPendingCallWatcher>
#include <QMouseEvent>
#include "logging-category.h"
#include "ui_connection-item.h"

#define NOTIFICATION_SERVICE "org.freedesktop.Notifications"
#define NOTIFICATION_PATH "/org/freedesktop/Notifications"
#define NOTIFICATION_INTERFACE "org.freedesktop.Notifications"

#define WATCHER_PROPERTY_PREFIX "_watcher_kiran_network"
#define WATCHER_PROPERTY_NAME WATCHER_PROPERTY_PREFIX "_name_"
#define WATCHER_PROPERTY_TYPE WATCHER_PROPERTY_PREFIX "_type_"

namespace Kiran
{
namespace Network
{
ConnectionItem::ConnectionItem(QWidget *parent)
    : KiranFrame(parent),
      ui(new Ui::ConnectionItem)
{
    ui->setupUi(this);
    initUI();
}

ConnectionItem::~ConnectionItem()
{
    delete ui;
}

void ConnectionItem::checkOpeartionResult(OpeartionType type, const QString &name, QDBusPendingCall &call)

{
    if (call.isFinished())
    {
        if (call.isError())
        {
            KLOG_WARNING(qLcNetwork) << type << name << "failed";
            dbusNotifyFailed(type, name);
            return;
        }
        else
        {
            KLOG_INFO(qLcNetwork) << type << name << "succeeded";
        }
    }
    else
    {
        auto watcher = new QDBusPendingCallWatcher(call, this);
        watcher->setProperty(WATCHER_PROPERTY_NAME, name);
        watcher->setProperty(WATCHER_PROPERTY_TYPE, type);
        connect(watcher, &QDBusPendingCallWatcher::finished, this, &ConnectionItem::processPendingCallFinished);
    }
}

void ConnectionItem::processPendingCallFinished(QDBusPendingCallWatcher *watcher)
{
    auto reply = watcher->reply();
    auto op = watcher->property(WATCHER_PROPERTY_TYPE).toInt();
    auto name = watcher->property(WATCHER_PROPERTY_NAME).toString();
    if (reply.type() == QDBusMessage::ErrorMessage)
    {
        KLOG_WARNING(qLcNetwork) << op << name << "failed";
        dbusNotifyFailed(static_cast<OpeartionType>(op), name);
    }
    else
    {
        KLOG_INFO(qLcNetwork) << op << name << "succeeded";
    }
    watcher->deleteLater();
}

void ConnectionItem::initUI()
{
    setFixedHeight(36);
    setDrawBroder(false);
    setAttribute(Qt::WA_Hover);

    ui->label_name->setElideMode(Qt::ElideRight);

    // 断开连接
    ui->btn_disconnect->setIcon(QIcon::fromTheme("ksvg-kcp-network-disconnect"));
    ui->btn_disconnect->setIconSize(QSize(18, 18));
    ui->btn_disconnect->setFlat(true);
    ui->btn_disconnect->setToolTip(tr("Disconnect"));
    connect(ui->btn_disconnect, &QPushButton::clicked, this, [this]()
            { emit ConnectionItem::requestDisconnect(QPrivateSignal()); });

    // 编辑连接
    ui->btn_edit->setIcon(QIcon::fromTheme("ksvg-kcp-edit-conn"));
    ui->btn_edit->setIconSize(QSize(18, 18));
    ui->btn_edit->setFlat(true);
    ui->btn_edit->setToolTip(tr("Edit"));
    connect(ui->btn_edit, &QPushButton::clicked, this, [this]()
            { emit ConnectionItem::requestEdit(QPrivateSignal()); });

    // 更多
    ui->btn_more->setIcon(QIcon::fromTheme("ksvg-kcp-more-options"));
    ui->btn_more->setIconSize(QSize(20, 4));
    ui->btn_more->setFlat(true);
    ui->btn_more->setStyleSheet("QPushButton::menu-indicator{image:none}");
    ui->btn_more->setToolTip(tr("More"));
}

void ConnectionItem::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        requestActive(QPrivateSignal());
    }
    return KiranFrame::mousePressEvent(event);
}

void ConnectionItem::setName(const QString &name)
{
    ui->label_name->setText(name);
}

void ConnectionItem::setTypeIconVisible(bool visible)
{
    ui->icon_type->setVisible(visible);
}

void ConnectionItem::setTypeIcon(const QString &iconName)
{
    ui->icon_type->setIcon(QIcon::fromTheme(iconName));
}

void ConnectionItem::setEditable(bool editable)
{
    ui->btn_edit->setVisible(editable);
}

void ConnectionItem::setDisconnectable(bool disconnectable)
{
    ui->btn_disconnect->setVisible(disconnectable);
}

void ConnectionItem::setMoreOptionsEnable(bool enable)
{
    ui->btn_more->setVisible(enable);
}

void ConnectionItem::setMoreOptions(QMenu *menu)
{
    ui->btn_more->setMenu(menu);
}

void ConnectionItem::updateStatus(bool isActivated, bool isLoading)
{
    ui->label_activated->setActivated(isActivated);
    ui->label_activated->setLoadingStatus(isLoading);
}

void ConnectionItem::dbusNotifyFailed(OpeartionType type, const QString &connectionName,const QString &resason)
{
    static QMap<OpeartionType, QString> typesTitleMap = {
        {OPERTION_ACTIVATE, QT_TR_NOOP("connection failure ")},
        {OPERTION_DEACTIVATE, QT_TR_NOOP("disconnect failure")}};
    static QMap<OpeartionType, QString> typesBodyMap = {
        {OPERTION_ACTIVATE, QT_TR_NOOP("Failed to connect to %1")},
        {OPERTION_DEACTIVATE, QT_TR_NOOP("Failed to disconnect %1")}};

    auto title = tr(typesTitleMap[type].toStdString().c_str());
    auto desc = tr(typesBodyMap[type].toStdString().c_str());
    desc = desc.arg(connectionName);

    QDBusInterface notifyInterface(NOTIFICATION_SERVICE,
                                   NOTIFICATION_PATH,
                                   NOTIFICATION_INTERFACE,
                                   QDBusConnection::sessionBus());
    notifyInterface.setTimeout(500);

    QVariantList args;
    args << "KiranControlPanel::Network";  // 应用名称
    args << (uint)0;                       // 替换现有通知的ID（0表示新通知）
    args << "";                            // 图标路径（可选）
    args << title;                         // 标题
    args << desc;                          // 内容
    args << QStringList();                 // 按钮列表（可选）
    args << QVariantMap();                 // 附加属性（如紧急程度）
    args << (int)2000;                     // 超时时间（毫秒）

    auto reply = notifyInterface.callWithArgumentList(QDBus::AutoDetect, "Notify", args);
    if (reply.type() == QDBusMessage::ErrorMessage)
    {
        KLOG_WARNING(qLcNetwork) << "send notify failed," << reply.errorMessage();
    }
}

}  // namespace Network
}  // namespace Kiran