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

#include "user-info-page.h"
#include "accounts-global-info.h"
#include "hover-tips/hover-tips.h"
#include "kiran-account-service-wrapper.h"
#include "passwd-helper.h"
#include "ui_user-info-page.h"

#include <kiran-push-button.h>
#include <kiran-switch-button.h>
#include <kiran-system-daemon/accounts-i.h>
#include <kiranwidgets-qt5/kiran-message-box.h>
#include <qt5-log-i.h>

#include <QEvent>
#include <QKeyEvent>
#include <QListView>
#include <QMessageBox>

enum PageEnum
{
    PAGE_USER_INFO,
    PAGE_CHANGE_PASSWD
};

UserInfoPage::UserInfoPage(QWidget *parent) : QWidget(parent),
                                              ui(new Ui::UserInfoPage)
{
    ui->setupUi(this);
    initUI();
}

UserInfoPage::~UserInfoPage()
{
    delete ui;
}

void UserInfoPage::updateInfo()
{
    m_errorTip->hideTip();

    auto userAPI = DBusWrapper::createKiranAccountServiceUserAPI(m_curShowUserPath);
    QString userName = userAPI->user_name();
    m_uid = userAPI->uid();
    int userType = userAPI->account_type();
    QString iconFile = userAPI->icon_file();
    bool locked = userAPI->locked();

    ui->label_name->setText(userName);
    ui->edit_userID->setText(QString::number(m_uid));
    ui->combo_userType->setCurrentIndex(userType);
    ui->avatar->setImage(iconFile);
    m_userStatusSwitch->setChecked(!locked);
    m_curShowUserName = userName;

    if (m_curShowUserName != AccountsGlobalInfo::instance()->getCurrentUser())
    {
        /// 显示用户非当前登录会话用户
        ///  不验证当前密码，由后端验证ROOT密码
        ui->passwd_row_1->setVisible(false);
        ui->passwd_row_2->setVisible(false);
        /// 允许删除用户
        ui->btn_deleteUser->setEnabled(true);
    }
    else
    {
        /// 显示用户为当前登录会话用户
        ///  验证当前密码
        ui->passwd_row_1->setVisible(true);
        ui->passwd_row_2->setVisible(true);
        /// 禁用删除用户
        ui->btn_deleteUser->setEnabled(false);
    }

    ui->stackedWidget->setCurrentIndex(PAGE_USER_INFO);
}

void UserInfoPage::setCurrentShowUserPath(const QString &userObj)
{
    m_curShowUserPath = userObj;
    updateInfo();
}

QString UserInfoPage::getCurrentShowUserPath()
{
    return m_curShowUserPath;
}

QString UserInfoPage::getCurrentShowUserName()
{
    return m_curShowUserName;
}

void UserInfoPage::setAvatarIconPath(const QString &iconPath)
{
    ui->avatar->setImage(iconPath);
}

