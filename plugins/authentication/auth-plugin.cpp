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
#include "auth-plugin.h"
#include "config.h"
#include "logging-category.h"
#include "plugin-subitem.h"
#include "utils/kiran-auth-dbus-proxy.h"

#include "pages/driver-page.h"
#include "pages/face-page.h"
#include "pages/finger-page.h"
#include "pages/iris-page.h"
#include "pages/prefs-page.h"
#include "pages/ukey-page.h"

#include <QCoreApplication>

Q_LOGGING_CATEGORY(qLcAuthentication,"kcp.authentication",QtMsgType::QtDebugMsg)

AuthPlugin::AuthPlugin(QObject* parent)
    : QObject(parent)
{
}

AuthPlugin ::~AuthPlugin()
{
}

int AuthPlugin::init(KiranControlPanel::PanelInterface* interface)
{
    m_panelInterface = interface;
    KiranAuthDBusProxy::globalInit();
    initItems();

    return 0;
}

void AuthPlugin::uninit()
{
    KiranAuthDBusProxy::globalDeinit();
}

QVector<KiranControlPanel::SubItemPtr> AuthPlugin::getSubItems()
{
    return m_subitems;
}

void AuthPlugin::initItems()
{
    auto FingerprintPageCreator = []() -> QWidget*
    {
        return new FingerPage(KiranAuthDBusProxy::getInstance(), FingerPage::FINGER_TYPE_FINGER_PRINT);
    };
    auto FingerveinPageCreator = []() -> QWidget*
    {
        return new FingerPage(KiranAuthDBusProxy::getInstance(), FingerPage::FINGER_TYPE_FINGER_VEIN);
    };
    auto UKeyPageCreator = []() -> QWidget*
    {
        return new UKeyPage(KiranAuthDBusProxy::getInstance());
    };
    auto IrisPageCreator = []() -> QWidget*
    {
        return new IrisPage(KiranAuthDBusProxy::getInstance());
    };
    auto FacePageCreator = []() -> QWidget*
    {
        return new FacePage(KiranAuthDBusProxy::getInstance());
    };
    auto DriverPageCreator = []() -> QWidget*
    {
        return new DriverPage(KiranAuthDBusProxy::getInstance());
    };
    auto PrefsPageCreator = []() -> QWidget*
    {
        return new PrefsPage(KiranAuthDBusProxy::getInstance());
    };

    m_subitems = {
        KiranControlPanel::SubItemPtr(new PluginSubItem("Fingerprint",
                                                        tr("Fingerprint"),
                                                        "authentication-manager",
                                                        "",
                                                        "ksvg-kcp-authentication-fingerprint",
                                                        99,
                                                        FingerprintPageCreator)),
        KiranControlPanel::SubItemPtr(new PluginSubItem("FingerVein",
                                                        tr("FingerVein"),
                                                        "authentication-manager",
                                                        "",
                                                        "ksvg-kcp-authentication-fingervein",
                                                        98,
                                                        FingerveinPageCreator)),
        KiranControlPanel::SubItemPtr(new PluginSubItem("UKey",
                                                        tr("UKey"),
                                                        "authentication-manager",
                                                        "",
                                                        "ksvg-kcp-authentication-ukey",
                                                        97,
                                                        UKeyPageCreator)),
        KiranControlPanel::SubItemPtr(new PluginSubItem("Iris",
                                                        tr("Iris"),
                                                        "authentication-manager",
                                                        "",
                                                        "ksvg-kcp-authentication-iris",
                                                        98,
                                                        IrisPageCreator)),
        KiranControlPanel::SubItemPtr(new PluginSubItem("Face",
                                                        tr("Face"),
                                                        "authentication-manager",
                                                        "",
                                                        "ksvg-kcp-authentication-face",
                                                        98,
                                                        FacePageCreator)),
        KiranControlPanel::SubItemPtr(new PluginSubItem("Driver",
                                                        tr("Driver Manager"),
                                                        "authentication-manager",
                                                        "",
                                                        "ksvg-kcp-authentication-driver",
                                                        97,
                                                        DriverPageCreator)),
        KiranControlPanel::SubItemPtr(new PluginSubItem("Prefs",
                                                        tr("Prefs"),
                                                        "authentication-manager",
                                                        "",
                                                        "ksvg-kcp-authentication-prefs",
                                                        96,
                                                        PrefsPageCreator))};
}