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

#include "upgrade-subitem.h"
#include <QEvent>
#include "upgrade-page.h"

UpgradeSubItem::UpgradeSubItem(QObject* parent)
    : QObject(parent), m_upgradePage(nullptr)
{
}

UpgradeSubItem::~UpgradeSubItem()
{
}

bool UpgradeSubItem::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == m_upgradePage && event->type() == QEvent::Destroy)
    {
        m_upgradePage = nullptr;
    }

    return QObject::eventFilter(watched, event);
}

QString UpgradeSubItem::getID()
{
    return "Upgrade";
}

QString UpgradeSubItem::getName()
{
    return tr("Upgrade");
}

QString UpgradeSubItem::getCategory()
{
    return "about-system";
}

QString UpgradeSubItem::getDesc()
{
    return "";
}

QString UpgradeSubItem::getIcon()
{
    return "ksvg-kcp-upgrade";
}

int UpgradeSubItem::getWeight()
{
    return 1;
}

QWidget* UpgradeSubItem::createWidget()
{
    m_upgradePage = new UpgradePage();
    m_upgradePage->installEventFilter(this);
    return m_upgradePage;
}

QVector<QPair<QString, QString>> UpgradeSubItem::getSearchKeys()
{
    return {};
}
