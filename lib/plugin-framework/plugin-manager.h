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

#include "plugin.h"

class PluginPrefs;
class PluginManager
{
public:
    static PluginManager* instance();
    ~PluginManager();

    /// @brief 插件管理初始化，加载所有插件信息
    bool init();
    void dump();

    /// @brief 获取插件列表
    /// @return 插件列表
    QList<Plugin*> getPlugins();

private:
    PluginManager() = default;

private:
    static PluginManager* _instance;
    PluginPrefs* m_pluginPrefs = nullptr;
    QList<Plugin*> m_plugins;
    bool m_isInited = false;
};