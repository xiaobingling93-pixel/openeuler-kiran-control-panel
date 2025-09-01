/**
 * Copyright (c) 2020 ~ 2024 KylinSec Co., Ltd.
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
#include "general-bio-page.h"
#include "auth-setting-container.h"
#include "auth-setting-item.h"
#include "checkpasswd-dialog.h"
#include "logging-category.h"
#include "utils/kiran-auth-dbus-proxy.h"

#include <kiran-message-box.h>
#include <kiran-input-dialog.h>
#include <kiran-push-button.h>
#include <qt5-log-i.h>
#include <QBoxLayout>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QTime>
#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
#include <QRandomGenerator>
#endif

#define MAX_FEATURE_NUMBER 1000

GeneralBioPage::GeneralBioPage(KiranAuthDBusProxy* proxy, KADAuthType authType, QWidget* parent)
    : QWidget(parent),
      m_proxy(proxy),
      m_authType(authType)
{
    initUI();

    refreshFeature();
    refreshDeviceComboBox();
}

GeneralBioPage::~GeneralBioPage()
{
}

void GeneralBioPage::setFeatureNamePrefix(const QString& prefix)
{
    m_featureNamePrefix = prefix;
}

QString GeneralBioPage::autoGenerateFeatureName()
{
    if (m_featureNamePrefix.isEmpty())
    {
        KLOG_WARNING(qLcAuthentication) << "generate feature name prefix is not set!";
    }

    for (int i = 0; i <= 10; ++i)
    {
// sonarqube block off
#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
        auto featureNumber = QRandomGenerator::global()->bounded(1, MAX_FEATURE_NUMBER);
#else
        qsrand(QTime(0, 0, 0).secsTo(QTime::currentTime()));
        auto featureNumber = qrand() % MAX_FEATURE_NUMBER + 1;
#endif
        // sonarqube block on
        auto temp = QString("%1 %2").arg(m_featureNamePrefix).arg(featureNumber);

        if (!m_featureNameSet.contains(temp))
        {
            return temp;
        }
    }

    return QString();
}

void GeneralBioPage::addFeature(const QString& featureName, const QString& featureIID)
{
    auto featureItem = new AuthSettingItem;
    featureItem->setUserData(featureIID);
    featureItem->setText(featureName);
    featureItem->setLeftButtonVisible(true, "ksvg-rename");
    featureItem->setRightButtonVisible(true, "ksvg-trash");

    m_featureContainer->addAuthSettingItem(featureItem);

    connect(featureItem, &AuthSettingItem::leftButtonClicked, this, &GeneralBioPage::renameFeature);
    connect(featureItem, &AuthSettingItem::rightButtonClicked, this, &GeneralBioPage::removeFeature);
}

int GeneralBioPage::addDeviceItem(const QString& deviceName, const QString& deviceID)
{
    m_comboDefaultDevice->addItem(deviceName, deviceID);
    return m_comboDefaultDevice->count() - 1;
}

void GeneralBioPage::setDefaultDeviceLabelDesc(const QString& text)
{
    m_labelDefaultDevice->setText(text);
}

void GeneralBioPage::setDeviceFeatureListDesc(const QString& text)
{
    m_labelFeatureList->setText(text);
}

void GeneralBioPage::refreshFeature()
{
    m_featureContainer->clear();
    m_featureNameSet.clear();

    auto identifications = m_proxy->getUserIdentifications(m_authType);
    for (auto iter : identifications)
    {
        m_featureNameSet << iter.name;
        addFeature(iter.name, iter.IID);
    }
}

void GeneralBioPage::refreshDeviceComboBox()
{
    QSignalBlocker blocker(m_comboDefaultDevice);

    m_comboDefaultDevice->clear();

    auto devices = m_proxy->getDevicesByType(m_authType);
    auto defaultDeviceID = m_proxy->getDefaultDeviceID(m_authType);

    for (auto device : devices)
    {
        auto idx = addDeviceItem(device.name, device.id);
        if (!defaultDeviceID.isEmpty() && defaultDeviceID == device.id)
        {
            m_comboDefaultDevice->setCurrentIndex(idx);
        }
    }
}

int GeneralBioPage::getDeviceCount()
{
    return m_comboDefaultDevice->count();
}

void GeneralBioPage::updateDefaultDevice(int idx)
{
    auto combo = qobject_cast<QComboBox*>(sender());
    auto name = combo->itemText(idx);
    auto id = combo->itemData(idx).toString();

    m_proxy->setDefaultDeviceID(m_authType, id);
}

void GeneralBioPage::renameFeature()
{
    auto settingItem = qobject_cast<AuthSettingItem*>(sender());
    auto iid = settingItem->getUserData().toString();
    auto name = settingItem->getText();

    KLOG_INFO(qLcAuthentication) << "start rename feature"
                                 << "iid" << iid << ","
                                 << "name" << name;

    KiranInputDialog renameDialog(this);
    renameDialog.setTitle(tr("Rename Feature"));
    renameDialog.setDesc(tr("Please enter the renamed feature name"));
    renameDialog.setInputMode(QLineEdit::Normal, 32);
    if (!renameDialog.exec())
    {
        KLOG_INFO(qLcAuthentication) << "cancel rename feature";
        return;
    }

    QString newName = renameDialog.getUserInput();
    m_proxy->renameIdentification(iid, newName);

    KLOG_INFO(qLcAuthentication) << "feature renamed:" << iid
                                 << name << "->" << newName;
    refreshFeature();
}

void GeneralBioPage::removeFeature()
{
    auto settingItem = qobject_cast<AuthSettingItem*>(sender());
    auto iid = settingItem->getUserData().toString();
    auto name = settingItem->getText();

    KLOG_INFO(qLcAuthentication) << "remove feature"
                                 << "idd" << iid << ","
                                 << "name" << name;

    QString text;
    if (m_authType == KAD_AUTH_TYPE_UKEY)
    {
        text = QString(tr("Are you sure you want to delete the feature called %1, "
                          "Ensure that the Ukey device is inserted; "
                          "otherwise the information stored in the Ukey will not be deleted"))
                   .arg(name);
    }
    else
    {
        text = QString(tr("Are you sure you want to delete the feature called %1")).arg(name);
    }

    if (KiranMessageBox::message(this, tr("tips"), text, KiranMessageBox::Yes | KiranMessageBox::No) != KiranMessageBox::Yes)
    {
        KLOG_INFO(qLcAuthentication) << "remove feature canceld";
        return;
    }

    m_proxy->deleteIdentification(iid);
    KLOG_INFO(qLcAuthentication) << "feature" << iid << name << "deleted";

    refreshFeature();
}

void GeneralBioPage::startEnrollFeature()
{
    CheckpasswdDialog dialog;
    if (!dialog.exec())
    {
        return;
    }
    dialog.hide();

    auto passwd = dialog.getUserInput();
    if (!dialog.checkPasswd(passwd))
    {
        KiranMessageBox::message(this, tr("Error"),
                                 tr(" Failed to enroll feature because the password verification failed！"),
                                 KiranMessageBox::Ok);
        return;
    }

    emit enrollFeature();
}

void GeneralBioPage::initUI()
{
    auto featureManagerLayout = new QBoxLayout(QBoxLayout::TopToBottom, this);
    featureManagerLayout->setSpacing(0);
    featureManagerLayout->setContentsMargins(0, 0, 0, 0);

    m_labelDefaultDevice = new QLabel(tr("default device"));
    featureManagerLayout->addWidget(m_labelDefaultDevice);

    featureManagerLayout->addSpacerItem(new QSpacerItem(10, 10, QSizePolicy::Minimum, QSizePolicy::Fixed));

    m_comboDefaultDevice = new QComboBox(this);
    featureManagerLayout->addWidget(m_comboDefaultDevice);
    connect(m_comboDefaultDevice, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &GeneralBioPage::updateDefaultDevice);

    featureManagerLayout->addSpacerItem(new QSpacerItem(10, 16, QSizePolicy::Minimum, QSizePolicy::Fixed));

    m_labelFeatureList = new QLabel(tr("feature list"));
    featureManagerLayout->addWidget(m_labelFeatureList);

    featureManagerLayout->addSpacerItem(new QSpacerItem(10, 10, QSizePolicy::Minimum, QSizePolicy::Fixed));

    m_featureContainer = new AuthSettingContainer(this);
    featureManagerLayout->addWidget(m_featureContainer, 1);

    featureManagerLayout->addSpacerItem(new QSpacerItem(10, 10, QSizePolicy::Minimum, QSizePolicy::Fixed));

    auto addButton = new QPushButton(this);
    featureManagerLayout->addWidget(addButton);
    addButton->setIcon(QPixmap(":/kcp-keyboard/images/addition.svg"));

    KiranPushButton::setButtonType(addButton, KiranPushButton::BUTTON_Default);
    connect(addButton, &QPushButton::clicked, this, &GeneralBioPage::startEnrollFeature);

    featureManagerLayout->addStretch();
}
