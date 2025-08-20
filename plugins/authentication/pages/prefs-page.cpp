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
#include "prefs-page.h"
#include "logging-category.h"
#include "utils/auth-setting-container.h"
#include "utils/auth-setting-item.h"
#include "utils/kiran-auth-dbus-proxy.h"

#include <QCoreApplication>
#include <QBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QStackedWidget>

enum PrefsPageEnum
{
    PREFS_PAGE_AUTH_TYPE,
    PREFS_PAGE_AUTH_APPLICATION
};

QString getAuthTypeDisplayName(KADAuthType authType)
{
    static QMap<KADAuthType, QString> authAppMap = {
        {KAD_AUTH_TYPE_PASSWORD, QT_TRANSLATE_NOOP("KADAuthType","password")},
        {KAD_AUTH_TYPE_FINGERPRINT, QT_TRANSLATE_NOOP("KADAuthType","fingerprint")},
        {KAD_AUTH_TYPE_FACE, QT_TRANSLATE_NOOP("KADAuthType","face")},
        {KAD_AUTH_TYPE_UKEY, QT_TRANSLATE_NOOP("KADAuthType","ukey")},
        {KAD_AUTH_TYPE_FINGERVEIN, QT_TRANSLATE_NOOP("KADAuthType","fingervein")},
        {KAD_AUTH_TYPE_IRIS, QT_TRANSLATE_NOOP("KADAuthType","iris")}
    };
    QString key = authAppMap.value(authType, QString());
    return QCoreApplication::translate("KADAuthType", key.toUtf8().constData());
}


PrefsPage::PrefsPage(KiranAuthDBusProxy* proxy, QWidget* parent)
    : QWidget(parent),
      m_proxy(proxy)
{
    initUI();
    refreshAuthTypeEnabled();
}

PrefsPage::~PrefsPage()
{
}

void PrefsPage::initUI()
{
    auto mainLayout = new QBoxLayout(QBoxLayout::TopToBottom, this);
    mainLayout->setContentsMargins(24, 24, 24, 24);
    mainLayout->setSpacing(0);

    m_stackedWidget = new QStackedWidget(this);
    mainLayout->addWidget(m_stackedWidget);

    m_stackedWidget->insertWidget(PREFS_PAGE_AUTH_TYPE, initAuthTypePage());
    m_stackedWidget->insertWidget(PREFS_PAGE_AUTH_APPLICATION, initAuthApplicationPage());
}

QWidget* PrefsPage::initAuthTypePage()
{
    auto authTypeWidget = new QWidget(this);

    auto authTypeLayout = new QBoxLayout(QBoxLayout::TopToBottom, authTypeWidget);
    authTypeLayout->setSpacing(0);
    authTypeLayout->setContentsMargins(0, 0, 0, 0);

    auto labelDesc = new QLabel(tr("Authentication type Enabled status"));
    authTypeLayout->addWidget(labelDesc);

    authTypeLayout->addSpacerItem(new QSpacerItem(10, 10, QSizePolicy::Minimum, QSizePolicy::Fixed));

    auto container = new AuthSettingContainer(this);
    authTypeLayout->addWidget(container, 1);

    authTypeLayout->addStretch();

    // fill auth types
    auto authTypes = {
        KAD_AUTH_TYPE_FINGERPRINT,KAD_AUTH_TYPE_FINGERVEIN,
        KAD_AUTH_TYPE_UKEY,KAD_AUTH_TYPE_IRIS,
        KAD_AUTH_TYPE_FACE
    };
    for (auto type : authTypes)
    {
        auto settingsItem = new AuthSettingItem(this);
        settingsItem->setSwitcherVisible(true);
        settingsItem->setUserData(type);
        settingsItem->setSwitcherVisible(true);
        settingsItem->setClickable(true);
        settingsItem->setRightButtonVisible(true, "ksvg-arrow");
        settingsItem->setText(getAuthTypeDisplayName(type));
        container->addAuthSettingItem(settingsItem);

        auto authTypeClickedSlot = std::bind(&PrefsPage::updateCurrentAuthType, this, type);
        connect(settingsItem, &AuthSettingItem::clicked, this, authTypeClickedSlot);
        connect(settingsItem, &AuthSettingItem::rightButtonClicked, this, authTypeClickedSlot);

        auto authTypeToggledSlot = std::bind(&PrefsPage::updateAuthTypeEnable, this, type, std::placeholders::_2);
        connect(settingsItem, &AuthSettingItem::switchButtonToggled, this, authTypeToggledSlot);

        m_authTypeMap[type] = settingsItem;
    }

    return authTypeWidget;
}

