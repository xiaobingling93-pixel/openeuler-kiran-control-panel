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
 * Author:     yangxiaoqing <yangxiaoqing@kylinsec.com.cn>
 */

#include "device-panel-widget.h"
#include <QButtonGroup>
#include <QPainter>
#include "display-config.h"

const int cAbsorbOffset = 5;
DevicePanelWidget::DevicePanelWidget(QWidget *parent) : QWidget(parent), m_isDrag(false), m_curCheckedItem(NULL)
{
    setAttribute(Qt::WA_TranslucentBackground, true);
    setMinimumSize(100, 50);
    m_btnGroup = new QButtonGroup(this);
    connect(m_btnGroup, SIGNAL(buttonToggled(QAbstractButton *, bool)), this, SLOT(onItemClicked(QAbstractButton *, bool)));

    m_displayConfig = DisplayConfig::instance();
    connect(m_displayConfig, &DisplayConfig::configModeChanged, this, &DevicePanelWidget::handleConfigModeChanged);
}

void DevicePanelWidget::setRotateDrect(const int &step)
{
    if (Q_UNLIKELY(!m_btnGroup)) return;

    DevicePanelItem *item = static_cast<DevicePanelItem *>(m_btnGroup->checkedButton());
    item->alterRotateDrect(step);
}

bool DevicePanelWidget::getHorizontalDisplayReflectType()
{
    if (Q_UNLIKELY(!m_btnGroup)) return false;

    DevicePanelItem *item = static_cast<DevicePanelItem *>(m_btnGroup->checkedButton());
    MonitorConfigDataPtr monitorBufferData = m_displayConfig->getMonitorConfigData(item->monitorPath());
    DisplayReflectTypes type = monitorBufferData->reflect();
    return type & DISPLAY_REFLECT_X;
}

bool DevicePanelWidget::getVerticalDisplayReflectType()
{
    if (Q_UNLIKELY(!m_btnGroup)) return false;

    DevicePanelItem *item = static_cast<DevicePanelItem *>(m_btnGroup->checkedButton());
    MonitorConfigDataPtr monitorBufferData = m_displayConfig->getMonitorConfigData(item->monitorPath());
    DisplayReflectTypes type = monitorBufferData->reflect();
    return type & DISPLAY_REFLECT_Y;
}

void DevicePanelWidget::setHorizontalDisplayReflectType(bool checked)
{
    if (Q_UNLIKELY(!m_btnGroup)) return;

    DevicePanelItem *item = static_cast<DevicePanelItem *>(m_btnGroup->checkedButton());
    MonitorConfigDataPtr monitorBufferData = m_displayConfig->getMonitorConfigData(item->monitorPath());
    DisplayReflectTypes type = monitorBufferData->reflect();
    type = checked ? (type | DISPLAY_REFLECT_X) : (type & ~DISPLAY_REFLECT_X);
    item->setDisplayReflectType(type);
}

void DevicePanelWidget::setVerticalDisplayReflectType(bool checked)
{
    if (Q_UNLIKELY(!m_btnGroup)) return;

    DevicePanelItem *item = static_cast<DevicePanelItem *>(m_btnGroup->checkedButton());
    MonitorConfigDataPtr monitorBufferData = m_displayConfig->getMonitorConfigData(item->monitorPath());
    DisplayReflectTypes type = monitorBufferData->reflect();
    type = checked ? (type | DISPLAY_REFLECT_Y) : (type & ~DISPLAY_REFLECT_Y);
    item->setDisplayReflectType(type);
}

void DevicePanelWidget::changeItemEnabled(const bool &enbled)
{
    if (Q_UNLIKELY(!m_btnGroup)) return;

    DevicePanelItem *item = static_cast<DevicePanelItem *>(m_btnGroup->checkedButton());
    item->changeEnabled(enbled);
    emit screenItemEnableChanged(item->enabled());
}

QString DevicePanelWidget::getCurMonitorText()
{
    if (Q_UNLIKELY(!m_curCheckedItem)) return QString();
    return m_curCheckedItem->text();
}

