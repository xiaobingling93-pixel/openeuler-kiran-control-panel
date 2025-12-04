/**
 * Copyright (c) 2022 KylinSec Co., Ltd.
 * kiran-control-panel is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 *
 * Author:     luoqing <luoqing@kylinsec.com.cn>
 */

#include "connection-indicator.h"
#include <QImage>
#include <QPainter>
#include <QIcon>
#include <QPainterPath>
#include <qt5-log-i.h>
#include "logging-category.h"

namespace Kiran
{
namespace Network
{
QPixmap convertOpacity(const QPixmap &source, double opacity)
{
    QImage image = source.toImage();

    for (int y = 0; y < image.height(); ++y)
    {
        for (int x = 0; x < image.width(); ++x)
        {
            auto pixelColor = image.pixelColor(x,y);
            auto newPixelColor = pixelColor;
            if( pixelColor.alpha() != 0 )
            {
                newPixelColor.setAlpha( pixelColor.alpha() * opacity );
            }
            image.setPixelColor(x,y,newPixelColor);
        }
    }

    return QPixmap::fromImage(image);
}

ConnectionIndicator::ConnectionIndicator(QWidget *parent) : QLabel(parent)
{
    init();
}

void ConnectionIndicator::init()
{
    setFixedSize(16, 16);
    setAttribute(Qt::WA_Hover);

    auto iconPixmap = QIcon(":/kcp-network/images/indicator-selected.svg").pixmap(16,16);
    m_connectedPixmap = iconPixmap;
    m_hoverPixmap = convertOpacity(m_connectedPixmap, 0.2);

    m_group = new QParallelAnimationGroup(this);
    m_group->setLoopCount(-1);

    m_animation = new QPropertyAnimation(this);
    m_animation->setPropertyName("angle");
    m_animation->setTargetObject(this);
    m_animation->setStartValue(0);
    m_animation->setEndValue(719);
    m_animation->setDuration(2050);

    m_group->addAnimation(m_animation);
}

void ConnectionIndicator::paintEvent(QPaintEvent *event)
{
    if (m_isLoading)
    {
        Q_UNUSED(event)

        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.translate(width() / 2, height() / 2);
        painter.rotate(m_angle);

        QConicalGradient gra(QPoint(0, 0), 0);
        gra.setColorAt(0, QColor("#3BB6FE"));
        gra.setColorAt(1, QColor("#FFFFFF"));
        QBrush brush(gra);

        QRect rect(-m_radiusSize, -m_radiusSize, m_radiusSize * 2, m_radiusSize * 2);
        QPainterPath path;
        path.arcTo(rect, 0, 270);

        QPainterPath subPath;
        subPath.addEllipse(rect.adjusted(m_lineWidth, m_lineWidth, -m_lineWidth, -m_lineWidth));

        path = path - subPath;
        painter.setBrush(brush);
        painter.setPen(Qt::NoPen);
        painter.drawPath(path);
    }
    else if (!m_isLoading && !m_isActivated && parentWidget()->underMouse() && !m_hoverPixmap.isNull())
    {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        auto pixmapRect = m_hoverPixmap.rect();
        pixmapRect.moveCenter(rect().center());
        painter.drawPixmap(pixmapRect,m_hoverPixmap);
    }
    else
    {
        QLabel::paintEvent(event);
    }
}

void ConnectionIndicator::setLoadingStatus(bool isLoading)
{
    m_isLoading = isLoading;
    if (isLoading)
        m_group->start();
    else
        m_group->stop();
    update();
}

void ConnectionIndicator::setActivated(bool isActivated)
{
    if (isActivated == m_isActivated)
    {
        return;
    }

    m_isActivated = isActivated;

    if (m_isActivated)
    {
        m_isLoading = false;
    }

    setPixmap(isActivated ? m_connectedPixmap : QPixmap());
    update();
}

void ConnectionIndicator::setAngle(int angle)
{
    m_angle = angle;
    this->update();
}

int ConnectionIndicator::angle() const
{
    return m_angle;
}
}  // namespace Network
}  // namespace Kiran