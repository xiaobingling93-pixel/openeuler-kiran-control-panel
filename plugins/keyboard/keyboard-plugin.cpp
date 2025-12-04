/**
 *  * Copyright (c) 2020 ~ 2022 KylinSec Co., Ltd.
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
#include "keyboard-plugin.h"
#include "general-subitem.h"
#include "layout-subitem.h"
#include "logging-category.h"

#include <kiran-session-daemon/keyboard-i.h>
#include <QCoreApplication>
#include <QDBusConnection>
#include <QDBusConnectionInterface>

Q_LOGGING_CATEGORY(qLcKeyboard, "kcp.keyboard", QtMsgType::QtDebugMsg);

KeyboardPlugin::KeyboardPlugin(QObject* parent)
    : QObject(parent)
{
}

KeyboardPlugin::~KeyboardPlugin()
{
}

// 主面板调用该接口初始化该插件，插件可在其中进行部分初始化操作，例如安装翻译等操作
// 成功返回0
int KeyboardPlugin::init(KiranControlPanel::PanelInterface* interface)
{
    if (!QDBusConnection::sessionBus().interface()->isServiceRegistered(KEYBOARD_DBUS_NAME).value())
    {
        KLOG_INFO() << "Connect keyboard dbus service failed!";
        return -1;
    }

    m_subitems.append(KiranControlPanel::SubItemPtr(new GeneralSubItem));
    m_subitems.append(KiranControlPanel::SubItemPtr(new LayoutSubItem));
    return 0;
}

// 主面板调用该接口取消掉该插件初始化做的操作并卸载该插件
void KeyboardPlugin::uninit()
{
}

// 功能项数组，生存周期由插件维护
// 功能项发生变更时，应调用init时传入KcpInterface接口，通知主面板相关信息变更,及时加载新的功能项信息
QVector<KiranControlPanel::SubItemPtr> KeyboardPlugin::getSubItems()
{
    return m_subitems;
}