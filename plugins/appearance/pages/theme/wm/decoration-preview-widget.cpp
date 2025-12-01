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
#include "decoration-preview-widget.h"
#include <kiran-frame/kiran-frame.h>
#include <QBoxLayout>
#include <QDebug>
#include "logging-category.h"
#include "preview-bridge.h"
#include "preview-client.h"
#include "preview-item.h"
#include "preview-settings.h"

namespace Kiran
{
namespace Decoration
{
DecorationPreviewWidget::DecorationPreviewWidget(QWidget* parent)
    : ExclusionWidget(parent), m_bridge(new PreviewBridge(this))
{
    connect(m_bridge, &PreviewBridge::validChanged, this, &DecorationPreviewWidget::updatePreview);
}

DecorationPreviewWidget::~DecorationPreviewWidget()
{
    // 手动管理依赖析构顺序
    delete m_activePreview;
    delete m_inactivePreview;
    delete m_bridge;
}

void DecorationPreviewWidget::setThemeData(const QString& plugin, const QString& theme, const QString& visibleName)
{
    m_plugin = plugin;
    m_theme = theme;
    m_visibleName = visibleName;

    m_bridge->setTheme(theme);
    m_bridge->setPlugin(plugin);
}

void DecorationPreviewWidget::updatePreview()
{
    if (m_activePreview)
    {
        delete m_activePreview;
        m_activePreview = nullptr;
    }

    if (m_inactivePreview)
    {
        delete m_inactivePreview;
        m_inactivePreview = nullptr;
    }

    if (!m_bridge->isValid())
    {
        KLOG_WARNING(qLcAppearance) << "decoration preview plugin:" << m_bridge->plugin()
                                    << "theme:" << m_bridge->theme() << "is not valid";
        return;
    }

    m_activePreview = new PreviewItem(m_bridge, this);
    m_activePreview->setAnchorInfo(QRectF(0.08, 0.08, 0.8, 0.8));
    m_activePreview->client()->setActive(false);
    m_activePreview->client()->setCaption(m_visibleName);

    m_inactivePreview = new PreviewItem(m_bridge, this);
    m_inactivePreview->setAnchorInfo(QRectF(0.15, 0.15, 0.8, 0.8));
    m_inactivePreview->client()->setActive(true);
    m_inactivePreview->client()->setCaption(m_visibleName);
}
}  // namespace Decoration
}  // namespace Kiran