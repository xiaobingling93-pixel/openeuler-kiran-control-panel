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
 * Author:     yuanxing <yuanxing@kylinsec.com.cn>
 */

#pragma once

#include <kiran-system-daemon/upgrade-i.h>
#include <QHeaderView>
#include <QRect>
#include <QStyleOptionButton>
#include <QStyleOptionHeader>

class FilterMenu;

class CustomHeaderView : public QHeaderView
{
    Q_OBJECT
public:
    explicit CustomHeaderView(QWidget *parent = nullptr);
    virtual ~CustomHeaderView();

    void setCheckState(Qt::CheckState checkState);
    Qt::CheckState checkState() const { return m_checkState; }

signals:
    void toggled(Qt::CheckState checkState);
    void filterTypesChanged(AdvisoryKindFlags filterTypes);

protected:
    void paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;

private:
    void showFilterMenu(const QPoint &pos);

private:
    Qt::CheckState m_checkState;
    QRect m_checkboxRect;
    QRect m_filterButtonRect;
    bool m_filterMenuVisible;
    FilterMenu *m_filterMenu;
};