void DevicePanelWidget::updatePreview()
{
    QButtonGroup *btnGroup = m_btnGroup;
    if (Q_UNLIKELY(!btnGroup)) return;
    QList<QAbstractButton *> btns = btnGroup->buttons();
    int count = btns.count();
    if (count <= 0) return;
    // 用于计算各个屏幕占比
    QRectF sumScreenRectF;
    foreach (QAbstractButton *btn, btns)
    {
        DevicePanelItem *item = static_cast<DevicePanelItem *>(btn);
        QRectF rectF = item->screenGeometryF();
        if (rectF.width() == 0 || rectF.height() == 0) return;
        sumScreenRectF = sumScreenRectF.united(rectF);
    }
    foreach (QAbstractButton *btn, btns)
    {
        DevicePanelItem *item = static_cast<DevicePanelItem *>(btn);
        item->moveScreenGeometryFOffset(-sumScreenRectF.topLeft());
    }

    float sumScreenSizeWidth = sumScreenRectF.width();
    float sumScreenHeight = sumScreenRectF.height();

    if (Q_UNLIKELY(sumScreenSizeWidth == 0 || sumScreenHeight == 0)) return;

    // 父窗口总大小
    float width = (float)this->width();
    float height = (float)this->height();
    if (Q_UNLIKELY(width == 0 || height == 0)) return;
    //
    float curZoom = 0;
    float offset = 0;
    bool smallHeight = width / height >= sumScreenSizeWidth / sumScreenHeight;
    if (smallHeight)
    {
        curZoom = height / sumScreenHeight;
        // 水平居中
        offset = qAbs(width - curZoom * sumScreenSizeWidth) / 2;
    }
    else
    {
        curZoom = width / sumScreenSizeWidth;
        offset = qAbs(height - curZoom * sumScreenHeight) / 2;
    }

    for (int i = 0; i < count; ++i)
    {
        DevicePanelItem *item = static_cast<DevicePanelItem *>(btns.at(i));
        const QPointF screenPos = item->screenGeometryF().topLeft();
        float new_x = curZoom * screenPos.x();
        float new_y = curZoom * screenPos.y();

        const QSizeF &screenSize = item->screenGeometryF().size();
        float new_h = curZoom * screenSize.height();
        float new_w = curZoom * screenSize.width();

        auto *btn = m_btnGroup->button(i);
        if (!btn) continue;

        smallHeight ? btn->setGeometry(new_x + offset, new_y, new_w, new_h)
                    : btn->setGeometry(new_x, new_y + offset, new_w, new_h);
    }
}

void DevicePanelWidget::clear()
{
    m_isDrag = false;
    m_curCheckedItem = NULL;
    m_anchorPos.clear();

    if (Q_LIKELY(m_btnGroup))
    {
        QList<QAbstractButton *> btns = m_btnGroup->buttons();
        foreach (QAbstractButton *btn, btns)
        {
            m_btnGroup->removeButton(btn);
            delete btn;  // deleteLater() 会导致点击应用弹出MessageBox后，关闭一个显示器，模拟显示器窗刷新不及时的问题。
            // btn->deleteLater();
        }
    }
}

QString DevicePanelWidget::getCurMonitorPath() const
{
    if (Q_UNLIKELY(!m_curCheckedItem)) return QString();

    return m_curCheckedItem->monitorPath();
}

void DevicePanelWidget::onItemDraging(QAbstractButton *b)
{
    if (Q_UNLIKELY(!m_btnGroup)) return;
    m_anchorPos = getMinDisGeometry(b, m_btnGroup->buttons());
    m_isDrag = true;
    update();
}

void DevicePanelWidget::onItemEndDrag(QAbstractButton *btn)
{
    if (Q_UNLIKELY(!m_btnGroup) || Q_UNLIKELY(!btn)) return;
    DevicePanelItem *item = static_cast<DevicePanelItem *>(btn);
    item->setZoomPair(m_anchorPos.zoomPair);
    item->clearAnchoredChildBtns();
    item->setAnchorByBtn(m_anchorPos.anchorByBtn, m_anchorPos.drect);
    insertItem(btn, m_anchorPos, m_btnGroup->buttons());
    QList<QAbstractButton *> childs;
    mainCluster(item, m_btnGroup->buttons(), childs);
    gatherItems(childs);
    updatePreview();

    m_isDrag = false;
    update();
}

