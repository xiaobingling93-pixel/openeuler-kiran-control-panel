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
#include "decoration-config.h"
#include <KCoreAddons/KPluginFactory>
#include <KCoreAddons/KPluginMetaData>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusInterface>
#include <QTimer>
#include "decoration-def.h"
#include "logging-category.h"

#define KWIN_CONFIG_FILE_NAME "kwinrc"
#define DEOCRATION_GROUP_NAME "org.kde.kdecoration2"
#define DECORATION_GROUP_KEY_PLUGIN "library"
#define DECORATION_GROUP_KEY_THEME "theme"

namespace Kiran
{
namespace Decoration
{
static bool isSupportThemes(const QVariantMap &decoSettingsMap)
{
    auto it = decoSettingsMap.find(QStringLiteral("themes"));
    if (it == decoSettingsMap.end())
    {
        return false;
    }
    return it.value().canConvert<bool>() && it.value().toBool();
}

static QList<ThemeEntry> getAvailableThemes()
{
    QList<ThemeEntry> result;
    static QSet<QString> blackList = {
        "org.kde.kwin.aurorae"  // 加载耗时过长，暂时过滤掉
    };

    const auto plugins = KPluginMetaData::findPlugins(DECORATION_PLUGIN_NAME);
    for (const auto &info : plugins)
    {
        const auto pluginRawMeta = info.rawData();
        if (!pluginRawMeta.contains(DECORATION_PLUGIN_NAME) ||
            pluginRawMeta.value(DECORATION_PLUGIN_NAME).isNull() ||
            pluginRawMeta.value(DECORATION_PLUGIN_NAME).isUndefined())
        {
            KLOG_DEBUG(qLcAppearance) << "plugin" << info.pluginId() << "does not support deco settings";
            continue;
        }

        if (blackList.contains(info.pluginId()))
        {
            KLOG_DEBUG(qLcAppearance) << "plugin" << info.pluginId() << "is in black list, skip";
            continue;
        }

        QScopedPointer<DecorationThemeProvider> themeFinder(KPluginFactory::instantiatePlugin<DecorationThemeProvider>(info).plugin);
        const auto decoSettingsMap = info.rawData().value(DECORATION_PLUGIN_NAME).toObject().toVariantMap();

        // 支持多主题的Decoration，枚举出所有主题
        // 不支持多主题的Decoration，使用默认主题
        if (themeFinder && isSupportThemes(decoSettingsMap))
        {
            auto themeMetaDataList = themeFinder->themes();
            for (const auto &theme : themeMetaDataList)
            {
                result << ThemeEntry{info.pluginId(), theme.themeName(), theme.visibleName()};
            }
        }
        else
        {
            const QString visibleName = info.name().isEmpty() ? info.pluginId() : info.name();
            result << ThemeEntry{info.pluginId(), QString(), visibleName};
        }
    }

    return result;
}

DecorationConfig::DecorationConfig(QObject *parent)
    : QObject(parent)
{
    // 扫描出所有可用的主题项
    m_availableThemes = getAvailableThemes();

    // 打开kwin配置文件，监听配置变化
    m_config = KSharedConfig::openConfig("kwinrc", KConfig::FullConfig);
    m_currentTheme = getConfigTheme();
    m_watcher = KConfigWatcher::create(m_config);
    connect(m_watcher.data(), &KConfigWatcher::configChanged, this, &DecorationConfig::onConfigChanged);
    
    // 设置定时器，避免频繁请求重新配置
    m_reconfigureTimer.setSingleShot(true);
    m_reconfigureTimer.setInterval(500);
    connect(&m_reconfigureTimer, &QTimer::timeout, this, &DecorationConfig::requestReconfigureInternal);
}

DecorationConfig::~DecorationConfig()
{
}

QList<ThemeEntry> DecorationConfig::availableThemes() const
{
    return m_availableThemes;
}

ThemeEntry DecorationConfig::currentTheme() const
{
    return m_currentTheme;
}

void DecorationConfig::setCurrentTheme(const ThemeEntry &config)
{
    m_config->group(DEOCRATION_GROUP_NAME).writeEntry(DECORATION_GROUP_KEY_PLUGIN, config.plugin, KConfig::Notify);
    m_config->group(DEOCRATION_GROUP_NAME).writeEntry(DECORATION_GROUP_KEY_THEME, config.theme, KConfig::Notify);
    if (m_config->isDirty())
    {
        KLOG_DEBUG(qLcAppearance) << "kwin config is dirty, sync";
        m_config->sync();
        // TODO: 是否提供确认框，让用户确认是否重新配置
        requestReconfigure();
    }
}

void DecorationConfig::setCurrentTheme(const QString &plugin, const QString &theme)
{
    setCurrentTheme(ThemeEntry{plugin, theme});
}

void DecorationConfig::onConfigChanged(const KConfigGroup &group, const QByteArrayList &names)
{
    if (group.name() == DEOCRATION_GROUP_NAME)
    {
        auto currentTheme = getConfigTheme();
        if (currentTheme != m_currentTheme)
        {
            KLOG_INFO(qLcAppearance) << "kwin config changed" << group.name() << names
                                     << ", update current decoration:"
                                     << currentTheme.plugin << "," << currentTheme.theme;
            setCurrentThemeInternal(currentTheme);
        }
    }
}

void DecorationConfig::setCurrentThemeInternal(const ThemeEntry &config)
{
    m_currentTheme = config;
    emit currentThemeChanged(config);
}

ThemeEntry DecorationConfig::getConfigTheme()
{
    QString plugin = m_config->group(DEOCRATION_GROUP_NAME).readEntry(DECORATION_GROUP_KEY_PLUGIN, QString());
    QString theme = m_config->group(DEOCRATION_GROUP_NAME).readEntry(DECORATION_GROUP_KEY_THEME, QString());

    // 从配置中读取主题配置，通过以上信息匹配到可见主题名
    QString visibleName;
    for (const auto &availableTheme : m_availableThemes)
    {
        if (availableTheme.plugin == plugin && availableTheme.theme == theme)
        {
            visibleName = availableTheme.visibleName;
            break;
        }
    }

    return ThemeEntry{plugin, theme, visibleName};
}

void DecorationConfig::requestReconfigure()
{
    m_reconfigureTimer.start();
}

void DecorationConfig::requestReconfigureInternal()
{
    auto sessionBus = QDBusConnection::sessionBus();
    auto sessionBusInterface = sessionBus.interface();

    if (!sessionBusInterface->isServiceRegistered("org.kde.KWin"))
    {
        KLOG_ERROR(qLcAppearance) << "kwin DBus service is not registered,"
                                  << "request reconfigure failed";
    }
    else
    {
        KLOG_INFO(qLcAppearance) << "request kwin reconfigure";
        QDBusInterface kwinInterface("org.kde.KWin", "/KWin", "org.kde.KWin");
        kwinInterface.call("reconfigure");
    }
}
}  // namespace Decoration
}  // namespace Kiran
