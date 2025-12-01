/**
 * Copyright (c) 2020 ~ 2023 KylinSec Co., Ltd.
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
#include "theme-preview.h"
#include <kiran-frame/kiran-frame.h>
#include <QBoxLayout>
#include <QEvent>
#include <QLabel>
#include <QVariant>

static const char* theme_preview_tag_property = "_theme_preview_flag_";
static const char* selected_indicator_pixmap = ":/kcp-appearance/images/indicator-selected.png";

ThemePreview::ThemePreview(QWidget* parent)
    : ExclusionWidget(parent)
{
    initUI();
}

ThemePreview::~ThemePreview()
{
}

void ThemePreview::setPreviewFixedHeight(int height)
{
    m_frame->setFixedHeight(height);
}

void ThemePreview::setPreviewFixedSize(QSize size)
{
    m_frame->setFixedSize(size);
}

void ThemePreview::setSpacingAndMargin(int spacing, QMargins margins)
{
    m_frameLayout->setSpacing(spacing);
    m_frameLayout->setContentsMargins(margins);
}

void ThemePreview::setSelectedIndicatorEnable(bool enable)
{
    m_selectedIndicatorEnable = enable;
    m_selectedIndicator->setVisible(enable);
}

void ThemePreview::setSelectedBorderWidth(int width)
{
    m_frame->setBorderWidth(width);
}

void ThemePreview::setThemeInfo(const QString& name,
                                      const QString& id)
{
    m_themeName = name;
    m_themeID = id;

    m_labelThemeName->setText(m_themeName);
}

void ThemePreview::setPreviewPixmaps(const QList<QPixmap>& pixmaps,const QSize& size)
{
    clearPreview();
    for (auto pixmap : pixmaps)
    {
        auto labelPixmap = new QLabel(this);
        labelPixmap->setProperty(theme_preview_tag_property, QVariant(1));
        labelPixmap->setFixedSize(size);
        labelPixmap->setPixmap(pixmap.scaled(size, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
        m_frameLayout->insertWidget(m_frameLayout->count(), labelPixmap, 0, Qt::AlignCenter);
    }
}

void ThemePreview::setPreviewWidget(QWidget* widget)
{
    clearPreview();
    widget->setProperty(theme_preview_tag_property, QVariant(1));
    widget->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
    m_frameLayout->insertWidget(m_frameLayout->count() - 1, widget, 10,Qt::AlignHCenter);
    
}

void ThemePreview::clearPreview()
{
    for (int i = 0; i < m_frameLayout->count();)
    {
        auto item = m_frameLayout->itemAt(i);
        auto widget = item->widget();

        if (widget && !widget->property(theme_preview_tag_property).isNull())
        {
            m_frameLayout->removeWidget(widget);
            delete widget;
            continue;
        }

        i++;
    }
}

QString ThemePreview::getID() const
{
    return m_themeID;
}

void ThemePreview::setSelected(bool selected)
{
    if (selected)
    {
        if (m_selectedIndicatorEnable)
        {
            m_selectedIndicator->setPixmap(QPixmap(selected_indicator_pixmap));
            m_frame->setDrawBroder(true);
        }
    }
    else
    {
        m_selectedIndicator->setPixmap(QPixmap());
        m_frame->setDrawBroder(false);
    }

    ExclusionWidget::setSelected(selected);
}

void ThemePreview::initUI()
{
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(6);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // 主题展示Frame
    m_frame = new KiranFrame(this);
    m_frame->setCursor(Qt::PointingHandCursor);
    m_frame->setObjectName("ThemePreviewFrame");
    m_frame->setDrawBroder(false);
    m_frame->setFixedBorderState(Kiran::Theme::Palette::ColorGroup::SELECTED);
    m_frame->installEventFilter(this);

    m_frameLayout = new QHBoxLayout(m_frame);
    m_frameLayout->setContentsMargins(0, 0, 0, 0);
    m_frameLayout->setSpacing(0);

    m_selectedIndicator = new QLabel(m_frame);
    m_selectedIndicator->setFixedSize(QSize(16, 16));
    m_frameLayout->addWidget(m_selectedIndicator,0,Qt::AlignHCenter|Qt::AlignRight);

    mainLayout->addWidget(m_frame);

    // 主题名字
    m_labelThemeName = new QLabel(this);
    m_labelThemeName->setObjectName("ThemePreviewName");
    m_labelThemeName->setStyleSheet("#ThemePreviewName{color: #919191;"
                                    "font-family: Noto Sans CJK SC regular;"
                                    "font-size: 14px}");
    m_labelThemeName->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(m_labelThemeName, Qt::AlignHCenter);
}

bool ThemePreview::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == m_frame && event->type() == QEvent::MouseButtonPress)
    {
        setSelected(true);
        emit pressed();
    }
    return false;
}
