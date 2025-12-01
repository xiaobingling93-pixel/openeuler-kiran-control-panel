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
#include "preview-item.h"
#include <kiran-integration/theme/palette.h>
#include <KDecoration2/Decoration>
#include <QCoreApplication>
#include <QHoverEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QTimer>
#include <cmath>
#include "preview-bridge.h"
#include "preview-client.h"
#include "preview-settings.h"

using namespace Kiran::Theme;
namespace Kiran
{
namespace Decoration
{
PreviewItem::PreviewItem(PreviewBridge *bridge, QWidget *parent)
    : QWidget(parent), m_bridge(bridge), m_anchorInfo(QRectF(0.0, 0.0, 1.0, 1.0))
{
    init();
}

PreviewItem::~PreviewItem()
{
    if (m_bridge)
    {
        m_bridge->unregisterPreviewItem(this);
    }
}

void PreviewItem::init()
{
    // 启用鼠标跟踪
    setMouseTracking(true);
    setAttribute(Qt::WA_Hover);
    // 根据锚定信息设置位置和大小，如果没有设置锚定信息则铺满父窗口
    if (parentWidget())
    {
        updateGeometryFromAnchor();
        parentWidget()->installEventFilter(this);
    }
    // 初始化Decoration
    createDecoration();
}

bool PreviewItem::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == parentWidget())
    {
        if (event->type() == QEvent::Resize)
        {
            QTimer::singleShot(0, this, [this]() { updateGeometryFromAnchor(); });
        }
    }
    return QWidget::eventFilter(watched, event);
}

bool PreviewItem::event(QEvent *event)
{
    switch (event->type())
    {
    // 同步预览控件尺寸到模拟DecorationClient
    // 确保Decoration框架内能获取到真实预览模拟窗口的尺寸
    case QEvent::Resize:
        QTimer::singleShot(0, this, [this]() { syncSizeToClient(); });
        break;
    // 转发鼠标等事件到Decoration框架，确保主题外部框预览更新
    case QEvent::MouseMove:
    // case QEvent::MouseButtonPress:
    // case QEvent::MouseButtonRelease:
        translateMouseEvent(dynamic_cast<QMouseEvent *>(event));
        return true;
    case QEvent::HoverEnter:
    case QEvent::HoverLeave:
    case QEvent::HoverMove:
        translateHoverEvent(dynamic_cast<QHoverEvent *>(event));
        return true;
    default:
        break;
    }
    return QWidget::event(event);
}

void PreviewItem::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);

    int paddingLeft, paddingTop, paddingRight, paddingBottom;
    paddingLeft = paddingTop = paddingRight = paddingBottom = 0;

    // 阴影绘制
    paintShadow(&painter, paddingLeft, paddingTop, paddingRight, paddingBottom);
    
    // Decoration绘制
    m_decoration->paint(&painter, QRect(0, 0, width(), height()));

    // 背景绘制
    QRect backgroundRect(m_decoration->borderLeft(), m_decoration->borderTop(),
                         width() - m_decoration->borderLeft() - m_decoration->borderRight() -
                             paddingLeft - paddingRight,
                         height() - m_decoration->borderTop() - m_decoration->borderBottom() -
                             paddingTop - paddingBottom);
    QColor backgroundColor = DEFAULT_PALETTE()->getColor(
        m_client->isActive() ? Palette::ColorGroup::ACTIVE : Palette::ColorGroup::INACTIVE,
        Palette::ColorRole::WINDOW);
    painter.fillRect(backgroundRect, backgroundColor);
}

void PreviewItem::createDecoration()
{
    m_settings = QSharedPointer<KDecoration2::DecorationSettings>::create(m_bridge, this);
    m_decoration.reset(m_bridge->createDecoration(nullptr));
    m_decoration->setSettings(m_settings);
    m_decoration->init();

    m_client = m_bridge->lastCreatedClient();
    m_bridge->registerPreviewItem(this);

    connect(m_decoration.get(), &KDecoration2::Decoration::bordersChanged, this,
            &PreviewItem::syncSizeToClient);
    connect(m_decoration.get(), &KDecoration2::Decoration::shadowChanged, this,
            &PreviewItem::syncSizeToClient);
    connect(m_decoration.get(), &KDecoration2::Decoration::shadowChanged, this,
            &PreviewItem::shadowChanged);
    connect(m_decoration.get(), &KDecoration2::Decoration::damaged, this, [this]() { update(); });
}

