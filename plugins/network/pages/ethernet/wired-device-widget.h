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
#include <kiran-collapse.h>
#include <NetworkManagerQt/WiredDevice>
#include <QWidget>

class QVBoxLayout;
class QLabel;
class KiranPushButton;
namespace Kiran
{
namespace Network
{
class WiredSettingsItem;
class WiredDeviceWidget : public KiranCollapse
{
    Q_OBJECT
public:
    explicit WiredDeviceWidget(NetworkManager::WiredDevice::Ptr ptr,
                                  QWidget* parent = 0);
    ~WiredDeviceWidget();

    QString uni() const { return m_uni; };

private:
    void initUI();
    void init();

private slots:
    void loadConnections();
    void addConnectionItem(const QString& connectionPath);
    void removeConnectionItem(const QString& connectionPath);
    void processConnectionSettingsChanged(const QString& connectionPath);
    void createConnection();
    void updateCurrentActiveConnection();

private:
    WiredSettingsItem* findConnectionItem(const QString& connectionPath);

private:
    NetworkManager::WiredDevice::Ptr m_device;
    QString m_uni;
    QVBoxLayout* m_contentLayout = nullptr;
    QLabel* m_activeConnectionLabel = nullptr;
    KiranPushButton* m_createConnectionButton = nullptr;
};
}  // namespace Network
}  // namespace Kiran