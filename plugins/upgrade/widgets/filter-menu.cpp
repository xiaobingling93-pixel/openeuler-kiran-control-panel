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

#include "filter-menu.h"
#include <kiran-log/qt5-log-i.h>
#include <QAction>
#include <QApplication>
#include <QCoreApplication>
#include <QCursor>
#include <QPaintEvent>
#include <QPainter>
#include <QStyle>
#include "def.h"

FilterMenu::FilterMenu(QWidget *parent)
    : QMenu(parent),
      m_filterTypes(AdvisoryKindHelper::getDefaultFilterTypes())  // 默认全选
{
    installEventFilter(this);

    auto flagToStringMap = AdvisoryKindHelper::getFlagToStringMap();
    for (auto it = flagToStringMap.begin(); it != flagToStringMap.end(); ++it)
    {
        auto actionStr = QCoreApplication::translate("AdvisoryKindHelper", it.value().toLatin1().data());
        auto action = new QAction(actionStr, this);
        action->setCheckable(true);
        action->setChecked(true);
        addAction(action);
        m_actionToFlag[action] = it.key();
    }
}

FilterMenu::~FilterMenu()
{
}

void FilterMenu::setFilterTypes(AdvisoryKindFlags filterTypes)
{
    if (m_filterTypes != filterTypes)
    {
        m_filterTypes = filterTypes;
        updateActionStates();
        update();  // 触发重绘
        emit filterTypesChanged(filterTypes);
    }
}

void FilterMenu::updateActionStates()
{
    // 更新每个菜单项的选中状态
    for (auto it = m_actionToFlag.begin(); it != m_actionToFlag.end(); ++it)
    {
        auto action = it.key();
        AdvisoryKindFlag flag = it.value();
        bool checked = (m_filterTypes & flag) != 0;
        action->setChecked(checked);
    }
}

void FilterMenu::paintEvent(QPaintEvent *e)
{
    QMenu::paintEvent(e);
}

bool FilterMenu::eventFilter(QObject *watched, QEvent *event)
{
    //用户多选菜单项后，不立即关闭菜单，点击空白区域再关闭
    if (watched == this && event->type() == QEvent::MouseButtonRelease)
    {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
        QPoint pos = mouseEvent->pos();
        bool ret = false;

        // 检查点击位置是否在菜单项上
        auto actions = this->actions();
        for (auto action : actions)
        {
            QRect actionRect = actionGeometry(action);
            if (actionRect.contains(pos))
            {
                action->setChecked(!action->isChecked());

                AdvisoryKindFlag flag = m_actionToFlag[action];
                AdvisoryKindFlags newFilterTypes = m_filterTypes;

                if (action->isChecked())
                {
                    newFilterTypes |= flag;  // 添加标志
                }
                else
                {
                    newFilterTypes &= ~flag;  // 移除标志
                }

                // 如果所有项都被取消选中，则选中所有项
                if (newFilterTypes == 0)
                {
                    newFilterTypes = AdvisoryKindHelper::getDefaultFilterTypes();
                }

                setFilterTypes(newFilterTypes);
                ret = true;
                break;
            }
        }
        return ret;
    }
    else
    {
        return QMenu::eventFilter(watched, event);
    }
}
