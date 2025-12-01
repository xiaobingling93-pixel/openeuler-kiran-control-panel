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
#pragma once
#include <KConfigCore/KConfigWatcher>
#include <KConfigCore/KSharedConfig>
#include <QObject>
#include <QTimer>

namespace Kiran
{
namespace Decoration
{
struct ThemeEntry
{
    QString plugin;
    QString theme;
    QString visibleName;
    bool operator==(const ThemeEntry &other) const
    {
        return plugin == other.plugin && theme == other.theme;
    }
    bool operator!=(const ThemeEntry &other) const
    {
        return !(*this == other);
    }
};
class DecorationConfig : public QObject
{
    Q_OBJECT
public:
    explicit DecorationConfig(QObject *parent = nullptr);
    ~DecorationConfig();

    QList<ThemeEntry> availableThemes() const;
    ThemeEntry currentTheme() const;
    void setCurrentTheme(const ThemeEntry &config);
    void setCurrentTheme(const QString &plugin, const QString &theme);
    void requestReconfigure();

signals:
    void currentThemeChanged(const ThemeEntry &config);

private slots:
    void onConfigChanged(const KConfigGroup &group, const QByteArrayList &names);
    ThemeEntry getConfigTheme();
    void setCurrentThemeInternal(const ThemeEntry &config);
    void requestReconfigureInternal();

private:
    KSharedConfig::Ptr m_config;
    KConfigWatcher::Ptr m_watcher;
    QTimer m_reconfigureTimer;
    ThemeEntry m_currentTheme;
    QList<ThemeEntry> m_availableThemes;
};
}  // namespace Decoration
}  // namespace Kiran