void UserInfoPage::initUI()
{
    ui->btn_changePasswd->setFontUnderLine(true);
    ui->btn_passwdExpirationPolicy->setFontUnderLine(true);

    m_errorTip = new KiranTips(this);
    m_errorTip->setShowPosition(KiranTips::POSITION_BOTTM);
    m_errorTip->setAnimationEnable(true);

    m_hoverTip = new HoverTips(this);

    /* 用户状态的开关按钮 */
    m_userStatusSwitch = new KiranSwitchButton(this);
    ui->layout_userStatusSwitch->insertWidget(0, m_userStatusSwitch);

    KiranPushButton::setButtonType(ui->btn_saveProperty, KiranPushButton::BUTTON_Default);
    KiranPushButton::setButtonType(ui->btn_deleteUser, KiranPushButton::BUTTON_Warning);
    KiranPushButton::setButtonType(ui->btn_savePasswd, KiranPushButton::BUTTON_Default);

    /* 用户显示页面 */
    // 用户头像
    ui->avatar->setHoverImage(":/kcp-account/images/change-user-avatar.png");
    ui->avatar->setClickEnable(true);
    connect(ui->avatar, &UserAvatarWidget::pressed, [this]()
            { emit requestIconPage(ui->avatar->iconPath()); });

    // 用户类型显示
    ui->combo_userType->addItem(tr("standard"));
    ui->combo_userType->addItem(tr("administrator"));

    // 修改密码按钮
    connect(ui->btn_changePasswd, &QPushButton::clicked, [this]()
            {
        resetPageSetPasswd();
        ui->stackedWidget->setCurrentIndex(PAGE_CHANGE_PASSWD); });

    // 确认按钮
    connect(ui->btn_saveProperty, &QPushButton::clicked,
            this, &UserInfoPage::handlerUpdateUserProperty);

    // 删除用户
    connect(ui->btn_deleteUser, &QPushButton::clicked,
            this, &UserInfoPage::handlerDeleteUser);

    /* 修改密码页面 */
    ui->editcheck_curpasswd->setEchoMode(QLineEdit::Password);
    ui->editcheck_curpasswd->setAttribute(Qt::WA_InputMethodEnabled, false);

    ui->editcheck_newPasswd->setMaxLength(20);
    ui->editcheck_newPasswd->setEchoMode(QLineEdit::Password);
    ui->editcheck_newPasswd->setAttribute(Qt::WA_InputMethodEnabled, false);
    ui->editcheck_newPasswd->installEventFilter(this);

    ui->editcheck_confirmPasswd->setMaxLength(20);
    ui->editcheck_confirmPasswd->setEchoMode(QLineEdit::Password);
    ui->editcheck_confirmPasswd->setAttribute(Qt::WA_InputMethodEnabled, false);
    ui->editcheck_confirmPasswd->installEventFilter(this);

    // 保存按钮
    connect(ui->btn_savePasswd, &QPushButton::clicked,
            this, &UserInfoPage::handlerUpdatePasswd);

    // 取消按钮
    connect(ui->btn_cancel, &QPushButton::clicked, [this]()
            {
        m_errorTip->hideTip();
        ui->stackedWidget->setCurrentIndex(PAGE_USER_INFO); });

    connect(ui->btn_passwdExpirationPolicy, &QPushButton::clicked, [this]()
            { emit requestPasswordExpirationPolicy(m_curShowUserPath); });

    if( !AccountsGlobalInfo::instance()->getShowPasswordExpirationPolicy() )
    {
        ui->btn_passwdExpirationPolicy->setVisible(false);
    }
}

void UserInfoPage::resetPageSetPasswd()
{
    ui->editcheck_curpasswd->clear();
    ui->editcheck_newPasswd->clear();
    ui->editcheck_confirmPasswd->clear();
}

void UserInfoPage::handlerUpdatePasswd()
{
    // 新密码不能为空
    QString newpasswd = ui->editcheck_newPasswd->text();
    if (newpasswd.isEmpty())
    {
        m_errorTip->setText(tr("Please enter the new user password"));
        m_errorTip->showTipAroundWidget(ui->editcheck_newPasswd);
        return;
    }
    // 确认新密码不为空，并且和确认密码相同
    QString confirmNewPasswd = ui->editcheck_confirmPasswd->text();
    if (confirmNewPasswd.isEmpty())
    {
        m_errorTip->setText(tr("Please enter the password again"));
        m_errorTip->showTipAroundWidget(ui->editcheck_confirmPasswd);
        return;
    }
    if (newpasswd != confirmNewPasswd)
    {
        m_errorTip->setText(tr("The password you enter must be the same as the former one"));
        m_errorTip->showTipAroundWidget(ui->editcheck_confirmPasswd);
        return;
    }
    QString encryptedCurPasswd;
    // 当前密码校验
    if (ui->passwd_row_1->isVisible() && ui->passwd_row_2->isVisible())
    {
        // 当前密码不能为空
        QString curpasswd = ui->editcheck_curpasswd->text();
        if (curpasswd.isEmpty())
        {
            m_errorTip->setText(tr("Please enter the current user password"));
            m_errorTip->showTipAroundWidget(ui->editcheck_curpasswd);
            return;
        }
        if (!PasswdHelper::checkUserPassword(ui->label_name->text(), curpasswd))
        {
            m_errorTip->setText(tr("The current password is incorrect"));
            m_errorTip->showTipAroundWidget(ui->editcheck_curpasswd);
            return;
        }
        // 当前密码是否和新密码相同
        if (curpasswd == newpasswd)
        {
            m_errorTip->setText(tr("The new password cannot be the same as the current password"));
            m_errorTip->showTipAroundWidget(ui->editcheck_newPasswd);
            return;
        }
        if (!PasswdHelper::encryptPasswordByRsa(AccountsGlobalInfo::rsaPublicKey(), curpasswd, encryptedCurPasswd))
        {
            QMessageBox::warning(this, tr("Error"), tr("Password encryption failed"));
            return;
        }
    }
    // 密码加密
    QString encryptedPasswd;
    if (!PasswdHelper::encryptPasswordByRsa(AccountsGlobalInfo::rsaPublicKey(), newpasswd, encryptedPasswd))
    {
        QMessageBox::warning(this, tr("Error"), tr("Password encryption failed"));
        return;
    }
    ui->btn_savePasswd->setBusy(true);
    emit busyChanged(true);
    emit requestUpdatePasswd(getCurrentShowUserPath(),
                             getCurrentShowUserName(),
                             encryptedCurPasswd,
                             encryptedPasswd);
}

