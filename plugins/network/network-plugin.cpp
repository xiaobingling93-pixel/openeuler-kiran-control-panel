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
#include "network-plugin.h"
#include <qt5-log-i.h>
#include <QCoreApplication>
#include <QEvent>
#include "config.h"
#include "connection-manager.h"
#include "device-manager.h"
#include "logging-category.h"
#include "network-widget.h"
#include "page-manager.h"

Q_LOGGING_CATEGORY(qLcNetwork, "kcp.network", QtMsgType::QtDebugMsg);

namespace Kiran
{
namespace Network
{
Plugin::Plugin(QObject *parent) : QObject(parent)
{
}

Plugin::~Plugin()
{
}

int Plugin::init(KiranControlPanel::PanelInterface *interface)
{
    DeviceManager::global_init();
    PageManager::global_init(DM_INSTANCE);
    ConnectionManager::global_init();
    auto subItem = new SubItem(interface, this);
    m_subitem.reset(subItem);
    return 0;
}

void Plugin::uninit()
{
    m_subitem.reset();
    PageManager::global_deinit();
    DeviceManager::global_deinit();
    ConnectionManager::global_deinit();
}

QVector<KiranControlPanel::SubItemPtr> Plugin::getSubItems()
{
    return {m_subitem};
}

SubItem::SubItem(KiranControlPanel::PanelInterface *interface, QObject *parent)
    : QObject(parent),
      m_interface(interface)
{
    connect(PageManager::instance(), &PageManager::pageAvailableChanged, this, &SubItem::processPageAvailableChanged);
    processPageAvailableChanged();
}

SubItem::~SubItem()
{
}

QWidget *SubItem::createWidget()
{
    m_widget = new Widget();
    return m_widget;
}

QVector<QPair<QString, QString>> SubItem::getSearchKeys()
{
    return m_searchKeys;
}

bool SubItem::jumpToSearchEntry(const QString &key)
{
    if (!m_widget)
    {
        return false;
    }

    return m_widget->jumpToSearchEntry(key);
}

void SubItem::processPageAvailableChanged()
{
    QVector<QPair<QString, QString>> searchKeywords;
    for (int i = 0; i < PAGE_LAST; i++)
    {
        auto pageType = static_cast<PageType>(i);
        if (PageManager::instance()->isAvaliable(pageType))
        {
            auto pageInfo = PageManager::instance()->description(pageType);
            searchKeywords << qMakePair<QString, QString>(pageInfo.name, pageInfo.key);
        }
    }
    m_searchKeys = searchKeywords;
    m_interface->handlePluginSubItemChanged();
}
}  // namespace Network
}  // namespace Kiran