void PreviewItem::syncSizeToClient()
{
    if (!m_client)
    {
        return;
    }

    int widthOffset = 0;
    int heightOffset = 0;
    auto shadow = m_decoration->shadow();
    if (shadow)
    {
        widthOffset = shadow->paddingLeft() + shadow->paddingRight();
        heightOffset = shadow->paddingTop() + shadow->paddingBottom();
    }
    m_client->setWidth(width() - m_decoration->borderLeft() - m_decoration->borderRight() -
                       widthOffset);
    m_client->setHeight(height() - m_decoration->borderTop() - m_decoration->borderBottom() -
                        heightOffset);
}

void PreviewItem::paintShadow(QPainter *painter, int &paddingLeft, int &paddingTop,
                              int &paddingRight, int &paddingBottom)
{
    const auto &shadow = m_decoration->shadow();
    if (!shadow)
    {
        return;
    }
    paddingLeft = shadow->paddingLeft();
    paddingTop = shadow->paddingTop();
    paddingRight = shadow->paddingRight();
    paddingBottom = shadow->paddingBottom();

    const QImage shadowPixmap = shadow->shadow();
    if (shadowPixmap.isNull())
    {
        return;
    }

    const QRect outerRect(-paddingLeft, -paddingTop, width(), height());
    const QRect shadowRect(shadowPixmap.rect());

    const QSize topLeftSize(shadow->topLeftGeometry().size());
    QRect topLeftTarget(QPoint(outerRect.x(), outerRect.y()), topLeftSize);

    const QSize topRightSize(shadow->topRightGeometry().size());
    QRect topRightTarget(
        QPoint(outerRect.x() + outerRect.width() - topRightSize.width(), outerRect.y()),
        topRightSize);

    const QSize bottomRightSize(shadow->bottomRightGeometry().size());
    QRect bottomRightTarget(QPoint(outerRect.x() + outerRect.width() - bottomRightSize.width(),
                                   outerRect.y() + outerRect.height() - bottomRightSize.height()),
                            bottomRightSize);

    const QSize bottomLeftSize(shadow->bottomLeftGeometry().size());
    QRect bottomLeftTarget(
        QPoint(outerRect.x(), outerRect.y() + outerRect.height() - bottomLeftSize.height()),
        bottomLeftSize);

    // Re-distribute the corner tiles so no one of them is overlapping with others.
    // By doing this, we assume that shadow's corner tiles are symmetric
    // and it is OK to not draw top/right/bottom/left tile between corners.
    // For example, let's say top-left and top-right tiles are overlapping.
    // In that case, the right side of the top-left tile will be shifted to left,
    // the left side of the top-right tile will shifted to right, and the top
    // tile won't be rendered.
    bool drawTop = true;
    if (topLeftTarget.x() + topLeftTarget.width() >= topRightTarget.x())
    {
        const float halfOverlap =
            qAbs(topLeftTarget.x() + topLeftTarget.width() - topRightTarget.x()) / 2.0f;
        topLeftTarget.setRight(topLeftTarget.right() - std::floor(halfOverlap));
        topRightTarget.setLeft(topRightTarget.left() + std::ceil(halfOverlap));
        drawTop = false;
    }

    bool drawRight = true;
    if (topRightTarget.y() + topRightTarget.height() >= bottomRightTarget.y())
    {
        const float halfOverlap =
            qAbs(topRightTarget.y() + topRightTarget.height() - bottomRightTarget.y()) / 2.0f;
        topRightTarget.setBottom(topRightTarget.bottom() - std::floor(halfOverlap));
        bottomRightTarget.setTop(bottomRightTarget.top() + std::ceil(halfOverlap));
        drawRight = false;
    }

    bool drawBottom = true;
    if (bottomLeftTarget.x() + bottomLeftTarget.width() >= bottomRightTarget.x())
    {
        const float halfOverlap =
            qAbs(bottomLeftTarget.x() + bottomLeftTarget.width() - bottomRightTarget.x()) / 2.0f;
        bottomLeftTarget.setRight(bottomLeftTarget.right() - std::floor(halfOverlap));
        bottomRightTarget.setLeft(bottomRightTarget.left() + std::ceil(halfOverlap));
        drawBottom = false;
    }

    bool drawLeft = true;
    if (topLeftTarget.y() + topLeftTarget.height() >= bottomLeftTarget.y())
    {
        const float halfOverlap =
            qAbs(topLeftTarget.y() + topLeftTarget.height() - bottomLeftTarget.y()) / 2.0f;
        topLeftTarget.setBottom(topLeftTarget.bottom() - std::floor(halfOverlap));
        bottomLeftTarget.setTop(bottomLeftTarget.top() + std::ceil(halfOverlap));
        drawLeft = false;
    }

    painter->translate(paddingLeft, paddingTop);

    painter->drawImage(topLeftTarget, shadowPixmap, QRect(QPoint(0, 0), topLeftTarget.size()));

    painter->drawImage(
        topRightTarget, shadowPixmap,
        QRect(QPoint(shadowRect.width() - topRightTarget.width(), 0), topRightTarget.size()));

    painter->drawImage(bottomRightTarget, shadowPixmap,
                       QRect(QPoint(shadowRect.width() - bottomRightTarget.width(),
                                    shadowRect.height() - bottomRightTarget.height()),
                             bottomRightTarget.size()));

    painter->drawImage(
        bottomLeftTarget, shadowPixmap,
        QRect(QPoint(0, shadowRect.height() - bottomLeftTarget.height()), bottomLeftTarget.size()));

    if (drawTop)
    {
        QRect topTarget(topLeftTarget.x() + topLeftTarget.width(), topLeftTarget.y(),
                        topRightTarget.x() - topLeftTarget.x() - topLeftTarget.width(),
                        topRightTarget.height());
        QRect topSource(shadow->topGeometry());
        topSource.setHeight(topTarget.height());
        topSource.moveTop(shadowRect.top());
        painter->drawImage(topTarget, shadowPixmap, topSource);
    }

    if (drawRight)
    {
        QRect rightTarget(topRightTarget.x(), topRightTarget.y() + topRightTarget.height(),
                          topRightTarget.width(),
                          bottomRightTarget.y() - topRightTarget.y() - topRightTarget.height());
        QRect rightSource(shadow->rightGeometry());
        rightSource.setWidth(rightTarget.width());
        rightSource.moveRight(shadowRect.right());
        painter->drawImage(rightTarget, shadowPixmap, rightSource);
    }

    if (drawBottom)
    {
        QRect bottomTarget(bottomLeftTarget.x() + bottomLeftTarget.width(), bottomLeftTarget.y(),
                           bottomRightTarget.x() - bottomLeftTarget.x() - bottomLeftTarget.width(),
                           bottomRightTarget.height());
        QRect bottomSource(shadow->bottomGeometry());
        bottomSource.setHeight(bottomTarget.height());
        bottomSource.moveBottom(shadowRect.bottom());
        painter->drawImage(bottomTarget, shadowPixmap, bottomSource);
    }

    if (drawLeft)
    {
        QRect leftTarget(topLeftTarget.x(), topLeftTarget.y() + topLeftTarget.height(),
                         topLeftTarget.width(),
                         bottomLeftTarget.y() - topLeftTarget.y() - topLeftTarget.height());
        QRect leftSource(shadow->leftGeometry());
        leftSource.setWidth(leftTarget.width());
        leftSource.moveLeft(shadowRect.left());
        painter->drawImage(leftTarget, shadowPixmap, leftSource);
    }
}