void DevicePanelWidget::onItemClicked(QAbstractButton *btn, bool isChecked)
{
    if (Q_UNLIKELY(!btn)) return;

    if (isChecked)
    {
        m_curCheckedItem = static_cast<DevicePanelItem *>(btn);
        emit screenItemChecked(m_curCheckedItem->monitorPath());
        emit screenItemEnableChanged(m_curCheckedItem->enabled());
    }
}

void DevicePanelWidget::handleConfigModeChanged(ConfigMode mode)
{
    KLOG_DEBUG() << "Config Mode Changed: " << mode;
    if (Q_UNLIKELY(!m_btnGroup)) return;

    clear();

    MonitorConfigDataList configDataList;
    if (mode == ConfigMode::CONFIG_MODE_COPY)
    {
        MonitorConfigDataPtr configData = DisplayConfig::instance()->initCopyMode();
        configDataList << configData;
    }
    else
        configDataList = DisplayConfig::instance()->initExtraMode();

    int count = configDataList.count();
    DevicePanelItem *checkedBtn = NULL;
    for (int i = 0; i < count; i++)
    {
        auto bufferData = configDataList.value(i);
        DevicePanelItem *btn = new DevicePanelItem(bufferData->path(), this);
        m_btnGroup->addButton(btn, i);
        btn->show();
        connect(btn, &DevicePanelItem::drag, this, &DevicePanelWidget::onItemDraging);
        connect(btn, &DevicePanelItem::endDrag, this, &DevicePanelWidget::onItemEndDrag);
        connect(btn, &DevicePanelItem::screenGeometryChanged, this, &DevicePanelWidget::updateScreenGeometry);

        if (checkedBtn)
        {
            if (checkedBtn->screenGeometryF().left() > btn->screenGeometryF().left()) checkedBtn = btn;  // 选出最左边的一个作为默认选中项。
        }
        else
        {
            checkedBtn = btn;
        }
    }

    gatherItemsFixPos(m_btnGroup->buttons());
    updatePreview();

    if (!checkedBtn) return;
    checkedBtn->setChecked(true);
}

void DevicePanelWidget::updateScreenGeometry()
{
    // 根据各个屏幕锚定位置重新更新所有虚拟屏幕显示几何位置信息
    QButtonGroup *btnGroup = m_btnGroup;
    const int count = btnGroup? btnGroup->buttons().count() : 0;
    for ( int i = 0; i < count; i++ )
    {
        DevicePanelItem *monitorItem = static_cast<DevicePanelItem *>(btnGroup->button(i));
        monitorItem->updateScreenGeometry();
    }

    // 更新预览，更新缩放率，以及偏移量。
    updatePreview();
}

void DevicePanelWidget::resizeEvent(QResizeEvent *event)
{
    updatePreview();
    QWidget::resizeEvent(event);
}

/*!
 * \brief DevicePanelWidget::getAvailableGeometry 获取指定方向上的Anchor Rect
 * \param g1
 * \param g2
 * \param drect
 * \param magnet
 * \return
 */
