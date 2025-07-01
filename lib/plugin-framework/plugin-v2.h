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
#include <qt5/QtCore/qvariant.h>
#include "plugin-interface-v2.h"
#include "plugin.h"

#include <QReadWriteLock>
#include <QVariant>
#include <QString>

class PluginPrefs;
class PluginV2 : public Plugin, public KiranControlPanel::PanelInterface
{
    Q_OBJECT
public:
    PluginV2(PluginPrefs* prefs = nullptr, QObject* parent = nullptr);
    ~PluginV2();

    /**
     * @brief 加载制定路径下的插件共享库
     * 
     * @param path 插件路径
     * @return 是否加载成功
     */
    bool load(const QString& path) override;

    /**
     * @brief 卸载插件
     * 
     */
    void unload() override;

    /**
     * @brief 获取功能项
     * 
     * @return 功能项数组 
     */
    QVector<KiranControlPanel::SubItemPtr> getSubItems() override;

    /**
     * @brief 插件调用，通知该插件包装向外发出信号，重新加载功能项信息, 更新搜索项
     * 
     * @param subItemID 功能项的ID
     */
    void handlePluginSubItemInfoChanged(const QString& subItemID) override;

    /**
     * @brief 插件调用，通知该插件包装向外发出信号，重新加载功能项列表, 更新二级分类
     *
     */
    void handlePluginSubItemChanged() override;

    /**
     * @brief 查询面板中相应插件配置中指定的键对应的值(GSettings)
     * 
     * @param key 插件配置项
     * @return 插件配置值
     */
    QVariant queryCofnig(const QString& key,const QVariant& defaultVar = QVariant()) override;
private:
    KiranControlPanel::PluginInterfaceV2* m_interfaceV2 = nullptr;
    QPluginLoader m_pluginLoader;
    PluginPrefs* m_prefs = nullptr;
};