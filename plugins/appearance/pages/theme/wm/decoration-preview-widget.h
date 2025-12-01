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
#include <QWidget>
#include "exclusion-group.h"

namespace Kiran
{
namespace Decoration
{
class PreviewItem;
class PreviewBridge;
/**
 * 提供Decoration插件的预览
 * 传入Decoration插件的plugin和theme，进行预览
 * 会提供两种类型预览窗口，一种是活动窗口，一种是非活动窗口
 * 实现类似于kcm中kwindecoration的预览效果, 该类实现参考kwin:Themes.qml中GridView Delegate实现
 */
class DecorationPreviewWidget : public ExclusionWidget
{
    Q_OBJECT
public:
    explicit DecorationPreviewWidget(QWidget* parent = nullptr);
    virtual ~DecorationPreviewWidget();

    void setThemeData(const QString& plugin, const QString& theme, const QString& visibleName);

private slots:
    void updatePreview();

private:
    PreviewBridge* m_bridge = nullptr;
    QString m_plugin;
    QString m_theme;
    QString m_visibleName;
    PreviewItem* m_activePreview = nullptr;
    PreviewItem* m_inactivePreview = nullptr;
};
}  // namespace Decoration
}  // namespace Kiran