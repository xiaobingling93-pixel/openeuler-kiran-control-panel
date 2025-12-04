/**
 * Copyright (c) 2020 ~ 2021 KylinSec Co., Ltd.
 * kiran-cpanel-group is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan
 * PSL v2. You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY
 * KIND, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO
 * NON-INFRINGEMENT, MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE. See the
 * Mulan PSL v2 for more details.
 *
 * Author:     wangshichang <shichang@isrc.iscas.ac.cn>
 */

#include "group-plugin.h"
#include "accounts-global-info.h"
#include "config.h"
#include "group-manager.h"
#include "group-page.h"
#include "group-subitem.h"
#include "logging-category.h"
#include <QLocale>

Q_LOGGING_CATEGORY(qLcGroup, "kcp.group", QtMsgType::QtDebugMsg);

GroupPlugin::GroupPlugin(QObject *parent)
    : QObject(parent)
{
}

GroupPlugin::~GroupPlugin()
{
}

int GroupPlugin::init(KiranControlPanel::PanelInterface *interface)
{
    m_panelInterface = interface;
    if (!GroupManager::instance()->init())
    {
        KLOG_ERROR() << "load group info failed!";
        return -1;
    }
    if (!AccountsGlobalInfo::instance()->init())
    {
        KLOG_ERROR() << "load user info failed!";
        return -1;
    }
    m_subitem.reset(new GroupSubItem());
    return 0;
}

void GroupPlugin::uninit()
{
}

QVector<KiranControlPanel::SubItemPtr> GroupPlugin::getSubItems()
{
    return {m_subitem};
}