void UserInfoPage::handlerUpdateUserProperty()
{
    QString account, icon;
    int userType;
    bool isLocked;

    account = getCurrentShowUserName();
    icon = ui->avatar->iconPath();
    userType = ui->combo_userType->currentIndex();
    isLocked = !m_userStatusSwitch->isChecked();

    ui->btn_saveProperty->setBusy(true);
    emit busyChanged(true);
    emit requestUpdateUserProperty(getCurrentShowUserPath(),
                                   account,
                                   icon,
                                   userType,
                                   isLocked);
}

void UserInfoPage::handlerUpdateUserPropertyDone(QString errMsg)
{
    ui->btn_saveProperty->setBusy(false);
    emit busyChanged(false);
    if (!errMsg.isEmpty())
    {
        KiranMessageBox::message(nullptr,
                                 tr("Error"), errMsg,
                                 KiranMessageBox::Ok);
    }
    else
    {
        m_hoverTip->show(HoverTips::HOVE_TIPS_SUC, tr("user information updated successfully"));
    }
    /// NOTE: 如果属性设置成功了AccountsGlobalInfo会更新当前页面
    ///       手动更新是为了避免设置失败,界面未复位
    updateInfo();
}

void UserInfoPage::handlerUpdatePasswdDone(QString errMsg)
{
    ui->btn_savePasswd->setBusy(false);
    emit busyChanged(false);
    if (!errMsg.isEmpty())
    {
        KiranMessageBox::message(nullptr,
                                 tr("Error"), errMsg,
                                 KiranMessageBox::Ok);
    }
    else
    {
        ui->stackedWidget->setCurrentIndex(PAGE_USER_INFO);
        m_hoverTip->show(HoverTips::HOVE_TIPS_SUC, tr("Password updated successfully"));
    }
}

void UserInfoPage::handlerDeleteUser()
{
    QString tip = QString(tr("The directory and files under the user's home directory are deleted with the user."
                             "Are you sure you want to delete the user(%1)?"))
                      .arg(m_curShowUserName);
    KiranMessageBox::KiranStandardButton btn = KiranMessageBox::message(this, tr("Warning"),
                                                                        tip,
                                                                        KiranMessageBox::Yes | KiranMessageBox::No);
    if (btn == KiranMessageBox::No)
    {
        return;
    }

    ui->btn_deleteUser->setBusy(true);
    emit busyChanged(true);
    emit requestDeleteUser(m_uid);
}

void UserInfoPage::handlerDeleteUserDone(QString errMsg)
{
    ui->btn_deleteUser->setBusy(false);
    emit busyChanged(false);
    if (!errMsg.isEmpty())
    {
        KiranMessageBox::message(this, tr("Error"), errMsg, KiranMessageBox::Ok);
    }
}

bool UserInfoPage::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == ui->editcheck_newPasswd || watched == ui->editcheck_confirmPasswd)
    {
        if (event->type() == QEvent::KeyPress)
        {
            auto keyEvent = dynamic_cast<QKeyEvent *>(event);
            if (keyEvent->matches(QKeySequence::Paste))
            {
                KLOG_DEBUG(qLcAccount) << "event filter QKeySequence::Paster for passwd lineedit!";
                return true;
            }
        }
        else if (event->type() == QEvent::MouseButtonPress)
        {
            auto mouseEvent = dynamic_cast<QMouseEvent *>(event);
            if (mouseEvent->buttons() & Qt::MidButton)
            {
                KLOG_DEBUG(qLcAccount) << "event filter Qt::MidButton for passwd lineedit!";
                return true;
            }
        }
    }
    return QWidget::eventFilter(watched, event);
}