QWidget* PrefsPage::initAuthApplicationPage()
{
    auto authApplicationPrefs = new QWidget(this);

    auto authAppLayout = new QBoxLayout(QBoxLayout::TopToBottom, authApplicationPrefs);
    authAppLayout->setSpacing(0);
    authAppLayout->setContentsMargins(0, 0, 0, 16);

    m_labelAuthApp = new QLabel(tr("..."));
    authAppLayout->addWidget(m_labelAuthApp);

    authAppLayout->addSpacerItem(new QSpacerItem(10, 10, QSizePolicy::Minimum, QSizePolicy::Fixed));

    auto container = new AuthSettingContainer(this);
    authAppLayout->addWidget(container, 1);

    authAppLayout->addStretch();

    auto buttonLayout = new QBoxLayout(QBoxLayout::LeftToRight);
    buttonLayout->addStretch();

    auto returnButton = new QPushButton(this);
    returnButton->setFixedSize(110, 40);
    returnButton->setText(tr("Return"));
    buttonLayout->addWidget(returnButton);
    connect(returnButton, &QPushButton::clicked, this, &PrefsPage::toPrefsHomePage);

    buttonLayout->addStretch();
    authAppLayout->addLayout(buttonLayout);

    std::list<std::tuple<KADAuthApplication, QString>> authTypes = {
        std::make_tuple(KAD_AUTH_APPLICATION_LOGIN, tr("login")),
        std::make_tuple(KAD_AUTH_APPLICATION_UNLOCK, tr("unlock")),
        std::make_tuple(KAD_AUTH_APPLICATION_EMPOWERMENT, tr("empowerment"))};

    // fill auth applications
    for (auto iter : authTypes)
    {
        auto authApp = std::get<0>(iter);
        auto settingsItem = new AuthSettingItem(this);

        settingsItem->setUserData(authApp);
        settingsItem->setSwitcherVisible(true);
        settingsItem->setText(std::get<1>(iter));
        container->addAuthSettingItem(settingsItem);

        auto slot = std::bind(&PrefsPage::updateAuthTypeEnableForApp, this, authApp, std::placeholders::_2);
        connect(settingsItem, &AuthSettingItem::switchButtonToggled, this, slot);

        m_authAppMap[authApp] = settingsItem;
    }

    return authApplicationPrefs;
}

void PrefsPage::refreshAuthTypeEnabled()
{
    for (auto iter = m_authTypeMap.begin(); iter != m_authTypeMap.end(); iter++)
    {
        auto authType = iter.key();
        auto settingItem = iter.value();

        QSignalBlocker blocker(settingItem);

        bool enable = m_proxy->getAuthTypeEnabled(authType);
        settingItem->setSwitcherChecked(enable);
    }
}

void PrefsPage::refreshAuthApplicationEanbled()
{
    QString authName = getAuthTypeDisplayName(m_currentAuthType);
    QString desc = QString(tr("Apply the %1 authentication to the following applications").arg(authName));
    m_labelAuthApp->setText(desc);

    for (auto iter = m_authAppMap.begin(); iter != m_authAppMap.end(); iter++)
    {
        auto authApp = iter.key();
        auto settingItem = iter.value();

        QSignalBlocker blocker(settingItem);

        bool enable = m_proxy->getAuthTypeEnabledForApp(m_currentAuthType, authApp);
        settingItem->setSwitcherChecked(enable);
    }
}

void PrefsPage::updateCurrentAuthType(KADAuthType authType)
{
    KLOG_INFO(qLcAuthentication) << "update current auth type" << authType;

    m_currentAuthType = authType;
    m_stackedWidget->setCurrentIndex(PREFS_PAGE_AUTH_APPLICATION);

    refreshAuthApplicationEanbled();
}

void PrefsPage::updateAuthTypeEnable(KADAuthType authType, bool checked)
{
    KLOG_INFO(qLcAuthentication) << "update auth type" << authType << checked;

    m_proxy->setAuthTypeEnabled(authType, checked);
    refreshAuthTypeEnabled();
}

void PrefsPage::updateAuthTypeEnableForApp(KADAuthApplication authApp, bool checked)
{
    KLOG_INFO(qLcAuthentication) << "update auth type" << m_currentAuthType << checked
                                 << "for" << authApp;
    m_proxy->setAuthTypeEnabledForApp(m_currentAuthType, authApp, checked);
    refreshAuthApplicationEanbled();
}

void PrefsPage::toPrefsHomePage()
{
    m_stackedWidget->setCurrentIndex(PREFS_PAGE_AUTH_TYPE);
}