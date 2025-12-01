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
#include "preview-palette.h"
#include <KConfigCore/KConfigGroup>

namespace Kiran
{
namespace Decoration
{
PreviewPalette::PreviewPalette(const QString &colorScheme)
    : m_colorScheme(colorScheme != QStringLiteral("kdeglobals") ? colorScheme : QString())
{
    if (m_colorScheme.isEmpty())
    {
        m_colorSchemeConfig = KSharedConfig::openConfig(m_colorScheme, KConfig::FullConfig);
    }
    else
    {
        m_colorSchemeConfig = KSharedConfig::openConfig(m_colorScheme, KConfig::SimpleConfig);
    }
    m_watcher = KConfigWatcher::create(m_colorSchemeConfig);

    connect(m_watcher.data(), &KConfigWatcher::configChanged, this, &PreviewPalette::update);
    update();
}

bool PreviewPalette::isValid() const { return true; }

QColor PreviewPalette::color(ColorGroup group, ColorRole role) const
{
    if (m_legacyPalette)
    {
        switch (role)
        {
        case ColorRole::Frame:
            switch (group)
            {
            case ColorGroup::Active:
                return m_legacyPalette->activeFrameColor;
            case ColorGroup::Inactive:
                return m_legacyPalette->inactiveFrameColor;
            default:
                return QColor();
            }
        case ColorRole::TitleBar:
            switch (group)
            {
            case ColorGroup::Active:
                return m_legacyPalette->activeTitleBarColor;
            case ColorGroup::Inactive:
                return m_legacyPalette->inactiveTitleBarColor;
            default:
                return QColor();
            }
        case ColorRole::Foreground:
            switch (group)
            {
            case ColorGroup::Active:
                return m_legacyPalette->activeForegroundColor;
            case ColorGroup::Inactive:
                return m_legacyPalette->inactiveForegroundColor;
            case ColorGroup::Warning:
                return m_legacyPalette->warningForegroundColor;
            default:
                return QColor();
            }
        default:
            return QColor();
        }
    }

    switch (role)
    {
    case ColorRole::Frame:
        switch (group)
        {
        case ColorGroup::Active:
            return m_palette.active.background().color();
        case ColorGroup::Inactive:
            return m_palette.inactive.background().color();
        default:
            return QColor();
        }
    case ColorRole::TitleBar:
        switch (group)
        {
        case ColorGroup::Active:
            return m_palette.active.background().color();
        case ColorGroup::Inactive:
            return m_palette.inactive.background().color();
        default:
            return QColor();
        }
    case ColorRole::Foreground:
        switch (group)
        {
        case ColorGroup::Active:
            return m_palette.active.foreground().color();
        case ColorGroup::Inactive:
            return m_palette.inactive.foreground().color();
        case ColorGroup::Warning:
            return m_palette.inactive.foreground(KColorScheme::ForegroundRole::NegativeText)
                .color();
        default:
            return QColor();
        }
    default:
        return QColor();
    }
}

QPalette PreviewPalette::palette() const
{
    return m_legacyPalette ? m_legacyPalette->palette
                           : KColorScheme::createApplicationPalette(m_colorSchemeConfig);
}

void PreviewPalette::update()
{
    m_colorSchemeConfig->sync();

    if (KColorScheme::isColorSetSupported(m_colorSchemeConfig, KColorScheme::Header))
    {
        m_palette.active =
            KColorScheme(QPalette::Normal, KColorScheme::Header, m_colorSchemeConfig);
        m_palette.inactive =
            KColorScheme(QPalette::Inactive, KColorScheme::Header, m_colorSchemeConfig);
        m_legacyPalette.reset();
    }
    else
    {
        KConfigGroup wmConfig(m_colorSchemeConfig, QStringLiteral("WM"));

        if (!wmConfig.exists())
        {
            m_palette.active =
                KColorScheme(QPalette::Normal, KColorScheme::Window, m_colorSchemeConfig);
            m_palette.inactive =
                KColorScheme(QPalette::Inactive, KColorScheme::Window, m_colorSchemeConfig);
            m_legacyPalette.reset();
            return;
        }
        m_legacyPalette.reset(new LegacyPalette());
        m_legacyPalette->palette = KColorScheme::createApplicationPalette(m_colorSchemeConfig);
        m_legacyPalette->activeFrameColor = wmConfig.readEntry(
            "frame", m_legacyPalette->palette.color(QPalette::Active, QPalette::Window));
        m_legacyPalette->inactiveFrameColor =
            wmConfig.readEntry("inactiveFrame", m_legacyPalette->activeFrameColor);
        m_legacyPalette->activeTitleBarColor = wmConfig.readEntry(
            "activeBackground",
            m_legacyPalette->palette.color(QPalette::Active, QPalette::Highlight));
        m_legacyPalette->inactiveTitleBarColor =
            wmConfig.readEntry("inactiveBackground", m_legacyPalette->inactiveTitleBarColor);
        m_legacyPalette->activeForegroundColor = wmConfig.readEntry(
            "activeForeground",
            m_legacyPalette->palette.color(QPalette::Active, QPalette::HighlightedText));
        m_legacyPalette->inactiveForegroundColor = wmConfig.readEntry(
            "inactiveForeground", m_legacyPalette->activeForegroundColor.darker());

        KConfigGroup windowColorsConfig(m_colorSchemeConfig, QStringLiteral("Colors:Window"));
        m_legacyPalette->warningForegroundColor =
            windowColorsConfig.readEntry("ForegroundNegative", QColor(237, 21, 2));
    }

    Q_EMIT changed();
}
}  // namespace Decoration
}  // namespace Kiran