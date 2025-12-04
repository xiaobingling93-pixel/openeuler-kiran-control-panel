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
#include "wired-device-widget.h"
#include <kiran-push-button.h>
#include <kiran-switch-button.h>
#include <qt5-log-i.h>
#include <NetworkManagerQt/Connection>
#include <NetworkManagerQt/ConnectionSettings>
#include <NetworkManagerQt/Manager>
#include <NetworkManagerQt/Settings>
#include <NetworkManagerQt/WiredDevice>
#include <NetworkManagerQt/WiredSetting>
#include <QBoxLayout>
#include <QCollator>
#include <QLabel>
#include <QTimer>
#include <QWidget>
#include "connection-manager.h"
#include "logging-category.h"
#include "page-manager.h"
#include "wired-settings-item.h"

using namespace NetworkManager;

namespace Kiran
{
namespace Network
{
WiredDeviceWidget::WiredDeviceWidget(NetworkManager::WiredDevice::Ptr ptr, QWidget *parent)
    : KiranCollapse(false, "", nullptr, parent),
      m_device(ptr),
      m_uni(ptr->uni())
{
    initUI();
    init();
}

WiredDeviceWidget::~WiredDeviceWidget()
{
}

void WiredDeviceWidget::initUI()
{
    auto contentWidget = new QWidget(this);
    m_contentLayout = new QVBoxLayout(contentWidget);
    m_contentLayout->setContentsMargins(0, 0, 0, 0);
    m_contentLayout->setSpacing(0);
    addExpansionSpaceWidget(contentWidget);
    setTopBarFixedHeight(36);

    setTitle(tr("Network card: %1").arg(m_device->interfaceName()));

    m_activeConnectionLabel = new QLabel();
    m_activeConnectionLabel->setStyleSheet("color:#919191;font-family: \"Noto Sans CJK SC Light\";");
    addTopBarWidget(m_activeConnectionLabel);

    m_createConnectionButton = new KiranPushButton(contentWidget);
    KiranPushButton::setButtonType(m_createConnectionButton, KiranPushButton::BUTTON_Default);
    m_createConnectionButton->setIcon(QIcon(":/kcp-network/images/addition.svg"));
    m_createConnectionButton->setToolTip(tr("Create Connection"));
    m_contentLayout->addWidget(m_createConnectionButton);
    connect(m_createConnectionButton, &QPushButton::clicked, this, &WiredDeviceWidget::createConnection);
}

void WiredDeviceWidget::init()
{
    loadConnections();
    updateCurrentActiveConnection();

    //  监听连接配置动态变化
    connect(CM_INSTANCE, &ConnectionManager::wiredConnectionAdded, this, &WiredDeviceWidget::addConnectionItem);
    connect(CM_INSTANCE, &ConnectionManager::wiredConnectionSettingsChanged, this, &WiredDeviceWidget::processConnectionSettingsChanged);
    connect(CM_INSTANCE, &ConnectionManager::connectionRemoved, this, &WiredDeviceWidget::removeConnectionItem);
    connect(m_device.data(), &Device::activeConnectionChanged, this, &WiredDeviceWidget::updateCurrentActiveConnection);

    // 延迟展开
    QTimer::singleShot(0, [this]()
                       { KiranCollapse::setIsExpand(true); });
}

/// @brief 新增连接配置项
/// @param connectionPath
void WiredDeviceWidget::addConnectionItem(const QString &connectionPath)
{
    auto newConn = findConnection(connectionPath);
    if (!newConn)
    {
        KLOG_WARNING(qLcNetwork) << "process new connection failed, can't find conncection by"
                                 << connectionPath;
        return;
    }

    if (!CM_INSTANCE->checkConfigurationValidityForWiredDevice(connectionPath, m_device))
    {
        return;
    }

    if (findConnectionItem(connectionPath))
    {
        KLOG_WARNING(qLcNetwork) << "new connection already existing：" << connectionPath;
        return;
    }

    auto newConnName = newConn->name();

    // 通过ConnectionName选择合适位置插入
    int insetPos = -1;
    QCollator collator;
    collator.setNumericMode(true);
    for (int i = 0; i < m_contentLayout->count(); i++)
    {
        auto layoutItem = m_contentLayout->itemAt(i);
        auto layoutWidget = layoutItem->widget();
        if (!layoutWidget)
        {
            continue;
        }

        auto wiredSettingsItem = qobject_cast<WiredSettingsItem *>(layoutWidget);
        if (!wiredSettingsItem)
        {
            continue;
        }

        auto name = wiredSettingsItem->name();

        if (collator.compare(newConnName, name) < 0)
        {
            insetPos = i;
            break;
        }
    }

    // 确保连接配置布局位置在新增连接配置按钮之上
    if (insetPos == -1)
    {
        insetPos = m_contentLayout->indexOf(m_createConnectionButton);
    }

    KLOG_INFO(qLcNetwork) << "add connection item: " << newConn->name() << newConn->path();
    auto itemWidget = new WiredSettingsItem(m_device, newConn, this);
    m_contentLayout->insertWidget(insetPos, itemWidget, 0);
}

/// @brief 从界面端移除连接配置项
/// @param connectionPath
void WiredDeviceWidget::removeConnectionItem(const QString &connectionPath)
{
    auto connectionItem = findConnectionItem(connectionPath);
    if (!connectionItem)
    {
        return;
    }

    KLOG_INFO(qLcNetwork) << "remove connection item: " << connectionItem->name() << connectionItem->path();
    m_contentLayout->removeWidget(connectionItem);
    connectionItem->deleteLater();
}

/// @brief 连接配置变更需判断是否影响可用连接列表
/// @param connectionPath
void WiredDeviceWidget::processConnectionSettingsChanged(const QString &connectionPath)
{
    auto connection = findConnection(connectionPath);
    if (connection == nullptr || connection->settings() == nullptr)
    {
        return;
    }

    auto connectionSettings = connection->settings();
    if (connectionSettings->connectionType() != NetworkManager::ConnectionSettings::Wired)
    {
        return;
    }

    bool isAvailable = CM_INSTANCE->checkConfigurationValidityForWiredDevice(connectionPath, m_device);
    bool isExist = findConnectionItem(connectionPath);
    if (isAvailable && !isExist)
    {
        addConnectionItem(connectionPath);
    }
    else if (!isAvailable && isExist)
    {
        removeConnectionItem(connectionPath);
    }
}

void WiredDeviceWidget::createConnection()
{
    emit PM_INSTANCE->requestEditConnectionSettings(m_device->uni(), QString());
}

void WiredDeviceWidget::updateCurrentActiveConnection()
{
    auto connection = m_device->activeConnection();
    if (!connection)
    {
        m_activeConnectionLabel->setText("");
    }
    else
    {
        m_activeConnectionLabel->setText(connection->id());
    }
}

/// @brief 通过连接配置路径找到对应的连接配置项
/// @param connectionPath
/// @return
WiredSettingsItem *WiredDeviceWidget::findConnectionItem(const QString &connectionPath)
{
    for (int i = 0; i < m_contentLayout->count(); i++)
    {
        auto layoutItem = m_contentLayout->itemAt(i);
        auto layoutWidget = layoutItem->widget();
        if (!layoutWidget)
        {
            continue;
        }

        auto wiredSettingsItem = qobject_cast<WiredSettingsItem *>(layoutWidget);
        if (!wiredSettingsItem)
        {
            continue;
        }

        if (wiredSettingsItem->path() == connectionPath)
        {
            return wiredSettingsItem;
        }
    }

    return nullptr;
}

/// @brief 初始化加载该设备所有可用连接
void WiredDeviceWidget::loadConnections()
{
    if (!m_device || m_device->type() != Device::Ethernet)
    {
        KLOG_WARNING(qLcNetwork) << "can't load connections for devices"
                                 << (m_device ? m_device->interfaceName() : "nullptr");
        return;
    }

    auto connections = CM_INSTANCE->wiredDeviceAvaiableConnections(m_device);
    for (auto connection : connections)
    {
        addConnectionItem(connection->path());
    }
}

}  // namespace Network
}  // namespace Kiran