DevicePanelWidget::AnchorRectPos DevicePanelWidget::getAvailableGeometry(const QRect &g1, const QRect &g2, const DevicePanelItem::AnchorByDrect &drect, const bool &magnet)
{
    QRect r = g1;
    int d = 999999;
    QLine line;
    QLine dashesLine;
    QPair<int, int> zoomPair;
    switch (drect)
    {
    case DevicePanelItem::PosLeft:
        return getAnchorRectPosLeft(g1, g2, magnet);
    case DevicePanelItem::PosTop:
        return getAnchorRectPosTop(g1, g2, magnet);
    case DevicePanelItem::PosRight:
        return getAnchorRectPosRight(g1, g2, magnet);
    case DevicePanelItem::PosBottom:
        return getAnchorRectPosBottom(g1, g2, magnet);
    case DevicePanelItem::PosTopLeft:
    {
        if (!(g1.bottom() <= g2.top() && g1.right() <= g2.left())) return DevicePanelWidget::AnchorRectPos();

        r.moveBottomRight(g2.topLeft());
        QLineF lineF(r.center(), g1.center());
        d = lineF.length();
        line.setPoints(g1.bottomRight(), g2.topLeft());
    }
    break;
    case DevicePanelItem::PosTopRight:
    {
        if (!(g1.left() >= g2.right() && g1.bottom() <= g2.top())) return DevicePanelWidget::AnchorRectPos();
        r.moveBottomLeft(g2.topRight());
        QLineF lineF(r.center(), g1.center());
        d = lineF.length();
        line.setPoints(g1.bottomLeft(), g2.topRight());
    }
    break;
    case DevicePanelItem::PosBottomLeft:
    {
        if (!(g1.top() >= g2.bottom() && g1.right() <= g2.left())) return DevicePanelWidget::AnchorRectPos();
        r.moveTopRight(g2.bottomLeft());
        QLineF lineF(r.center(), g1.center());
        d = lineF.length();
        line.setPoints(g1.topRight(), g2.bottomLeft());
    }
    break;
    case DevicePanelItem::PosBottomRight:
    {
        if (!(g1.top() >= g2.bottom() && g1.left() >= g2.right())) return DevicePanelWidget::AnchorRectPos();
        r.moveTopLeft(g2.bottomRight());
        QLineF lineF(r.center(), g1.center());
        d = lineF.length();
        line.setPoints(g1.topLeft(), g2.bottomRight());
    }
    default:
        break;
    }

    DevicePanelWidget::AnchorRectPos ret;
    ret.r = r;
    ret.d = d;
    ret.line = line;
    ret.dashesLine = dashesLine;
    ret.drect = drect;
    ret.zoomPair = zoomPair;
    return ret;
}

DevicePanelWidget::AnchorRectPos DevicePanelWidget::getAnchorRectPosLeft(const QRect &g1, const QRect &g2, const bool &magnet)
{
    QRect r = g1;
    int d = 999999;
    QLine line;
    QLine dashesLine;
    QPair<int, int> zoomPair;

    if (g1.bottom() <= g2.top() || g1.top() >= g2.bottom() || g1.left() >= g2.center().x()) return DevicePanelWidget::AnchorRectPos();
    r.moveRight(g2.left());

    if (magnet)
    {
        if (r.top() <= (g2.top() + cAbsorbOffset) && r.top() >= (g2.top() - cAbsorbOffset)) r.moveTop(g2.top());
        if (r.top() <= (g2.bottom() + cAbsorbOffset) && r.top() >= (g2.bottom() - cAbsorbOffset)) r.moveTop(g2.bottom());

        if (r.bottom() <= (g2.top() + cAbsorbOffset) && r.bottom() >= (g2.top() - cAbsorbOffset)) r.moveBottom(g2.top());
        if (r.bottom() <= (g2.bottom() + cAbsorbOffset) && r.bottom() >= (g2.bottom() - cAbsorbOffset)) r.moveBottom(g2.bottom());
    }

    d = qAbs(r.center().x() - g1.center().x());
    line.setPoints(r.topRight(), r.bottomRight());
    dashesLine.setLine(line.x1(), -999999, line.x1(), 999999);
    zoomPair.first = r.top() - g2.top();
    zoomPair.second = g2.height();

    DevicePanelWidget::AnchorRectPos ret;
    ret.r = r;
    ret.d = d;
    ret.line = line;
    ret.dashesLine = dashesLine;
    ret.drect = DevicePanelItem::PosLeft;
    ret.zoomPair = zoomPair;
    return ret;
}

