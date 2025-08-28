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

#pragma once

#include <QEvent>
#include <QPushButton>
#include "display-config.h"
#include "kiran-session-daemon/display-i.h"

class DevicePanelItem : public QPushButton
{
    Q_OBJECT
public:
    explicit DevicePanelItem(const QString &monitorPath, QWidget *parent = nullptr);

    enum AnchorByDrect
    {
        PosLeft = 0,
        PosRight,
        PosTop,
        PosBottom,
        PosTopLeft,
        PosTopRight,
        PosBottomLeft,
        PosBottomRight
    };

    void init();
    QRectF screenGeometryF() const;
    void setScreenGeometryF(const QRectF &screenGeometryF);

    void moveScreenGeometryFOffset(const QPointF &offsetF);

    DevicePanelItem *anchorByBtn() const;
    void setAnchorByBtn(DevicePanelItem *anchorByBtn, const AnchorByDrect &anchorByDrect);

    AnchorByDrect anchorByDrect() const;
    void setAnchorByDrect(const AnchorByDrect &anchorByDrect);

    void removeAnchoredChildBtn(DevicePanelItem *childBtn);
    void appendAnchoredChildBtn(DevicePanelItem *childBtn);
    void clearAnchorByBtn();
    void clearAnchoredChildBtns();

    bool hasIntersects(DevicePanelItem *item);

    QPair<int, int> zoomPair() const;
    void setZoomPair(const QPair<int, int> &zoomPair);

    /**
     * @brief alterRotateDrect 旋转角度修改之后，需要通知item容器重新计算位置，要发送信号出去。
     * @param rotateDrect
     */
    void alterRotateDrect(const int &step = 1);
    void initRotateDrect(const DisplayRotationType &rotateDrect);

    void setDisplayReflectType(const DisplayReflectTypes &displayReflectType);

    QString monitorPath() const;

    void setEnabled(bool enabled);
    void changeEnabled(const bool &enabled);
    bool enabled() const;

    void updateScreenGeometry();

signals:
    void drag(QAbstractButton *btn);
    void endDrag(QAbstractButton *btn);
    void screenGeometryChanged();

private slots:
    void handleConfigResolvingChanged(const QSize &size);

private:
    void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    bool eventFilter(QObject *obj, QEvent *event);

    void updateOffset(DevicePanelItem *anchorByBtn, const AnchorByDrect &anchorByDrect, const bool &isDrag);
    DisplayRotationType rotationType(const DisplayRotationType &curType, const int &step);

private:
    bool m_mousePress;
    bool m_mouseDrag;
    QPoint m_pressPos;
    QEvent::Type m_statusType;
    //desktop invented data
    DevicePanelItem *m_anchorByBtn;
    QPair<int, int> m_zoomPair;
    //reality data
    bool m_enabled;
    QString m_monitorPath;
    AnchorByDrect m_anchorByDrect;
    QPointF m_screenOffset;
    QRectF m_screenGeometryF;
    DisplayRotationType m_rotateDrect;
    DisplayReflectTypes m_displayReflectType;

    QList<DevicePanelItem *> m_childAnchorBtns;

    MonitorConfigDataPtr m_monitorConfigData;
};
