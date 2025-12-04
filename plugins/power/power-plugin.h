/**
 * Copyright (c) 2020 ~ 2022 KylinSec Co., Ltd.
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
#ifndef __POWER_PLUGIN_H__
#define __POWER_PLUGIN_H__

#include <QObject>

#include "panel-interface.h"
#include "plugin-interface-v2.h"
#include "plugin-subitem-interface.h"

class PowerPlugin : public QObject,
                    public KiranControlPanel::PluginInterfaceV2
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID KiranControlPanel_PluginInterfaceV2_iid)
    Q_INTERFACES(KiranControlPanel::PluginInterfaceV2)
public:
    PowerPlugin(QObject* parent = nullptr);
    ~PowerPlugin();

    int init(KiranControlPanel::PanelInterface* interface) override;
    void initDesktopPower();
    void initServerPower();
    void uninit() override;

    QVector<KiranControlPanel::SubItemPtr> getSubItems() override;

private:
    KiranControlPanel::PanelInterface* m_panelInterface = nullptr;
    QVector<KiranControlPanel::SubItemPtr> m_subitems;
};

#endif