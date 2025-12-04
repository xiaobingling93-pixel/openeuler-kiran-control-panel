/**
 * Copyright (c) 2020 ~ 2021 KylinSec Co., Ltd.
 * kiran-control-panel is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 *
 * Author:     luoqing <luoqing@kylinsec.com.cn>
 */

#ifndef __DISPLAYPLUGIN_H__
#define __DISPLAYPLUGIN_H__

#include <QObject>
#include "panel-interface.h"
#include "plugin-interface-v2.h"
#include "plugin-subitem-interface.h"

class DisplayPlugin : public QObject,
                      public KiranControlPanel::PluginInterfaceV2
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID KiranControlPanel_PluginInterfaceV2_iid)
    Q_INTERFACES(KiranControlPanel::PluginInterfaceV2)

public:
    explicit DisplayPlugin(QObject *parent = nullptr);
    ~DisplayPlugin();

    // 主面板调用该接口初始化该插件，插件可在其中进行部分初始化操作，例如安装翻译等操作
    // 成功返回0
    int init(KiranControlPanel::PanelInterface* interface) override;

    // 主面板调用该接口取消掉该插件初始化做的操作并卸载该插件
    void uninit() override;

    // 功能项数组，生存周期由插件维护
    // 功能项发生变更时，应调用init时传入KcpInterface接口，通知主面板相关信息变更,及时加载新的功能项信息
    QVector<KiranControlPanel::SubItemPtr> getSubItems() override;

private:
    KiranControlPanel::SubItemPtr m_subitem;

};

#endif // DISPLAYPLUGIN_H
