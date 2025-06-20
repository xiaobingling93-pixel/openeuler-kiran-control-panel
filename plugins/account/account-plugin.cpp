/**
 * Copyright (c) 2020 ~ 2022 KylinSec Co., Ltd.
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

#include "account-plugin.h"
#include "accounts-global-info.h"
#include "config.h"
#include "temporary-dir-manager.h"
#include "logging-category.h"

#include <QApplication>
#include <QLocale>

AccountPlugin::AccountPlugin(QObject* parent)
    : QObject(parent)
{
}

AccountPlugin ::~AccountPlugin()
{
}

int AccountPlugin::init(KiranControlPanel::PanelInterface* interface)
{
    m_panelInterface = interface;

    auto showRoot = m_panelInterface->queryCofnig("showRootAccount",false).toBool();
    auto showPasswordExpirationPolicy = m_panelInterface->queryCofnig("showPasswordExpirationPolicy",false).toBool();
    if (!AccountsGlobalInfo::instance()->init(showRoot,showPasswordExpirationPolicy))
    {
        KLOG_ERROR(qLcAccount) << "load user info failed!";
        return -1;
    }

    if (!TemporaryDirManager::instance()->init(qAppName()))
    {
        KLOG_ERROR(qLcAccount) << "init temporary dir manager failed!";
        return -1;
    }

    m_subitem.reset(new AccountSubItem(interface, this));
    return 0;
}

void AccountPlugin::uninit()
{
}

QVector<KiranControlPanel::SubItemPtr> AccountPlugin::getSubItems()
{
    return {m_subitem};
}