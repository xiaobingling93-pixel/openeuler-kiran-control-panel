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
#include <KConfigCore/KConfigWatcher>
#include <KConfigCore/KSharedConfig>
#include <KConfigWidgets/KColorScheme>
#include <QObject>
#include <QPalette>
#include <memory>
#include "decoration-def.h"

namespace Kiran
{
namespace Decoration
{
struct LegacyPalette
{
    QPalette palette;

    QColor activeTitleBarColor;
    QColor inactiveTitleBarColor;

    QColor activeFrameColor;
    QColor inactiveFrameColor;

    QColor activeForegroundColor;
    QColor inactiveForegroundColor;
    QColor warningForegroundColor;
};

struct ModernPalette
{
    KColorScheme active;
    KColorScheme inactive;
};
/**
 * 获取KDE规范颜色定义
 * 给Client提供接口，用于给Decoration提供颜色方案
 * NOTE:
 * 1.Kiran目前的Decoration插件未使用该调色盘, 直接使用KiranStyle提供的共享库接口提取颜色
 *   see also: kiran-qt5-integration:plugins/kdecoration/decoration.cpp
 * 2.由于Kiran给Kwin的Patch修改，窗口管理器右键菜单也不会使用该调色盘
 *   see also: kwin:/src/useractions.cpp
 */
class PreviewPalette : public QObject
{
    Q_OBJECT
public:
    explicit PreviewPalette(const QString &colorScheme);

    bool isValid() const;

    QColor color(ColorGroup group, ColorRole role) const;
    QPalette palette() const;

Q_SIGNALS:
    void changed();

private:
    void update();
    QString m_colorScheme;
    KConfigWatcher::Ptr m_watcher;
    std::unique_ptr<LegacyPalette> m_legacyPalette;
    KSharedConfig::Ptr m_colorSchemeConfig;
    ModernPalette m_palette;
};
}  // namespace Decoration
}  // namespace Kiran