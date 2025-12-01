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
#include <QWidget>

namespace Ui
{
class WMThemePage;
}

namespace Kiran
{
namespace Decoration
{
class DecorationConfig;
struct ThemeEntry;
}
}

class ThemePreview;
class ExclusionGroup;
class QTimer;
class WMThemePage : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QString currentTheme READ currentTheme NOTIFY currentThemeChanged)

public:
    explicit WMThemePage(QWidget* parent = 0);
    ~WMThemePage();

    QString currentTheme() const;

signals:
    void requestReturn();
    void currentThemeChanged(const QString& theme);

private slots:
    void exclusionGroupCurrentChanged();

private:
    void init();
    ThemePreview* createPreviewWidget(const Kiran::Decoration::ThemeEntry* themeConfig,
                                      bool selected = false);
    void updateExclusionGroupCurrent(const QString& plugin, const QString& theme);

private:
    Ui::WMThemePage* ui;
    ExclusionGroup* m_exclusionGroup = nullptr;
    Kiran::Decoration::DecorationConfig* m_decorationConfig = nullptr;
};
