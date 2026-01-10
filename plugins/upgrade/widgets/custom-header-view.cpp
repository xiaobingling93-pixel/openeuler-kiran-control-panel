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

#include "custom-header-view.h"
#include <palette.h>
#include <QAbstractItemModel>
#include <QApplication>
#include <QHeaderView>
#include <QMouseEvent>
#include <QPainter>
#include <QStyle>
#include <QStyleOptionButton>
#include <QStyleOptionHeader>
#include "def.h"
#include "filter-menu.h"

using namespace Kiran::Theme;

CustomHeaderView::CustomHeaderView(QWidget *parent)
    : QHeaderView(Qt::Horizontal, parent),
      m_checkState(Qt::Unchecked),
      m_filterMenuVisible(false),
      m_filterMenu(nullptr)
{
    setSectionsClickable(true);
    setMouseTracking(true);

    // 创建自定义多选筛选菜单
    m_filterMenu = new FilterMenu(this);
    connect(m_filterMenu, &FilterMenu::filterTypesChanged, this, &CustomHeaderView::filterTypesChanged);
    connect(m_filterMenu, &FilterMenu::aboutToHide, [this]()
            {
                m_filterMenuVisible = false;
                viewport()->update();
            });
}

CustomHeaderView::~CustomHeaderView()
{
}

void CustomHeaderView::setCheckState(Qt::CheckState checkState)
{
    if (m_checkState != checkState)
    {
        m_checkState = checkState;
        viewport()->update();
    }
}

void CustomHeaderView::paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const
{
    painter->save();

    // 绘制背景
    auto kiranPalette = DEFAULT_PALETTE();
    QColor backgroundColor = kiranPalette->getColor(Palette::ColorGroup::ACTIVE, Palette::ColorRole::WIDGET);
    QColor textColor = kiranPalette->getColor(Palette::ColorGroup::ACTIVE, Palette::ColorRole::TEXT);

    painter->fillRect(rect, backgroundColor);

    // 绘制复选框
    if (logicalIndex == PackageTableField::PACKAGE_TABLE_FIELD_CHECKBOX)
    {
        auto widget = const_cast<CustomHeaderView *>(this);
        auto style = widget->style();

        QStyleOptionButton checkboxOption;
        int indicatorSize = style->pixelMetric(QStyle::PM_IndicatorWidth, nullptr, widget);
        QRect checkboxRect = QRect(0, 0, indicatorSize, indicatorSize);
        checkboxRect.moveCenter(rect.center());

        const_cast<CustomHeaderView *>(this)->m_checkboxRect = checkboxRect;

        checkboxOption.rect = checkboxRect;
        checkboxOption.state = QStyle::State_Enabled;

        if (m_checkState == Qt::Checked)
        {
            checkboxOption.state |= QStyle::State_On;
        }
        else if (m_checkState == Qt::PartiallyChecked)
        {
            checkboxOption.state |= QStyle::State_NoChange;
        }
        else
        {
            checkboxOption.state |= QStyle::State_Off;
        }

        style->drawPrimitive(QStyle::PE_IndicatorCheckBox, &checkboxOption, painter, widget);
    }
    // 绘制类型列文本和下拉箭头
    else if (logicalIndex == PackageTableField::PACKAGE_TABLE_FIELD_KINDS)
    {
        // 绘制文本
        QString text = model() ? model()->headerData(logicalIndex, Qt::Horizontal, Qt::DisplayRole).toString() : QString();
        painter->setPen(textColor);
        QRect textRect = rect.adjusted(10, 0, -25, 0);
        painter->drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, text);

        // 绘制下拉箭头
        QRect arrowRect = QRect(rect.right() - 25, rect.top(), 20, rect.height());
        const_cast<CustomHeaderView *>(this)->m_filterButtonRect = arrowRect;

        QStyleOptionViewItem arrowOption;
        arrowOption.rect = arrowRect;
        arrowOption.state = QStyle::State_Enabled;
        if (m_filterMenuVisible)
        {
            arrowOption.state |= QStyle::State_Active;
        }

        QStyle::PrimitiveElement arrowElement = m_filterMenuVisible
                                                    ? QStyle::PE_IndicatorArrowUp
                                                    : QStyle::PE_IndicatorArrowDown;

        style()->drawPrimitive(arrowElement, &arrowOption, painter, this);
    }
    // 其他列，绘制文本
    else
    {
        QString text = model() ? model()->headerData(logicalIndex, Qt::Horizontal, Qt::DisplayRole).toString() : QString();
        painter->setPen(textColor);
        QRect textRect = rect.adjusted(10, 0, 0, 0);  // 左边距10
        painter->drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, text);
    }

    painter->restore();
}

void CustomHeaderView::mousePressEvent(QMouseEvent *e)
{
    int column = logicalIndexAt(e->pos());

    // 复选框列：处理复选框点击
    if (column == PACKAGE_TABLE_FIELD_CHECKBOX && m_checkboxRect.contains(e->pos()))
    {
        m_checkState = (m_checkState == Qt::Checked) ? Qt::Unchecked : Qt::Checked;
        emit toggled(m_checkState);
        viewport()->update();
        return;
    }

    // 类型列：处理筛选按钮点击
    if (column == PackageTableField::PACKAGE_TABLE_FIELD_KINDS)
    {
        // 检查是否点击在箭头区域或整个表头区域
        if (m_filterButtonRect.contains(e->pos()) ||
            (sectionViewportPosition(column) >= 0 &&
             QRect(sectionViewportPosition(column), 0, sectionSize(column), height()).contains(e->pos())))
        {
            QPoint globalPos = mapToGlobal(QPoint(sectionViewportPosition(column), height()));
            showFilterMenu(globalPos);
            return;
        }
    }

    QHeaderView::mousePressEvent(e);
}

void CustomHeaderView::mouseMoveEvent(QMouseEvent *e)
{
    int column = logicalIndexAt(e->pos());

    // 更新鼠标悬停状态
    if (column == 0 && m_checkboxRect.contains(e->pos()))
    {
        setCursor(Qt::PointingHandCursor);
    }
    else if (column == PackageTableField::PACKAGE_TABLE_FIELD_KINDS && m_filterButtonRect.contains(e->pos()))
    {
        setCursor(Qt::PointingHandCursor);
    }
    else
    {
        setCursor(Qt::ArrowCursor);
    }

    QHeaderView::mouseMoveEvent(e);
}

void CustomHeaderView::showFilterMenu(const QPoint &pos)
{
    m_filterMenuVisible = true;
    viewport()->update();
    m_filterMenu->exec(pos);
}