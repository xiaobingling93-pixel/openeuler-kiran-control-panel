/**
 * Copyright (c) 2020 ~ 2021 KylinSec Co., Ltd.
 * kiran-cpanel-group is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 *
 * Author:     wangshichang <shichang@isrc.iscas.ac.cn>
 */

#include "create-group-page.h"
#include "accounts-global-info.h"
#include "group-manager.h"
#include "group-name-checker.h"
#include "group-name-validator.h"
#include "kiran-tips/kiran-tips.h"
#include "ksd_accounts_user_proxy.h"
#include "ui_create-group-page.h"
#include "user-list-item.h"
#include "users-container.h"

#include <kiran-message-box.h>
#include <kiran-push-button.h>
#include <qt5-log-i.h>

CreateGroupPage::CreateGroupPage(QWidget *parent)
    : QWidget(parent), ui(new Ui::CreateGroupPage)
{
    ui->setupUi(this);

    initUI();
}

CreateGroupPage::~CreateGroupPage()
{
    delete ui;
}

void CreateGroupPage::initUI()
{
    /// 提示框
    m_errorTip = new KiranTips(this);
    m_errorTip->setShowPosition(KiranTips::POSITION_BOTTM);
    m_errorTip->setAnimationEnable(true);

    /// 用户组名输入框
    ui->edit_name->setValidator(new GroupNameValidator(ui->edit_name));

    // 用户列表
    m_userContainter = new UsersContainer(this);
    ui->userListLayout->addWidget(m_userContainter);

    /// 初始用户列表
    QList<QString> userObjList;
    userObjList = AccountsGlobalInfo::instance()->getUserList();
    for (auto &iter : userObjList)
    {
        appendUserListItem(iter);
    }

    /// 确认按钮
    KiranPushButton::setButtonType(ui->btn_confirm, KiranPushButton::BUTTON_Default);
    connect(ui->btn_confirm, &QPushButton::clicked, this, &CreateGroupPage::createGroup);
}

void CreateGroupPage::appendUserListItem(const QString &userPath)
{
    QString userName;
    if (AccountsGlobalInfo::instance()->getUserName(userPath, userName))
    {
        auto item = new UserListItem(m_userContainter);
        item->setRightBtnIcon(QIcon(":/kcp-group/images/indicator-selected.svg"));
        item->setRightBtnVisible(false);
        item->setName(userName);
        item->setClickable(true);
        m_userContainter->addItem(item);
    }
}

void CreateGroupPage::createGroup()
{
    QString errorMessage;
    auto groupName = ui->edit_name->text();
    if (!GroupNameChecker::isValid(groupName, errorMessage))
    {
        m_errorTip->setText(errorMessage);
        m_errorTip->showTipAroundWidget(ui->edit_name);
        return;
    }

    // 添加用户到用户组
    QStringList userNameList;
    auto items = m_userContainter->getSelectedItems();
    for (auto item : items)
    {
        userNameList.append(item->name());
    }

    ui->btn_confirm->setBusy(true);
    emit requestCreateGroup(groupName, userNameList);
}

void CreateGroupPage::handleGroupAdded(const QString &groupPath, const QString &errMsg)
{
    Q_UNUSED(groupPath);
    ui->btn_confirm->setBusy(false);
    if (!errMsg.isEmpty())
    {
        KiranMessageBox::message(nullptr, tr("Error"),
                                 errMsg, KiranMessageBox::Ok);
    }
}

void CreateGroupPage::reset()
{
    ui->edit_name->clear();
    m_userContainter->clear();
    QList<QString> userObjList;
    userObjList = AccountsGlobalInfo::instance()->getUserList();
    for (auto &iter : userObjList)
    {
        appendUserListItem(iter);
    }
    m_errorTip->hideTip();
}
