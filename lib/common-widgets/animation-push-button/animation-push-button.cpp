/**
 * Copyright (c) 2020 ~ 2021 KylinSec Co., Ltd.
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

#include <QPainter>
#include <QPropertyAnimation>
#include <QStyleOption>
#include <QMouseEvent>
#include <qt5-log-i.h>
#include "logging-category.h"
#include "animation-push-button.h"

AnimationPushButton::AnimationPushButton(QWidget *parent)
    : QPushButton(parent),
      m_svgRender(QString(":/kiran-control-panel/images/loading.svg"), this)
{
    initTimeLine();
}

void AnimationPushButton::setBusy(bool busy)
{
    if (m_isBusy == busy)
        return;

    if (busy && !m_svgRender.isValid())
    {
        KLOG_WARNING(qLcCommonWidget) << "AnimationPushButton: animation pixmap isNull!";
        return;
    }
    m_isBusy = busy;
    if (m_isBusy)
    {
        m_rotationAngle = 0;
        m_timeLine.setCurrentTime(0);
        m_timeLine.start();
    }
    else
    {
        m_timeLine.stop();
        m_timeLine.setCurrentTime(0);
        m_rotationAngle = 0;
    }
}

bool AnimationPushButton::busy()
{
    return m_isBusy;
}

void AnimationPushButton::paintEvent(QPaintEvent *event)
{
    if (m_isBusy && isEnabled())
    {
        QPainter painter(this);
        painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

        QStyleOption opt;
        opt.init(this);
        style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);

        if (m_svgRender.isValid())
        {
            painter.translate(this->rect().center());
            painter.rotate(m_rotationAngle);
            int svgDrawSize = qMin(width(), height()) - 20;
            QRect renderRect((width() - svgDrawSize) / 2 - width() / 2,
                             (height() - svgDrawSize) / 2 - height() / 2,
                             svgDrawSize,
                             svgDrawSize);
            m_svgRender.render(&painter, renderRect);
        }
    }
    else
    {
        QPushButton::paintEvent(event);
    }
}

void AnimationPushButton::mousePressEvent(QMouseEvent *e)
{
    if( m_isBusy )
    {
        e->ignore();
        return;
    }
    QPushButton::mousePressEvent(e);
}

void AnimationPushButton::initTimeLine()
{
    m_timeLine.setCurrentTime(0);
    m_timeLine.setLoopCount(0);
    m_timeLine.setUpdateInterval(50);
    m_timeLine.setDuration(750);
    m_timeLine.setFrameRange(0, 360);
    m_timeLine.setEasingCurve(QEasingCurve(QEasingCurve::Linear));
    connect(&m_timeLine, &QTimeLine::frameChanged, [this](int value) {
        m_rotationAngle = value;
        update();
    });
}
