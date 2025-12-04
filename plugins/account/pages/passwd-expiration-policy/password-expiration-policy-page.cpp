/**
 * Copyright (c) 2020 ~ 2021 KylinSec Co., Ltd.
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

#include "password-expiration-policy-page.h"
#include "logging-category.h"
#include "ui_password-expiration-policy-page.h"
#include "account.h"

#include <kiran-push-button.h>
#include <kiran-switch-button.h>
#include <kiran-system-daemon/accounts-i.h>
#include <qt5-log-i.h>
#include <QDBusPendingReply>
#include <QJsonDocument>

PasswordExpirationPolicyPage::PasswordExpirationPolicyPage(QWidget *parent)
    : QWidget(parent),
      ui(new Ui::PasswordExpirationPolicyPage)
{
    ui->setupUi(this);
    initUI();
}

PasswordExpirationPolicyPage::~PasswordExpirationPolicyPage()
{
    delete ui;
}

void PasswordExpirationPolicyPage::setCurrentUser(const QString &objectPath)
{
    if (!m_userProxy.isNull())
    {
        disconnect(m_userProxy.data(), &AccountUserInterface::dbusPropertyChanged,
                   this, &PasswordExpirationPolicyPage::slotUserPropertyChanged);
        m_userProxy.clear();
    }

    m_userProxy = DBusWrapper::Account::userInterface(objectPath);
    connect(m_userProxy.data(), &AccountUserInterface::dbusPropertyChanged,
            this, &PasswordExpirationPolicyPage::slotUserPropertyChanged);

    updateInfo();
}

void PasswordExpirationPolicyPage::initUI()
{
    KiranPushButton::setButtonType(ui->btn_save, KiranPushButton::BUTTON_Default);

    // 用户过期时间
    m_userExpiresSwitch = new KiranSwitchButton(this);
    ui->layout_labelUserExpires->addWidget(m_userExpiresSwitch);
    ui->dateEdit_userExpires->setMinimumDate(QDate::currentDate());
    connect(m_userExpiresSwitch, &QAbstractButton::toggled, this, &PasswordExpirationPolicyPage::handleUserExpireSwitchToggled);

    // 上次密码修改时间
    ui->label_lastPasswdChange->setText("");

    // 密码最大有效天数
    m_passwdMaxVaildDaysSwtich = new KiranSwitchButton(this);
    ui->layout_PwdMaximumVaildDays->addWidget(m_passwdMaxVaildDaysSwtich);
    ui->spinBox_passwdMaxVaildDays->setSuffix(tr("day"));
    ui->spinBox_passwdMaxVaildDays->setMaximum(99998);  /// 99999被认为是无限期
    connect(m_passwdMaxVaildDaysSwtich, &QAbstractButton::toggled, this, &PasswordExpirationPolicyPage::handlePasswdMaxVaildDaysSwitchToggled);

    // 密码过期之前多少天提醒
    m_promptBeforePasswdExpirationSwitch = new KiranSwitchButton(this);
    ui->layout_passwdExpirationPromptTime->addWidget(m_promptBeforePasswdExpirationSwitch);
    ui->spinBox_promptTime->setSuffix(tr("day"));
    ui->spinBox_promptTime->setMaximum(99998);
    ui->spinBox_promptTime->setMinimum(1);
    connect(m_promptBeforePasswdExpirationSwitch, &QAbstractButton::toggled, this, &PasswordExpirationPolicyPage::handlePromptTimeSwitchToggled);

    // 密码过期多久被认为密码失效
    m_passwdInactiveSwitch = new KiranSwitchButton(this);
    ui->layout_passwdInactive->addWidget(m_passwdInactiveSwitch);
    ui->spinBox_inactiveDays->setSuffix(tr("day"));
    ui->spinBox_inactiveDays->setMaximum(365);
    ui->spinBox_inactiveDays->setMinimum(0);
    connect(m_passwdInactiveSwitch, &QAbstractButton::toggled, this, &PasswordExpirationPolicyPage::handleInactiveDaysSwitchToggled);

    connect(ui->btn_return, &QPushButton::clicked, [this]()
            { emit sigReturn(); });
    connect(ui->btn_save, &QPushButton::clicked, [this]()
            { save(); });
}

void PasswordExpirationPolicyPage::slotUserPropertyChanged(QString propertyName, QVariant value)
{
    // TODO:监控密码过期策略变化
}

void PasswordExpirationPolicyPage::updateInfo()
{
    QString policy = m_userProxy->password_expiration_policy();
    QJsonDocument jsonDoc = QJsonDocument::fromJson(policy.toUtf8());
    QJsonObject jsonObject = jsonDoc.object();

    KLOG_DEBUG(qLcAccount) << "update passwd expiration policy page: \n\t" << policy;

    QVariantMap varMap = jsonObject.toVariantMap();
    if (!varMap.contains(ACCOUNTS_PEP_EXPIRATION_TIME) ||
        !varMap.contains(ACCOUNTS_PEP_LAST_CHANGED_TIME) ||
        !varMap.contains(ACCOUNTS_PEP_MIN_DAYS) ||
        !varMap.contains(ACCOUNTS_PEP_MAX_DAYS) ||
        !varMap.contains(ACCOUNTS_PEP_DAYS_TO_WARN) ||
        !varMap.contains(ACCOUNTS_PEP_INACTIVE_DAYS))
    {
        KLOG_ERROR(qLcAccount) << "password expiration policy format error!";
        return;
    }

    qlonglong userExpirationTime = varMap[ACCOUNTS_PEP_EXPIRATION_TIME].toLongLong();
    qulonglong lastChangedTime = varMap[ACCOUNTS_PEP_LAST_CHANGED_TIME].toLongLong();
    qlonglong maxVaildDays = varMap[ACCOUNTS_PEP_MAX_DAYS].toLongLong();
    qlonglong promptDays = varMap[ACCOUNTS_PEP_DAYS_TO_WARN].toLongLong();
    qlonglong inactiveDays = varMap[ACCOUNTS_PEP_INACTIVE_DAYS].toLongLong();

    KLOG_DEBUG(qLcAccount) << "user expiration time:" << userExpirationTime;
    KLOG_DEBUG(qLcAccount) << "last changed timt:" << lastChangedTime;
    KLOG_DEBUG(qLcAccount) << "max valid days:" << maxVaildDays;
    KLOG_DEBUG(qLcAccount) << "prompt days:" << promptDays;
    KLOG_DEBUG(qLcAccount) << "inactive days:" << inactiveDays;

    if (userExpirationTime == -1)
    {
        m_userExpiresSwitch->setChecked(false);
        ui->dateEdit_userExpires->setVisible(false);
        QDate date = QDate::currentDate();
        date = date.addYears(1);
        ui->dateEdit_userExpires->setDate(date);
    }
    else
    {
        m_userExpiresSwitch->setChecked(true);
        ui->dateEdit_userExpires->setVisible(true);
        QDate tempDate(1970, 01, 01);
        tempDate = tempDate.addDays(userExpirationTime);
        ui->dateEdit_userExpires->setDate(tempDate);
    }

    QDate changePasswdDate(1970, 01, 01);
    changePasswdDate = changePasswdDate.addDays(lastChangedTime);
    ui->label_lastPasswdChange->setText(changePasswdDate.toString("yyyy-MM-dd"));

    if (maxVaildDays == 99999)
    {
        m_passwdMaxVaildDaysSwtich->setChecked(false);
        ui->spinBox_passwdMaxVaildDays->setVisible(false);
        ui->spinBox_passwdMaxVaildDays->setValue(365);
    }
    else
    {
        m_passwdMaxVaildDaysSwtich->setChecked(true);
        ui->spinBox_passwdMaxVaildDays->setVisible(true);
        ui->spinBox_passwdMaxVaildDays->setValue(maxVaildDays);
    }

    if (promptDays == 0)
    {
        m_promptBeforePasswdExpirationSwitch->setChecked(false);
        ui->spinBox_promptTime->setVisible(false);
        ui->spinBox_promptTime->setValue(7);
    }
    else
    {
        m_promptBeforePasswdExpirationSwitch->setChecked(true);
        ui->spinBox_promptTime->setVisible(true);
        ui->spinBox_promptTime->setValue(promptDays);
    }

    if (inactiveDays == -1)
    {
        m_passwdInactiveSwitch->setChecked(false);
        ui->spinBox_inactiveDays->setVisible(false);
        ui->spinBox_inactiveDays->setValue(7);
    }
    else
    {
        m_passwdInactiveSwitch->setChecked(true);
        ui->spinBox_inactiveDays->setVisible(true);
        ui->spinBox_inactiveDays->setValue(inactiveDays);
    }
}

void PasswordExpirationPolicyPage::handleUserExpireSwitchToggled(bool checked)
{
    ui->dateEdit_userExpires->setVisible(checked);
}
void PasswordExpirationPolicyPage::handlePasswdMaxVaildDaysSwitchToggled(bool checked)
{
    ui->spinBox_passwdMaxVaildDays->setVisible(checked);
}

void PasswordExpirationPolicyPage::handlePromptTimeSwitchToggled(bool checked)
{
    ui->spinBox_promptTime->setVisible(checked);
}

void PasswordExpirationPolicyPage::handleInactiveDaysSwitchToggled(bool checked)
{
    ui->spinBox_inactiveDays->setVisible(checked);
}

void PasswordExpirationPolicyPage::save()
{
    QJsonObject jsonObject;

    // 用户过期时间
    if (m_userExpiresSwitch->isChecked())
    {
        QDate date = ui->dateEdit_userExpires->date();
        QDate tempDate(1970, 01, 01);
        qint64 days = tempDate.daysTo(date);
        jsonObject[ACCOUNTS_PEP_EXPIRATION_TIME] = days;
    }
    else
    {
        jsonObject[ACCOUNTS_PEP_EXPIRATION_TIME] = -1;
    }

    // 密码有效天数限制
    if (m_passwdMaxVaildDaysSwtich->isChecked())
    {
        int maxValidDays = ui->spinBox_passwdMaxVaildDays->value();
        jsonObject[ACCOUNTS_PEP_MAX_DAYS] = maxValidDays;
    }
    else
    {
        jsonObject[ACCOUNTS_PEP_MAX_DAYS] = 99999;
    }

    // 提示账户过期时间
    if (m_promptBeforePasswdExpirationSwitch->isChecked())
    {
        int promptDay = ui->spinBox_promptTime->value();
        jsonObject[ACCOUNTS_PEP_DAYS_TO_WARN] = promptDay;
    }
    else
    {
        jsonObject[ACCOUNTS_PEP_DAYS_TO_WARN] = 0;
    }

    // 密码失效开关
    if (m_passwdInactiveSwitch->isChecked())
    {
        int inactiveDays = ui->spinBox_inactiveDays->value();
        jsonObject[ACCOUNTS_PEP_INACTIVE_DAYS] = inactiveDays;
    }
    else
    {
        jsonObject[ACCOUNTS_PEP_INACTIVE_DAYS] = -1;
    }

    QJsonDocument jsonDoc(jsonObject);

    QDBusPendingReply<> reply = m_userProxy->SetPasswordExpirationPolicy(jsonDoc.toJson(QJsonDocument::Compact));
    reply.waitForFinished();
    if (reply.isError())
    {
        KLOG_ERROR(qLcAccount) << "set password expiration policy failed," << reply.error();
    }
    else
    {
        KLOG_DEBUG(qLcAccount) << "update password expiration policy success:" << jsonDoc;
    }
}