DevicePanelWidget::AnchorRectPos DevicePanelWidget::getAnchorRectPosRight(const QRect &g1, const QRect &g2, const bool &magnet)
{
    QRect r = g1;
    int d = 999999;
    QLine line;
    QLine dashesLine;
    QPair<int, int> zoomPair;
    if (g1.bottom() <= g2.top() || g1.top() >= g2.bottom() || g1.right() <= g2.center().x()) return DevicePanelWidget::AnchorRectPos();

    r.moveLeft(g2.right());

    if (magnet)
    {
        if (r.top() <= (g2.top() + cAbsorbOffset) && r.top() >= (g2.top() - cAbsorbOffset)) r.moveTop(g2.top());
        if (r.top() <= (g2.bottom() + cAbsorbOffset) && r.top() >= (g2.bottom() - cAbsorbOffset)) r.moveTop(g2.bottom());

        if (r.bottom() <= (g2.top() + cAbsorbOffset) && r.bottom() >= (g2.top() - cAbsorbOffset)) r.moveBottom(g2.top());
        if (r.bottom() <= (g2.bottom() + cAbsorbOffset) && r.bottom() >= (g2.bottom() - cAbsorbOffset)) r.moveBottom(g2.bottom());
    }

    d = qAbs(r.center().x() - g1.center().x());
    line.setPoints(r.topLeft(), r.bottomLeft());
    dashesLine.setLine(line.x1(), -999999, line.x1(), 999999);
    zoomPair.first = r.top() - g2.top();
    zoomPair.second = g2.height();

    DevicePanelWidget::AnchorRectPos ret;
    ret.r = r;
    ret.d = d;
    ret.line = line;
    ret.dashesLine = dashesLine;
    ret.drect = DevicePanelItem::PosRight;
    ret.zoomPair = zoomPair;
    return ret;
}

DevicePanelWidget::AnchorRectPos DevicePanelWidget::getAnchorRectPosTop(const QRect &g1, const QRect &g2, const bool &magnet)
{
    QRect r = g1;
    int d = 999999;
    QLine line;
    QLine dashesLine;
    QPair<int, int> zoomPair;
    if (g1.right() <= g2.left() || g1.left() >= g2.right() || g1.top() >= g2.center().y()) return DevicePanelWidget::AnchorRectPos();
    r.moveBottom(g2.top());

    if (magnet)
    {
        if (r.left() >= (g2.left() - cAbsorbOffset) && r.left() <= (g2.left() + cAbsorbOffset)) r.moveLeft(g2.left());
        if (r.left() >= (g2.right() - cAbsorbOffset) && r.left() <= (g2.right() + cAbsorbOffset)) r.moveLeft(g2.right());

        if (r.right() >= (g2.left() - cAbsorbOffset) && r.right() <= (g2.left() + cAbsorbOffset)) r.moveRight(g2.left());
        if (r.right() >= (g2.right() - cAbsorbOffset) && r.right() <= (g2.right() + cAbsorbOffset)) r.moveRight(g2.right());
    }

    d = qAbs(r.center().y() - g1.center().y());
    line.setPoints(r.bottomLeft(), r.bottomRight());
    dashesLine.setLine(-999999, line.y1(), 999999, line.y1());
    zoomPair.first = r.left() - g2.left();
    zoomPair.second = g2.width();

    DevicePanelWidget::AnchorRectPos ret;
    ret.r = r;
    ret.d = d;
    ret.line = line;
    ret.dashesLine = dashesLine;
    ret.drect = DevicePanelItem::PosTop;
    ret.zoomPair = zoomPair;
    return ret;
}

DevicePanelWidget::AnchorRectPos DevicePanelWidget::getAnchorRectPosBottom(const QRect &g1, const QRect &g2, const bool &magnet)
{
    QRect r = g1;
    int d = 999999;
    QLine line;
    QLine dashesLine;
    QPair<int, int> zoomPair;

    if (g1.right() <= g2.left() || g1.left() >= g2.right() || g1.bottom() <= g2.center().y()) return DevicePanelWidget::AnchorRectPos();
    r.moveTop(g2.bottom());

    if (magnet)
    {
        if (r.left() >= (g2.left() - cAbsorbOffset) && r.left() <= (g2.left() + cAbsorbOffset)) r.moveLeft(g2.left());
        if (r.left() >= (g2.right() - cAbsorbOffset) && r.left() <= (g2.right() + cAbsorbOffset)) r.moveLeft(g2.right());

        if (r.right() >= (g2.left() - cAbsorbOffset) && r.right() <= (g2.left() + cAbsorbOffset)) r.moveRight(g2.left());
        if (r.right() >= (g2.right() - cAbsorbOffset) && r.right() <= (g2.right() + cAbsorbOffset)) r.moveRight(g2.right());
    }

    d = qAbs(r.center().y() - g1.center().y());
    line.setPoints(r.topLeft(), r.topRight());
    dashesLine.setLine(-999999, line.y1(), 999999, line.y1());
    zoomPair.first = r.left() - g2.left();
    zoomPair.second = g2.width();

    DevicePanelWidget::AnchorRectPos ret;
    ret.r = r;
    ret.d = d;
    ret.line = line;
    ret.dashesLine = dashesLine;
    ret.drect = DevicePanelItem::PosBottom;
    ret.zoomPair = zoomPair;
    return ret;
}

