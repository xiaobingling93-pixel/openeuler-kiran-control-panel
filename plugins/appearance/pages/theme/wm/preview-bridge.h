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
#pragma once
#include <KCoreAddons/KPluginFactory>
#include <QObject>
#include "decoration-def.h"

namespace Kiran
{
namespace Decoration
{
class PreviewClient;
class PreviewSettings;
class PreviewItem;
class PreviewBridge : public DecorationBridge
{
    Q_OBJECT
    Q_PROPERTY(QString plugin READ plugin WRITE setPlugin NOTIFY pluginChanged)
    Q_PROPERTY(QString theme READ theme WRITE setTheme NOTIFY themeChanged)
    Q_PROPERTY(bool valid READ isValid NOTIFY validChanged)
public:
    explicit PreviewBridge(QObject *parent = nullptr);
    ~PreviewBridge() override;
    std::unique_ptr<DecoratedClientPrivate> createClient(DecoratedClient *client,
                                                         Decoration *decoration) override;
    std::unique_ptr<DecorationSettingsPrivate> settings(DecorationSettings *parent) override;

    PreviewClient *lastCreatedClient() { return m_lastCreatedClient; }
    PreviewSettings *lastCreatedSettings() { return m_lastCreatedSettings; }

    void registerPreviewItem(PreviewItem *item);
    void unregisterPreviewItem(PreviewItem *item);
    void setPlugin(const QString &plugin);
    QString plugin() const;
    void setTheme(const QString &theme);
    QString theme() const;
    bool isValid() const;

    Decoration *createDecoration(QObject *parent = nullptr);
    DecorationButton *createButton(Decoration *decoration,
                                   DecorationButtonType type,
                                   QObject *parent = nullptr);
signals:
    void pluginChanged();
    void themeChanged();
    void validChanged();

private:
    void createFactory();
    void setValid(bool valid);

private:
    PreviewClient *m_lastCreatedClient = nullptr;
    PreviewSettings *m_lastCreatedSettings = nullptr;
    QList<PreviewItem *> m_previewItems;
    QString m_plugin;
    QString m_theme;
    QPointer<KPluginFactory> m_factory;
    bool m_valid = false;
};
}  // namespace Decoration
}  // namespace Kiran