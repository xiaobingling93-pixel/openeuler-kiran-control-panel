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
#include "plugin-prefs.h"
#include <QGSettings>
#include "logging-category.h"
#include "qgsettings.h"
#include "qt5-log-i.h"

// 插件所需的配置此处不维护，只提供获取入口
#define SCHEMA_CONTROL_PANEL_PLUGIN "com.kylinsec.kiran.control-panel.plugin"
#define KEY_DISABLED_PLUGINS "disabledPlugins"

PluginPrefs::PluginPrefs()
{
}

PluginPrefs::~PluginPrefs()
{
    delete m_settings;
}

bool PluginPrefs::init()
{
    if (!QGSettings::isSchemaInstalled(SCHEMA_CONTROL_PANEL_PLUGIN))
    {
        KLOG_WARNING(qLcPluginFramework) << "schema " << SCHEMA_CONTROL_PANEL_PLUGIN << " is not installed!";
        return false;
    }

    m_settings = new QGSettings(SCHEMA_CONTROL_PANEL_PLUGIN);
    m_isInited = true;
    return true;
}

void PluginPrefs::dump()
{
    if (!m_isInited)
    {
        KLOG_WARNING(qLcPluginFramework) << "plugin prefs is not inited!";
        return;
    }

    KLOG_DEBUG(qLcPluginFramework) << "dump plugin prefs:";
    QStringList keys = m_settings->keys();
    for (const auto& key : keys)
    {
        KLOG_DEBUG(qLcPluginFramework) << "  " << key << ": " << m_settings->get(key);
    }
}

QStringList PluginPrefs::getDisabledPlugins() const
{
    if (!m_isInited)
    {
        KLOG_WARNING(qLcPluginFramework) << "plugin prefs is not inited!";
        return {};
    }

    return m_settings->get(KEY_DISABLED_PLUGINS).toStringList();
}

QVariant PluginPrefs::queryConfig(const QString& key, const QVariant& defaultVar) const
{
    if (!m_isInited)
    {
        KLOG_WARNING(qLcPluginFramework) << "plugin prefs is not inited!";
        return defaultVar;
    }

    if (!m_settings->keys().contains(key))
    {
        return defaultVar;
    }

    auto var = m_settings->get(key);
    if (var.isValid())
    {
        return var;
    }
    else
    {
        return defaultVar;
    }
};
