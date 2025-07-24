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

#include "plugin-loader.h"
#include "config.h"
#include "logging-category.h"
#include "plugin-prefs.h"
#include "plugin-v1.h"
#include "plugin-v2.h"

#include <qt5-log-i.h>
#include <QDir>
#include <QGSettings>
#include <QScopedPointer>

QString getPluginIDFromFile(QString baseName)
{
    static const char* prefixs[] = {"kiran-cpnael-", "libkiran-cpanel-", NULL};
    for (int i = 0; prefixs[i]; i++)
    {
        if (baseName.startsWith(prefixs[i]))
        {
            return baseName.mid(strlen(prefixs[i]));
        }
    }
    return baseName;
}

QList<Plugin*> PluginLoader::loadAllPlugins(PluginPrefs* prefs)
{
    QStringList loadedPluginLibrarys;
    QList<Plugin*> plugins;

    auto disabledPlugins = prefs->getDisabledPlugins();

    // 老版本接口,通过插件desktop文件加载插件信息以及插件的共享库
    QDir desktopDir(PLUGIN_DESKTOP_DIR);
    QFileInfoList desktopFileInfoList = desktopDir.entryInfoList({"*.desktop"}, QDir::Files);
    for (auto desktopFileInfo : desktopFileInfoList)
    {
        QString pluginPath = desktopFileInfo.absoluteFilePath();
        QString desktopName = desktopFileInfo.baseName();
        if (disabledPlugins.contains(getPluginIDFromFile(desktopName)))
        {
            KLOG_INFO(qLcPluginFramework) << "plugin" << desktopName << "is disabled, skip load it.";
            continue;
        }

        QScopedPointer<PluginV1> pPlugin(new PluginV1());
        if (!pPlugin->load(pluginPath))
        {
            KLOG_WARNING(qLcPluginFramework) << "can't load plugin v1:" << pluginPath;
            continue;
        }

        QString libraryPath = pPlugin->getLibraryPath();
        loadedPluginLibrarys << libraryPath;

        KLOG_DEBUG(qLcPluginFramework) << "loaded plugin v1:" << libraryPath;
        plugins << pPlugin.take();
    }

    // 新版本接口，直接加载插件共享库
    QDir libraryDir(PLUGIN_LIBRARY_DIR);
    QFileInfoList libraryFileInfoList = libraryDir.entryInfoList({"*.so"}, QDir::Files);
    for (auto libraryFileInfo : libraryFileInfoList)
    {
        QString libraryPath = libraryFileInfo.absoluteFilePath();

        QString libraryName = libraryFileInfo.baseName();
        if (disabledPlugins.contains(getPluginIDFromFile(libraryName)))
        {
            KLOG_INFO(qLcPluginFramework) << "plugin" << libraryName << "is disabled, skip load it.";
            continue;
        }

        if (loadedPluginLibrarys.contains(libraryPath))
            continue;

        QScopedPointer<PluginV2> pPlugin(new PluginV2(prefs));
        if (!pPlugin->load(libraryPath))
        {
            KLOG_WARNING(qLcPluginFramework) << "can't load plugin v2:" << libraryPath;
            continue;
        }

        KLOG_DEBUG(qLcPluginFramework) << "loaded plugin v2:" << libraryPath;
        plugins << pPlugin.take();
    }

    return plugins;
}