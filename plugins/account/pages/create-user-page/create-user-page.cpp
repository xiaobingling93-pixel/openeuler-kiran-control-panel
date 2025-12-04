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

#include "create-user-page.h"
#include "accounts-global-info.h"
#include "advance-settings-page/advance-settings.h"
#include "kiran-tips/kiran-tips.h"
#include "logging-category.h"
#include "passwd-helper.h"
#include "ui_create-user-page.h"
#include "user-name-validator.h"

#include <kiran-push-button.h>
#include <kiranwidgets-qt5/kiran-message-box.h>
#include <qt5-log-i.h>
#include <QEvent>
#include <QKeyEvent>
#include <QMessageBox>

CreateUserPage::CreateUserPage(QWidget *parent)
    : QWidget(parent),
      ui(new Ui::CreateUserPage)
{
    ui->setupUi(this);
    initUI();
}

CreateUserPage::~CreateUserPage()
{
    delete ui;
}

void CreateUserPage::reset()
{
    ui->avatar->setDefaultImage();
    ui->edit_name->clear();
    ui->combo_userType->setCurrentIndex(0);
    ui->editcheck_passwd->resetVerificationStatus();
    ui->editcheck_passwd->clear();
    ui->editcheck_confirmPasswd->resetVerificationStatus();
    ui->editcheck_confirmPasswd->clear();
    m_errorTip->hideTip();

    m_advanceSettingsInfo.clear();
}

void CreateUserPage::setAvatarIconPath(const QString &iconPath)
{
    ui->avatar->setImage(iconPath);
}

// 初始化界面
void CreateUserPage::initUI()
{
    /// 提示框
    m_errorTip = new KiranTips(this);
    m_errorTip->setShowPosition(KiranTips::POSITION_BOTTM);
    m_errorTip->setAnimationEnable(true);

    /// 用户头像
    ui->avatar->setHoverImage(":/kcp-account/images/change-user-avatar.png");
    ui->avatar->setClickEnable(true);
    connect(ui->avatar, &UserAvatarWidget::pressed, [this]()
            { emit requestIconPageForNewUser(ui->avatar->iconPath()); });

    /// 用户类型ComboBox
    ui->combo_userType->addItem(tr("standard"));
    ui->combo_userType->addItem(tr("administrator"));

    /// 用户名输入框
    ui->edit_name->setValidator(new UserNameValidator(ui->edit_name));
    // NOTE:用户名不能超过32字符长
    ui->edit_name->setMaxLength(32);

    /// 密码输入框
    ui->editcheck_passwd->setMaxLength(20);
    ui->editcheck_passwd->setEchoMode(QLineEdit::Password);
    ui->editcheck_passwd->setAttribute(Qt::WA_InputMethodEnabled, false);
    ui->editcheck_passwd->installEventFilter(this);

    ui->editcheck_confirmPasswd->setMaxLength(20);
    ui->editcheck_confirmPasswd->setEchoMode(QLineEdit::Password);
    ui->editcheck_confirmPasswd->setAttribute(Qt::WA_InputMethodEnabled, false);
    ui->editcheck_confirmPasswd->installEventFilter(this);

    /// 高级设置按钮
    connect(ui->btn_advanceSetting, &QPushButton::clicked, [this]()
            {
                if (ui->edit_name->text().isEmpty())
                {
                    m_errorTip->setText(tr("Please enter user name first"));
                    m_errorTip->showTipAroundWidget(ui->edit_name);
                    return;
                }

                AdvanceSettings::exec(ui->edit_name->text(), m_advanceSettingsInfo);
            });

    /// 确认按钮
    KiranPushButton::setButtonType(ui->btn_confirm, KiranPushButton::BUTTON_Default);
    connect(ui->btn_confirm, &QPushButton::clicked, this, &CreateUserPage::onCreateUserClicked);

    /// 取消按钮
    connect(ui->btn_cancel, &QPushButton::clicked, [this]()
            { reset(); });
}