void PreviewItem::translateMouseEvent(QMouseEvent *event)
{
    const auto &shadow = m_decoration->shadow();
    if (shadow)
    {
        QMouseEvent e(event->type(),
                      event->localPos() - QPointF(shadow->paddingLeft(), shadow->paddingTop()),
                      event->button(), event->buttons(), event->modifiers());
        QCoreApplication::instance()->sendEvent(m_decoration.get(), &e);
    }
    else
    {
        QCoreApplication::instance()->sendEvent(m_decoration.get(), event);
    }
}
void PreviewItem::translateHoverEvent(QHoverEvent *event)
{
    const auto &shadow = m_decoration->shadow();
    if (shadow)
    {
        QHoverEvent e(event->type(),
                      event->posF() - QPointF(shadow->paddingLeft(), shadow->paddingTop()),
                      event->oldPosF() - QPointF(shadow->paddingLeft(), shadow->paddingTop()),
                      event->modifiers());
        QCoreApplication::instance()->sendEvent(m_decoration.get(), &e);
    }
    else
    {
        QCoreApplication::instance()->sendEvent(m_decoration.get(), event);
    }
}

void PreviewItem::setAnchorInfo(const QRectF &anchorRect)
{
    m_anchorInfo = anchorRect;
    if (parentWidget() && m_anchorInfo.isValid() && !m_anchorInfo.isEmpty())
    {
        updateGeometryFromAnchor();
    }
}

PreviewClient *PreviewItem::client() const { return m_client; }

void PreviewItem::updateGeometryFromAnchor()
{
    if (!parentWidget() || !m_anchorInfo.isValid() || m_anchorInfo.isEmpty())
    {
        return;
    }

    QRect parentRect = parentWidget()->rect();
    int x = static_cast<int>(parentRect.x() + m_anchorInfo.x() * parentRect.width());
    int y = static_cast<int>(parentRect.y() + m_anchorInfo.y() * parentRect.height());
    int width = static_cast<int>(m_anchorInfo.width() * parentRect.width());
    int height = static_cast<int>(m_anchorInfo.height() * parentRect.height());

    setGeometry(x, y, width, height);
}
}  // namespace Decoration
}  // namespace Kiran