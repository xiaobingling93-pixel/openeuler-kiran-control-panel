/**
 * Copyright (c) 2020 ~ 2025 KylinSec Co., Ltd.
 * kiran-control-panel is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 *
 * Author:     yuanxing <yuanxing@kylinsec.com.cn>
 */

#include "add-users-page.h"
#include "accounts-global-info.h"
#include "group-manager.h"
#include "ksd_accounts_user_proxy.h"
#include "ui_add-users-page.h"
#include "user-list-item.h"
#include "users-container.h"

#include <kiran-message-box.h>
#include <kiran-push-button.h>
#include <qt5-log-i.h>

AddUsersPage::AddUsersPage(QWidget *parent)
    : QWidget(parent), ui(new Ui::AddUsersPage)
{
    ui->setupUi(this);
    initUI();
}

AddUsersPage::~AddUsersPage()
{
    delete ui;
}

void AddUsersPage::initUI()
{
    // 所有用户名
    m_allUserName = getAllUserName();

    // 用户列表容器
    m_usersContainer = new UsersContainer(this);
    ui->layout_user_list->addWidget(m_usersContainer);

    KiranPushButton::setButtonType(ui->btn_save, KiranPushButton::BUTTON_Default);

    ui->search_box->setPlaceholderText(tr("Please input keys for search..."));
    /// 连接搜索框和user_list，使搜索框输入文字时在user_list上显示匹配到的用户
    connect(ui->search_box, &QLineEdit::textEdited, this, &AddUsersPage::searchFilter);

    connect(ui->btn_cancel, &QPushButton::clicked, [this]()
            { emit requestGroupInfoPage(); });

    connect(ui->btn_save, &QPushButton::clicked, [this]()
            {
                QStringList userNameList;
                for (auto item : m_usersContainer->getSelectedItems())
                {
                    userNameList.append(item->name());
                }
                if (!userNameList.isEmpty())
                {
                    ui->btn_save->setBusy(true);
                    emit requestAddUserToGroup(m_curShowGroupPath, userNameList);} });
}

QStringList AddUsersPage::getAllUserName()
{
    QStringList result;
    QString userName;
    auto userObjList = AccountsGlobalInfo::instance()->getUserList();
    for (auto iter : userObjList)
    {
        if (AccountsGlobalInfo::instance()->getUserName(iter, userName))
        {
            result.append(userName);
        }
    }
    return result;
}

void AddUsersPage::updateUsersList(const QString &groupObj)
{
    m_curShowGroupPath = groupObj;

    m_usersContainer->clear();
    m_usersInGroup.clear();

    GroupManager::GroupInfo groupInfo;
    if (GroupManager::instance()->getGroupInfo(m_curShowGroupPath, groupInfo))
    {
        /// 加载不在用户组中的用户
        m_usersInGroup = groupInfo.users;
        for (auto name : m_allUserName)
        {
            if (!m_usersInGroup.contains(name))
            {
                appendUserListItem(name);
            }
        }
    }

    ui->search_box->setFocus();
    ui->search_box->clear();
}

void AddUsersPage::appendUserListItem(const QString &userName)
{
    auto item = new UserListItem(m_usersContainer);
    item->setName(userName);
    item->setClickable(true);
    item->setRightBtnIcon(QIcon(":/kcp-group-images/chosen_icon.svg"));
    item->setRightBtnVisible(false);
    m_usersContainer->addItem(item);
}

void AddUsersPage::searchFilter(const QString &filterString)
{
    for (auto name : m_allUserName)
    {
        if (!m_usersInGroup.contains(name))
        {
            bool isVisible = filterString.isEmpty() || name.contains(filterString);
            m_usersContainer->setItemVisible(name, isVisible);
        }
    }
}

void AddUsersPage::updateUI(const QString &errMsg)
{
    ui->btn_save->setBusy(false);
    if (!errMsg.isEmpty())
    {
        KiranMessageBox::message(nullptr, tr("Error"),
                                 errMsg, KiranMessageBox::Ok);
    }
}