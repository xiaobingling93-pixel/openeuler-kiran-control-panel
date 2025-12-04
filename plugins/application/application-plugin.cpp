/**
 * Copyright (c) 2022 ~ 2023 KylinSec Co., Ltd.
 * kiran-control-panel is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 *
 * Author:     yinhongchang <yinhongchang@kylinsec.com.cn>
 */

#include "application-plugin.h"
#include "application-subitem.h"
#include "autostart/autostart-page.h"
#include "config.h"
#include "defaultapp/defaultapp.h"
#include "logging-category.h"

#include <kiran-log/qt5-log-i.h>
#include <kiran-session-daemon/appearance-i.h>
#include <QCoreApplication>
#include <tuple>

Q_LOGGING_CATEGORY(qLcApplication,"kcp.application",QtMsgType::QtDebugMsg)

ApplicationPlugin::ApplicationPlugin(QObject* parent) : QObject(parent)
{
}

ApplicationPlugin::~ApplicationPlugin()
{
}

int ApplicationPlugin::init(KiranControlPanel::PanelInterface* interface)
{
    initSubItem();
    return 0;
}

void ApplicationPlugin::uninit()
{
}

QVector<KiranControlPanel::SubItemPtr> ApplicationPlugin::getSubItems()
{
    return m_subitems;
}

void ApplicationPlugin::initSubItem()
{
    auto defaultAppSubItemCreater = []() -> QWidget*
    {
        return new DefaultApp();
    };
    auto autoStartSubItemCreater = []() -> QWidget*
    {
        return new AutostartPage();
    };
    struct SubItemStruct
    {
        QString id;
        QString name;
        QString category;
        QString desc;
        QString icon;
        int weight;
        CreateWidgetFunc func;
    };
    QList<SubItemStruct> subitemInfos = {
        {"DefaultApp",
         tr("DefaultApp"),
         "app-manager",
         "",
         "ksvg-kcp-app-defaultapp",
         99,
         defaultAppSubItemCreater},
        {"AutoStart",
         tr("AutoStart"),
         "app-manager",
         "",
         "ksvg-kcp-app-autostart",
         98,
         autoStartSubItemCreater}};

    for (const SubItemStruct& subitemInfo : subitemInfos)
    {
        ApplicationSubItem* subitem = new ApplicationSubItem(subitemInfo.func);

        subitem->setID(subitemInfo.id);
        subitem->setName(subitemInfo.name);
        subitem->setCategory(subitemInfo.category);
        subitem->setDesc(subitemInfo.desc);
        subitem->setIcon(subitemInfo.icon);
        subitem->setWeight(subitemInfo.weight);
        m_subitems.append(KiranControlPanel::SubItemPtr(subitem));
    }
}