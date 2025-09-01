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
#pragma once
#include <NetworkManagerQt/WirelessDevice>
#include <kiran-collapse.h>

class QVBoxLayout;
class QLabel;
class QEvent;
namespace Kiran
{
class WirelessNetworkManager;
namespace Network
{
class WirelessNetworkItem;
class WirelessDeviceWidget : public KiranCollapse
{
    Q_OBJECT

public:
    explicit WirelessDeviceWidget(NetworkManager::WirelessDevice::Ptr device, QWidget* parent = 0);
    ~WirelessDeviceWidget();
    QString uni() const;

private:
    void initUI();
    void init();
    void insetNetworkItem(WirelessNetworkItem* item);
    WirelessNetworkItem* findNetworkItem(const QString& ssid);

private slots:
    void updateCurrentActiveConnection();
    void processNetworkAppeared(const QString& ssid);
    void processNetworkDisappeared(const QString& ssid);

private:
    NetworkManager::WirelessDevice::Ptr m_device;
    WirelessNetworkManager* m_networkManager = nullptr;
    QVBoxLayout* m_contentLayout = nullptr;
    QLabel* m_activeConnectionLabel = nullptr;
};
}  // namespace Network
}  // namespace Kiran