/*!
 * \brief DevicePanelWidget::getAvailableGeometrys 获取所有方向的Anchor Rect
 * \param g1
 * \param g2
 * \param magnet
 * \return
 */
QList<DevicePanelWidget::AnchorRectPos> DevicePanelWidget::getAvailableGeometrys(const QRect &g1, const QRect &g2, const bool &magnet)
{
    QList<DevicePanelWidget::AnchorRectPos> ret;
    for (int i = 0; i < 8; ++i)
    {
        DevicePanelWidget::AnchorRectPos st = getAvailableGeometry(g1, g2, (DevicePanelItem::AnchorByDrect)i, magnet);
        if (st.r.isEmpty()) continue;

        ret << st;
    }
    return ret;
}

DevicePanelWidget::AnchorRectPos DevicePanelWidget::getMinDisScreenGeometry(DevicePanelItem *b, QList<QAbstractButton *> btns, const bool &magnet)
{
    QList<DevicePanelWidget::AnchorRectPos> list;
    QRectF g = b->screenGeometryF();
    foreach (QAbstractButton *btn, btns)
    {
        if (btn == b) continue;
        DevicePanelItem *item = static_cast<DevicePanelItem *>(btn);
        QRectF geometry = item->screenGeometryF();
        QList<DevicePanelWidget::AnchorRectPos> anchorList = getAvailableGeometrys(g.toRect(), geometry.toRect(), magnet);
        for (int i = 0; i < anchorList.count(); ++i)
        {
            anchorList[i].anchorByBtn = static_cast<DevicePanelItem *>(btn);
        }
        list << anchorList;
    }

    return getMinDisGeometryPrivate(list);
}
DevicePanelWidget::AnchorRectPos DevicePanelWidget::getMinDisGeometry(QAbstractButton *b, QList<QAbstractButton *> btns, const bool &magnet)
{
    QList<DevicePanelWidget::AnchorRectPos> list;
    QRect g = b->geometry();
    foreach (QAbstractButton *btn, btns)
    {  // must use btns
        if (btn == b) continue;
        QRect geometry = btn->geometry();
        QList<DevicePanelWidget::AnchorRectPos> anchorList = getAvailableGeometrys(g, geometry, magnet);
        for (int i = 0; i < anchorList.count(); ++i)
        {
            anchorList[i].anchorByBtn = static_cast<DevicePanelItem *>(btn);
        }
        list << anchorList;
    }

    return getMinDisGeometryPrivate(list);
}

DevicePanelWidget::AnchorRectPos DevicePanelWidget::getMinDisGeometryPrivate(const QList<DevicePanelWidget::AnchorRectPos> &list)
{
    DevicePanelWidget::AnchorRectPos ret;
    ret.d = 999999;
    foreach (DevicePanelWidget::AnchorRectPos l, list)
    {
        if (l.d < ret.d)
            ret = l;
    }

    return ret;
}

