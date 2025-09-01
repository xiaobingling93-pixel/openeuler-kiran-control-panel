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
#include "ukey-page.h"
#include "logging-category.h"
#include "utils/auth-setting-container.h"
#include "utils/auth-setting-item.h"
#include "utils/general-bio-page.h"
#include "utils/kiran-auth-dbus-proxy.h"

#include <kiran-authentication-service/kas-authentication-i.h>
#include <kiran-message-box.h>
#include <kiran-input-dialog.h>
#include <qt5-log-i.h>
#include <QBoxLayout>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QLabel>
#include <QPushButton>
#include <QSpacerItem>

UKeyPage::UKeyPage(KiranAuthDBusProxy* proxy, QWidget* parent)
    : QWidget(parent),
      m_proxy(proxy)
{
    initUI();
    connect(m_proxy, &KiranAuthDBusProxy::EnrollStatus, this, &UKeyPage::updateEnrollStatus);
}

UKeyPage::~UKeyPage()
{
}

void UKeyPage::initUI()
{
    auto mainLayout = new QBoxLayout(QBoxLayout::TopToBottom, this);
    mainLayout->setContentsMargins(24, 24, 24, 24);
    mainLayout->setSpacing(0);

    m_featureManager = new GeneralBioPage(m_proxy, KAD_AUTH_TYPE_UKEY, this);
    m_featureManager->setFeatureNamePrefix(tr("Ukey"));
    m_featureManager->setDefaultDeviceLabelDesc(tr("Default Ukey device"));
    m_featureManager->setDeviceFeatureListDesc(tr("List of devices bound to the Ukey"));
    connect(m_featureManager, &GeneralBioPage::enrollFeature, this, &UKeyPage::startEnroll);

    mainLayout->addWidget(m_featureManager);
}

bool UKeyPage::doEnroll(bool rebinding)
{
    QString errorString;
    auto randomFeatureName = m_featureManager->autoGenerateFeatureName();

    QJsonDocument jsonDoc(QJsonObject{{"ukey", QJsonObject{{"pin", m_pinCode}, {"rebinding", rebinding}}}});
    if (!m_proxy->startEnroll(KAD_AUTH_TYPE_UKEY, randomFeatureName, jsonDoc.toJson().data(), errorString))
    {
        KLOG_ERROR(qLcAuthentication,
                   "ukey start enroll feature(%s) failed,%s!",
                   randomFeatureName.toStdString().c_str(),
                   errorString.toStdString().c_str());

        QString tips = QString("Failed to record the UKey device %1").arg(errorString.isEmpty() ? "." : errorString);
        KiranMessageBox::message(this, tr("error"), tips, KiranMessageBox::Ok);
        return false;
    }

    KLOG_INFO(qLcAuthentication,
              "ukey start enroll feature(%s) success",
              randomFeatureName.toStdString().c_str());
    return true;
}

void UKeyPage::startEnroll()
{
    if (m_featureManager->getDeviceCount() == 0)
    {
        KiranMessageBox::message(this, tr("error"),
                                 tr("No UKey device detected, pelease insert the UKey device and perform operations"),
                                 KiranMessageBox::Ok);
        return;
    }

    KiranInputDialog dialog;
    dialog.setTitle(tr("UKey Enroll"));
    dialog.setDesc(tr("Please enter the ukey pin code"));
    dialog.setInputMode(QLineEdit::Password, 32);
    if (!dialog.exec())
    {
        return;
    }

    m_pinCode = dialog.getUserInput();
    doEnroll(false);
}

void UKeyPage::updateEnrollStatus(const QString& iid, bool isComplete,
                                  int progress, const QString& message)
{
    KLOG_DEBUG(qLcAuthentication,
               "ukey enroll status: iid(%s),isCompelte(%s),progress(%d),message(%s)",
               iid.toStdString().c_str(),
               isComplete ? "true" : "false",
               progress,
               message.toStdString().c_str());

    if (isComplete)
    {
        if (progress == 100)
        {
            m_featureManager->refreshFeature();
        }
        else
        {
            QString tips = QString("Failed to record UKey device features %1").arg(message.isEmpty() ? "." : message);
            KiranMessageBox::message(this, tr("error"), tips, KiranMessageBox::Ok);
        }
    }
}