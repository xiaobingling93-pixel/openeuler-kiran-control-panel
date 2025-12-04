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
#include "vpn-page.h"
#include <kiran-push-button.h>
#include <NetworkManagerQt/Settings>
#include <QProcess>
#include <QScrollBar>
#include "connection-manager.h"
#include "logging-category.h"
#include "ui_vpn-page.h"
#include "vpn-setting-item.h"

namespace Kiran
{
namespace Network
{
VpnPage::VpnPage(QWidget *parent)
    : Page(PAGE_VPN, parent),
      ui(new ::Ui::VpnPage)
{
    ui->setupUi(this);
    initUI();
    init();
}

VpnPage::~VpnPage()
{
    delete ui;
}

void VpnPage::reset()
{
    ui->scrollArea->verticalScrollBar()->setValue(0);
}

void VpnPage::initUI()
{
    KiranPushButton::setButtonType(ui->btn_create, KiranPushButton::BUTTON_Default);
    ui->btn_create->setIcon(QIcon(":/kcp-network/images/addition.svg"));
    ui->btn_create->setToolTip(tr("Create VPN Setting"));
    connect(ui->btn_create, &QPushButton::clicked, this, &VpnPage::createConnection);
}

void VpnPage::init()
{
    connect(CM_INSTANCE, &ConnectionManager::vpnConnectionAdded, this, &VpnPage::appendVPNConnection);
    connect(CM_INSTANCE, &ConnectionManager::connectionRemoved, this, &VpnPage::connectionRemoved);

    auto vpnConnections = CM_INSTANCE->availableConnections(ConnectionType::Vpn);
    for (auto connection : vpnConnections)
    {
        KLOG_INFO(qLcNetwork) << "load vpn connection path: " << connection->path();
        appendVPNConnection(connection->path());
    }
}

VpnSettingItem *VpnPage::findConnectionItemByPath(const QString &connectionPath)
{
    auto comparePathFunc = [this](VpnSettingItem *item, QString path)
    {
        return item->path() == path;
    };
    return findConnectionItem(comparePathFunc, connectionPath);
}

VpnSettingItem *VpnPage::findConnectionItemByUUID(const QString &uuid)
{
    auto compareUUIDFunc = [this](VpnSettingItem *item, QString userData)
    {
        return item->uuid() == userData;
    };
    return findConnectionItem(compareUUIDFunc, uuid);
}

VpnSettingItem *VpnPage::findConnectionItem(std::function<bool(VpnSettingItem *, QString)> func,
                                            const QString &userData)
{
    auto containerLayout = ui->layout_vpnContainer;

    for (int i = 0; i < containerLayout->count(); i++)
    {
        auto layoutItem = containerLayout->itemAt(i);
        auto layoutWidget = layoutItem->widget();
        if (!layoutWidget)
        {
            continue;
        }

        auto vpnSettingItem = qobject_cast<VpnSettingItem *>(layoutWidget);
        if (!vpnSettingItem)
        {
            continue;
        }

        if (func(vpnSettingItem, userData))
        {
            return vpnSettingItem;
        }
    }

    return nullptr;
}

void VpnPage::appendVPNConnection(const QString &connectionPath)
{
    auto connection = NetworkManager::findConnection(connectionPath);
    if (!connection)
    {
        return;
    }

    if (findConnectionItemByPath(connectionPath))
    {
        return;
    }

    auto vpnSettingItem = new VpnSettingItem(connection, this);
    ui->layout_vpnContainer->insertWidget(0, vpnSettingItem);
}

void VpnPage::connectionRemoved(const QString &connectionPath)
{
    auto vpnSettingItem = findConnectionItemByPath(connectionPath);
    if (vpnSettingItem)
    {
        ui->layout_vpnContainer->removeWidget(vpnSettingItem);
        vpnSettingItem->deleteLater();
    }
}

void VpnPage::createConnection()
{
    QProcess::startDetached("nm-connection-editor", {"-t", "vpn", "-c"});
}

}  // namespace Network
}  // namespace Kiran
