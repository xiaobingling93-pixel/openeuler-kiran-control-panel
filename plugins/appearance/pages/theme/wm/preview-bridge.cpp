/**
 * Copyright (c) 2020 ~ 2025 KylinSec Co., Ltd.
 * kwindecoration-preview is licensed under Mulan PSL v2.
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
#include "preview-bridge.h"
#include <KPluginMetaData>
#include <QDebug>
#include "preview-client.h"
#include "preview-item.h"
#include "preview-settings.h"

namespace Kiran
{
namespace Decoration
{
// see also: kwin previewbridge.cpp

PreviewBridge::PreviewBridge(QObject *parent)
    : DecorationBridge(parent), m_lastCreatedClient(nullptr), m_lastCreatedSettings(nullptr), m_valid(false)
{
    connect(this, &PreviewBridge::pluginChanged, this, &PreviewBridge::createFactory);
}

PreviewBridge::~PreviewBridge()
{
}

std::unique_ptr<DecoratedClientPrivate> PreviewBridge::createClient(DecoratedClient *client, Decoration *decoration)
{
    auto ptr = std::unique_ptr<PreviewClient>(new PreviewClient(client, decoration));
    m_lastCreatedClient = ptr.get();
    return ptr;
}

std::unique_ptr<DecorationSettingsPrivate> PreviewBridge::settings(DecorationSettings *parent)
{
    auto ptr = std::unique_ptr<PreviewSettings>(new PreviewSettings(parent));
    m_lastCreatedSettings = ptr.get();
    return ptr;
}

void PreviewBridge::registerPreviewItem(PreviewItem *item)
{
    m_previewItems.append(item);
}

void PreviewBridge::unregisterPreviewItem(PreviewItem *item)
{
    m_previewItems.removeAll(item);
}

void PreviewBridge::setPlugin(const QString &plugin)
{
    if (m_plugin == plugin)
    {
        return;
    }
    m_plugin = plugin;
    Q_EMIT pluginChanged();
}

QString PreviewBridge::theme() const
{
    return m_theme;
}

void PreviewBridge::setTheme(const QString &theme)
{
    if (m_theme == theme)
    {
        return;
    }
    m_theme = theme;
    Q_EMIT themeChanged();
}

QString PreviewBridge::plugin() const
{
    return m_plugin;
}

void PreviewBridge::createFactory()
{
    m_factory.clear();

    if (m_plugin.isNull())
    {
        setValid(false);
        return;
    }

    const auto offers = KPluginMetaData::findPlugins(DECORATION_PLUGIN_NAME);
    auto item = std::find_if(offers.constBegin(), offers.constEnd(), [this](const KPluginMetaData &plugin)
                             { return plugin.pluginId() == m_plugin; });
    if (item != offers.constEnd())
    {
        m_factory = KPluginFactory::loadFactory(*item).plugin;
    }

    setValid(!m_factory.isNull());
}

bool PreviewBridge::isValid() const
{
    return m_valid;
}

void PreviewBridge::setValid(bool valid)
{
    if (m_valid == valid)
    {
        return;
    }
    m_valid = valid;
    Q_EMIT validChanged();
}

Decoration *PreviewBridge::createDecoration(QObject *parent)
{
    if (!m_valid)
    {
        return nullptr;
    }
    QVariantMap args({{QStringLiteral("bridge"), QVariant::fromValue(this)}});
    if (!m_theme.isNull())
    {
        args.insert(QStringLiteral("theme"), m_theme);
    }
    return m_factory->create<KDecoration2::Decoration>(parent, QVariantList({args}));
}

DecorationButton *PreviewBridge::createButton(Decoration *decoration, DecorationButtonType type, QObject *parent)
{
    if (!m_valid)
    {
        return nullptr;
    }

    QVariantList args({QVariant::fromValue(type), QVariant::fromValue(decoration)});
    return m_factory->create<DecorationButton>(parent, args);
}
}  // namespace Decoration
}  // namespace Kiran