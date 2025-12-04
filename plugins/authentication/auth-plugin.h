/**
 * Copyright (c) 2020 ~ 2023 KylinSec Co., Ltd.
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

#include <QObject>
#include "plugin-interface-v2.h"
#include "plugin-subitem-interface.h"

class KiranAuthDBusProxy;
class AuthPlugin : public QObject,
                   public KiranControlPanel::PluginInterfaceV2
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID KiranControlPanel_PluginInterfaceV2_iid)
    Q_INTERFACES(KiranControlPanel::PluginInterfaceV2)
public:
    AuthPlugin(QObject* parent = nullptr);
    ~AuthPlugin();

    int init(KiranControlPanel::PanelInterface* interface) override;
    void uninit() override;

    QVector<KiranControlPanel::SubItemPtr> getSubItems() override;

private:
    void initItems();

private:
    KiranControlPanel::PanelInterface* m_panelInterface = nullptr;
    QVector<KiranControlPanel::SubItemPtr> m_subitems;
};