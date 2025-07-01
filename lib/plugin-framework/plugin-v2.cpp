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
#include "plugin-v2.h"
#include "logging-category.h"
#include "plugin-prefs.h"

#include <qt5-log-i.h>
#include <qvariant.h>
#include <QFileInfo>

#define RETURN_VAL_IF_IVALID(val)             \
    {                                         \
        if (!(isValid()))                     \
        {                                     \
            KLOG_DEBUG("plugin is invalid."); \
            return val;                       \
        }                                     \
    }

PluginV2::PluginV2(PluginPrefs* prefs, QObject* parent)
    : Plugin(parent),
      m_prefs(prefs)
{
    m_isValid = false;
}

PluginV2::~PluginV2()
{
}

bool PluginV2::load(const QString& path)
{
    if (isValid())
    {
        KLOG_WARNING(qLcPluginFramework) << "plugin is already loaded!,please unload first!";
        return false;
    }

    QFileInfo fileInfo(path);
    if (!fileInfo.exists())
    {
        KLOG_ERROR(qLcPluginFramework) << "can't load plugin," << path << ",file isn't exist!";
        return false;
    }

    m_pluginLoader.setFileName(path);
    if (!m_pluginLoader.load())
    {
        KLOG_ERROR(qLcPluginFramework) << "can't load plugin," << m_pluginLoader.errorString();
    }

    if (!m_pluginLoader.isLoaded())
    {
        KLOG_ERROR(qLcPluginFramework) << "can't load plugin,"
                                       << m_pluginLoader.errorString()
                                       << "," << path;
        return false;
    }

    KiranControlPanel::PluginInterfaceV2* pInterface = qobject_cast<KiranControlPanel::PluginInterfaceV2*>(m_pluginLoader.instance());
    if (!pInterface)
    {
        KLOG_ERROR(qLcPluginFramework) << "can't convert to plugin interface v2!" << m_pluginLoader.errorString();
        m_pluginLoader.unload();
        return false;
    }

    int iret = pInterface->init(this);
    if (iret != 0)
    {
        KLOG_ERROR(qLcPluginFramework) << "plugin init failed!" << path << "error code:" << iret;
        m_pluginLoader.unload();
        return false;
    }

    m_interfaceV2 = pInterface;
    m_isValid = true;
    return true;
}

void PluginV2::unload()
{
    if (!isValid())
    {
        return;
    }

    m_interfaceV2->uninit();
    m_pluginLoader.unload();
    m_isValid = false;
}

QVector<KiranControlPanel::SubItemPtr> PluginV2::getSubItems()
{
    RETURN_VAL_IF_IVALID(QVector<KiranControlPanel::SubItemPtr>{});
    return m_interfaceV2->getSubItems();
}

void PluginV2::handlePluginSubItemInfoChanged(const QString& subItemID)
{
    emit Plugin::subItemInfoChanged(subItemID);
}

void PluginV2::handlePluginSubItemChanged()
{
    emit Plugin::subItemChanged();
}

QVariant PluginV2::queryCofnig(const QString& key,const QVariant& defaultVar)
{
    return m_prefs->queryConfig(key,defaultVar);
}
