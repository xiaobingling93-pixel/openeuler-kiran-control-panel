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
#include "exclusion-group.h"
#include <qt5-log-i.h>
#include "logging-category.h"

ExclusionWidget::ExclusionWidget(QWidget* parent)
    : QWidget(parent)
{
}

ExclusionWidget::~ExclusionWidget()
{
}

ExclusionGroup::ExclusionGroup(QObject* parent)
    : QObject(parent)
{
}

ExclusionGroup::~ExclusionGroup()
{
}

void ExclusionGroup::setCurrent(const QString& id)
{
    for( auto item : m_exclusionItems )
    {
        if( item->getID() != id )
            continue;
        setCurrent(item);
        break;
    }
}

void ExclusionGroup::setCurrent(ExclusionWidget* widget)
{
    if (m_current == widget)
    {
        return;
    }

    if (!m_exclusionItems.contains(widget) && widget != nullptr)
    {
        KLOG_ERROR(qLcAppearance) << "exclusion items not contain this item:" << widget
                                  << "set current failed.";
        return;
    }

    if (m_current != nullptr)
    {
        // 临时状态，屏蔽当前选择项，选中状态变更信号
        QSignalBlocker blocker(m_current);
        m_current->setSelected(false);
    }

    if (widget != nullptr)
    {
        widget->setSelected(true);
    }

    m_current = widget;
    emit currentItemChanged();
}

ExclusionWidget* ExclusionGroup::getCurrent() const
{
    return m_current;
}

QString ExclusionGroup::getCurrentID() const
{
    return m_current != nullptr ? m_current->getID() : QString();
}

QSet<QString> ExclusionGroup::getExclusionItemIDs() const
{
    QSet<QString> ids;
    for( auto item : m_exclusionItems )
    {
        ids << item->getID();
    }
    return ids;
}

void ExclusionGroup::addExclusionItem(ExclusionWidget* widget)
{
    if (m_exclusionItems.contains(widget))
    {
        KLOG_WARNING(qLcAppearance) << "this exclusion item" << widget << "existed!";
        return;
    }

    // 此时已有选中项，应重置当前添加项选择状态
    if (m_current != nullptr && widget->getSelected())
    {
        // 默认保当前的选中状态，重置新增的互斥项
        widget->setSelected(false);
    }

    m_exclusionItems << widget;
    connect(widget, &ExclusionWidget::selectedStatusChanged,
            this, &ExclusionGroup::onItemSelectedChanged);

    // 当前没有选中项，新增项是选中的，更新当前缓存的当前项
    if (m_current == nullptr && widget->getSelected())
    {
        setCurrent(widget);
    }
}

void ExclusionGroup::removeExclusionItem(ExclusionWidget* widget)
{
    if (!m_exclusionItems.contains(widget))
    {
        KLOG_WARNING() << "remove exclusion item failed,"
                       << widget << "not in group";
        return;
    }

    // ExclusionGroup不负责释放，释放交由ExclusionWidget的parentWidget!
    if (m_current == widget)
    {
        setCurrent(nullptr);
    }

    disconnect(widget, &ExclusionWidget::selectedStatusChanged,
               this, &ExclusionGroup::onItemSelectedChanged);
    m_exclusionItems.remove(widget);
}

void ExclusionGroup::onItemSelectedChanged(bool selected)
{
    auto item = qobject_cast<ExclusionWidget*>(sender());

    // // 只处理当前项的选中取消
    // // 非当前项不应出现取消信号
    // if (item == m_current && !selected)
    // {
    //     setCurrent(nullptr);
    // }

    // 处理非当前项目的选中信号
    if(item != m_current && selected)
    {
        setCurrent(item);
    }
}