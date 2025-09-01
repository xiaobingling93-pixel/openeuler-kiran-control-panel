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
#include "wireless-device-widget.h"
#include <wireless-network-manager.h>
#include <QBoxLayout>
#include <QEvent>
#include <QLabel>
#include <QTimer>
#include "kiran-push-button.h"
#include "logging-category.h"
#include "wireless-network-item.h"

using namespace NetworkManager;

namespace Kiran
{
namespace Network
{
WirelessDeviceWidget::WirelessDeviceWidget(WirelessDevice::Ptr devicePtr, QWidget *parent)
    : KiranCollapse(false, "", nullptr, parent),
      m_device(devicePtr)
{
    initUI();
    init();
}

WirelessDeviceWidget::~WirelessDeviceWidget()
{
}

QString WirelessDeviceWidget::uni() const
{
    return m_device->uni();
}

void WirelessDeviceWidget::initUI()
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
}

void WirelessDeviceWidget::init()
{
    if (!m_device || m_device->type() != Device::Wifi)
    {
        KLOG_WARNING(qLcNetwork) << "can't init wireless device widget, invalid device";
        return;
    }

    m_networkManager = new WirelessNetworkManager(m_device, this);

    connect(m_device.data(), &Device::activeConnectionChanged, this, &WirelessDeviceWidget::updateCurrentActiveConnection);
    connect(m_networkManager, &WirelessNetworkManager::networkAppeared, this, &WirelessDeviceWidget::processNetworkAppeared);
    connect(m_networkManager, &WirelessNetworkManager::networkDisappeared, this, &WirelessDeviceWidget::processNetworkDisappeared);

    // 加载所有已扫描出的无线网络
    auto networkList = m_networkManager->getNetworkInfoList();
    for (const auto &network : networkList)
    {
        processNetworkAppeared(network.ssid);
    }

    // 添加隐藏网络选项
    WirelessNetworkInfo hiddenNetworkInfo(tr("Connect to hidden network"), QString(), -1);
    insetNetworkItem(new WirelessNetworkItem(m_device, m_networkManager, hiddenNetworkInfo));

    // 更新当前激活连接标签显示
    updateCurrentActiveConnection();

    // 延迟展开
    QTimer::singleShot(0, [this]()
                       { KiranCollapse::setIsExpand(true); });
}

WirelessNetworkItem *WirelessDeviceWidget::findNetworkItem(const QString &ssid)
{
    for (int i = 0; i < m_contentLayout->count(); i++)
    {
        auto layoutItem = m_contentLayout->itemAt(i);
        auto layoutWidget = layoutItem->widget();
        if (!layoutWidget)
        {
            continue;
        }

        auto wiredSettingsItem = qobject_cast<WirelessNetworkItem *>(layoutWidget);
        if (!wiredSettingsItem)
        {
            continue;
        }

        if (wiredSettingsItem->ssid() == ssid)
        {
            return wiredSettingsItem;
        }
    }

    return nullptr;
}

void WirelessDeviceWidget::insetNetworkItem(WirelessNetworkItem *newOne)
{
    // 根据信号强度，寻找合适位置插入布局
    int insetPos = -1;
    for (int i = 0; i < m_contentLayout->count(); i++)
    {
        auto layoutItem = m_contentLayout->itemAt(i);
        auto widget = layoutItem->widget();
        if (!widget)
        {
            continue;
        }

        auto item = qobject_cast<WirelessNetworkItem *>(widget);
        if (!item)
        {
            continue;
        }

        if (newOne->signalStrength() > item->signalStrength())
        {
            insetPos = i;
            break;
        }
    }
    m_contentLayout->insertWidget(insetPos, newOne, 0);
}

void WirelessDeviceWidget::processNetworkAppeared(const QString &ssid)
{
    if (findNetworkItem(ssid))
    {
        KLOG_WARNING(qLcNetwork) << "wireless networ" << ssid << "appeared, but already exist";
        return;
    }
    insetNetworkItem(new WirelessNetworkItem(m_device, m_networkManager, m_networkManager->getNetworkInfo(ssid)));
}

void WirelessDeviceWidget::processNetworkDisappeared(const QString &ssid)
{
    auto item = findNetworkItem(ssid);
    if (!item)
    {
        KLOG_WARNING(qLcNetwork) << "wireless network" << ssid << "disappeared, is't exist";
        return;
    }

    m_contentLayout->removeWidget(item);
    item->deleteLater();
}

void WirelessDeviceWidget::updateCurrentActiveConnection()
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

}  // namespace Network
}  // namespace Kiran