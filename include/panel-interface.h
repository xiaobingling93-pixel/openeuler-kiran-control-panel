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

#pragma once

#include <QVariant>
#include <QString>

namespace KiranControlPanel
{
//控制面板接口，控制面板提供给插件使用的接口
class PanelInterface
{
public:
    // 通知控制中心主面板功能项信息变化
    virtual void handlePluginSubItemInfoChanged(const QString& subItemID) = 0;
    // 通知控制中心主面板 功能项 发生改变,调用该接口,控制中心将重新加载该插件下的功能项信息
    virtual void handlePluginSubItemChanged() = 0;
    // 从面板GSettings中获取插件内部相关的配置项
    virtual QVariant queryCofnig(const QString& key,const QVariant& default_var=QVariant()) = 0; 
};
}  // namespace KiranControlPanel