void DevicePanelWidget::insertItem(QAbstractButton *insertBtn, const DevicePanelWidget::AnchorRectPos &anchorPos, const QList<QAbstractButton *> &btns)
{
    if (Q_UNLIKELY(!insertBtn)) return;
    DevicePanelItem *item = static_cast<DevicePanelItem *>(insertBtn);
    QRectF itemScreenRect = item->screenGeometryF();

    foreach (QAbstractButton *btn, btns)
    {
        if (btn == item) continue;
        DevicePanelItem *otherItem = static_cast<DevicePanelItem *>(btn);
        QRectF otherScreenGeometry = otherItem->screenGeometryF();

        QRectF intersec = itemScreenRect.intersected(otherScreenGeometry);
        if (intersec.isValid() && !intersec.isEmpty())
        {
            DevicePanelItem::AnchorByDrect spDrect = insertDrect(itemScreenRect, otherScreenGeometry, anchorPos.drect);
            otherItem->setAnchorByBtn(static_cast<DevicePanelItem *>(insertBtn), spDrect);
            //
            intersec = itemScreenRect.intersected(otherItem->screenGeometryF());
            if (intersec.isValid() && !intersec.isEmpty())
            {
                insertItem(insertBtn, anchorPos, btns);
            }
            else
            {
                AnchorRectPos anchPos;
                anchPos.drect = anchorPos.drect;

                QList<QAbstractButton *> btnList = btns;
                btnList.removeOne(insertBtn);
                btnList.removeOne(btn);
                insertItem(btn, anchPos, btnList);
            }
        }
    }
}

void DevicePanelWidget::gatherItemsFixPos(const QList<QAbstractButton *> &items)
{
    foreach (QAbstractButton *btn, items)
    {
        DevicePanelItem *item = static_cast<DevicePanelItem *>(btn);

        DevicePanelWidget::AnchorRectPos anchorPos = getMinDisScreenGeometry(item, items, false);

        if (!anchorPos.anchorByBtn) continue;
        if (anchorPos.anchorByBtn->anchorByBtn() == item) continue;

        item->setAnchorByBtn(anchorPos.anchorByBtn, anchorPos.drect);
    }
}

DevicePanelItem::AnchorByDrect DevicePanelWidget::insertDrect(const QRectF &r, const QRectF &movedR, const DevicePanelItem::AnchorByDrect &drect)
{
    DevicePanelItem::AnchorByDrect ret = drect;
    switch (drect)
    {
    case DevicePanelItem::PosLeft:
        if (movedR.right() > r.right())
        {
            ret = DevicePanelItem::PosRight;
        }
        break;
    case DevicePanelItem::PosRight:
        if (movedR.left() < r.left())
        {
            ret = DevicePanelItem::PosLeft;
        }
        break;
    case DevicePanelItem::PosTop:
        if (movedR.bottom() > r.bottom())
        {
            ret = DevicePanelItem::PosBottom;
        }
        break;
    case DevicePanelItem::PosBottom:
        if (movedR.top() < r.top())
        {
            ret = DevicePanelItem::PosTop;
        }
        break;
    default:
        break;
    }

    return ret;
}

void DevicePanelWidget::gatherItems(QList<QAbstractButton *> &items)
{
    QList<QAbstractButton *> t_items = items;
    QList<QAbstractButton *> btns = m_btnGroup->buttons();
    foreach (QAbstractButton *btn, btns)
    {
        if (items.contains(btn)) continue;
        DevicePanelItem *item = static_cast<DevicePanelItem *>(btn);

        DevicePanelWidget::AnchorRectPos anchorPos = getMinDisScreenGeometry(item, t_items, false);
        item->setAnchorByBtn(anchorPos.anchorByBtn, anchorPos.drect);
        t_items << item;
    }
}

void DevicePanelWidget::mainCluster(DevicePanelItem *dragBtn, const QList<QAbstractButton *> &list, QList<QAbstractButton *> &ret)
{
    if (Q_UNLIKELY(!dragBtn)) return;

    QRectF screenGeo = dragBtn->screenGeometryF();
    screenGeo.adjust(-1, -1, 1, 1);
    foreach (QAbstractButton *btn, list)
    {
        DevicePanelItem *b = static_cast<DevicePanelItem *>(btn);
        if (screenGeo.intersects(b->screenGeometryF()) && !ret.contains(b))
        {
            ret << b;
            QList<QAbstractButton *> t_list = list;
            t_list.removeAll(b);
            mainCluster(b, t_list, ret);
        }
    }
}
