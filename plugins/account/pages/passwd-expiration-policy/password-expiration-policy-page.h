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

#ifndef KIRAN_CPANEL_ACCOUNT_SRC_PAGES_PASSWD_EXPIRATION_POLICY_PASSWORD_EXPIRATION_POLICY_PAGE_H_
#define KIRAN_CPANEL_ACCOUNT_SRC_PAGES_PASSWD_EXPIRATION_POLICY_PASSWORD_EXPIRATION_POLICY_PAGE_H_

#include <QWidget>
#include "account.h"

QT_BEGIN_NAMESPACE
namespace Ui
{
class PasswordExpirationPolicyPage;
}
QT_END_NAMESPACE

class KiranSwitchButton;
class KSDAccountsUserProxy;
class PasswordExpirationPolicyPage : public QWidget
{
    Q_OBJECT

public:
    explicit PasswordExpirationPolicyPage(QWidget *parent = nullptr);
    ~PasswordExpirationPolicyPage() override;

    void setCurrentUser(const QString &userObj);

private:
    void initUI();
    void updateInfo();
    void save();

signals:
    void sigReturn();

private slots:
    void slotUserPropertyChanged(QString propertyName, QVariant value);

    void handleUserExpireSwitchToggled(bool checked);
    void handlePasswdMaxVaildDaysSwitchToggled(bool checked);
    void handlePromptTimeSwitchToggled(bool checked);
    void handleInactiveDaysSwitchToggled(bool checked);

private:
    Ui::PasswordExpirationPolicyPage *ui;
    DBusWrapper::Account::AccountUserInterfacePtr m_userProxy;
    KiranSwitchButton* m_userExpiresSwitch = nullptr;
    KiranSwitchButton* m_passwdMaxVaildDaysSwtich = nullptr;
    KiranSwitchButton* m_promptBeforePasswdExpirationSwitch = nullptr;
    KiranSwitchButton* m_passwdInactiveSwitch = nullptr;
};

#endif  //KIRAN_CPANEL_ACCOUNT_SRC_PAGES_PASSWD_EXPIRATION_POLICY_PASSWORD_EXPIRATION_POLICY_PAGE_H_