void CreateUserPage::onCreateUserClicked()
{
    // step1.检验用户名是否为空，是否重名
    KLOG_DEBUG(qLcAccount) << "create user clicked,check user name";
    QString userName = ui->edit_name->text();

    if (userName.isEmpty())
    {
        m_errorTip->setText(tr("Please enter your user name"));
        m_errorTip->showTipAroundWidget(ui->edit_name);
        return;
    }

    bool isPureDigital = true;
    for (QChar ch : userName)
    {
        if (!ch.isNumber())
        {
            isPureDigital = false;
            break;
        }
    }
    if (isPureDigital)
    {
        m_errorTip->setText(tr("user name cannot be a pure number"));
        m_errorTip->showTipAroundWidget(ui->edit_name);
        return;
    }

    if (!AccountsGlobalInfo::instance()->checkUserNameAvaliable(userName))
    {
        m_errorTip->setText(tr("user name already exists"));
        m_errorTip->showTipAroundWidget(ui->edit_name);
        return;
    }

    // step2.检验密码、确认密码是否为空，是否相等
    KLOG_DEBUG(qLcAccount) << "create user clicked,check user passwd";
    QString passwd = ui->editcheck_passwd->text();
    QString confirmPasswd = ui->editcheck_confirmPasswd->text();
    if (passwd.isEmpty())
    {
        ui->editcheck_passwd->setVerificationStatus(false);
        m_errorTip->setText(tr("Please enter your password"));
        m_errorTip->showTipAroundWidget(ui->editcheck_passwd);
        return;
    }
    if (confirmPasswd.isEmpty())
    {
        ui->editcheck_confirmPasswd->setVerificationStatus(false);
        m_errorTip->setText(tr("Please enter the password again"));
        m_errorTip->showTipAroundWidget(ui->editcheck_confirmPasswd);
        return;
    }
    if (passwd != confirmPasswd)
    {
        ui->editcheck_confirmPasswd->setVerificationStatus(false);
        m_errorTip->setText(tr("The password you enter must be the same as the former one"));
        m_errorTip->showTipAroundWidget(ui->editcheck_confirmPasswd);
        return;
    }

    // step3.调用crypt密码加密
    KLOG_DEBUG(qLcAccount) << "create user clicked,start encrypt passwd";
    QString encryptedPasswd;
    if (!PasswdHelper::encryptPasswordByRsa(AccountsGlobalInfo::rsaPublicKey(), passwd, encryptedPasswd))
    {
        QMessageBox::warning(this, tr("Error"), tr("Password encryption failed"));
        return;
    }

    qint64 uid = -1;
    if (!m_advanceSettingsInfo.uid.isEmpty())
    {
        bool toNumberOk = false;
        uid = m_advanceSettingsInfo.uid.toLongLong(&toNumberOk);
        if (!toNumberOk)
        {
            uid = -1;
        }
    }
    int accountType = ui->combo_userType->currentIndex();
    QString homeDir = m_advanceSettingsInfo.homeDir;
    QString shell = m_advanceSettingsInfo.shell;
    QString iconFile = ui->avatar->iconPath();

    emit busyChanged(true);
    ui->btn_confirm->setBusy(true);
    emit requestCreateUser(userName, uid, accountType,
                           encryptedPasswd,
                           homeDir,
                           shell,
                           iconFile);
}

void CreateUserPage::onCreateUserDone(QString userPath,
                                      QString errMsg)
{
    emit busyChanged(false);
    ui->btn_confirm->setBusy(false);
    if (!errMsg.isEmpty())
    {
        KiranMessageBox::message(nullptr, tr("Error"),
                                 errMsg, KiranMessageBox::Ok);
    }
    if (!userPath.isEmpty())
    {
        emit requestSetCurrentUser(userPath);
    }
}

bool CreateUserPage::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == ui->editcheck_passwd || watched == ui->editcheck_confirmPasswd)
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
