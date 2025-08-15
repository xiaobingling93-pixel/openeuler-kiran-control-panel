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

#pragma once

#include <QThread>
#include <QWidget>

class QStackedWidget;
class KiranSidebarWidget;
class GroupInterface;
class QListWidgetItem;
class CreateGroupPage;
class GroupInfoPage;
class AddUsersPage;

class GroupPage : public QWidget
{
    Q_OBJECT

public:
    enum StackWidgetPageEnum
    {
        PAGE_CREATE_GROUP,
        PAGE_GROUP_INFO,
        PAGE_ADD_USERS
    };

public:
    explicit GroupPage(QWidget *parent = nullptr);
    ~GroupPage();
    QSize sizeHint() const;
    void jumpToPage(StackWidgetPageEnum page);

private Q_SLOTS:
    void addGroup(const QString &groupPath);
    void deleteGroup(const QString &groupPath);
    void handleGroupChanged(const QString &groupPath);

    void appendSidebarItem(const QString &groupPath);
    void deleteSidebarItem(const QString &groupPath);
    void updateSidebarItem(const QString &groupPath);

private:
    void initUI();
    void initGroupList();
    void initPageCreateGroup();
    void initPageGroupInfo();
    void initPageAddUsers();
    void connectToInfoChange();

    void setDefaultSiderbarItem();

private:
    KiranSidebarWidget *m_tabList = nullptr;
    QThread m_workThread;
    GroupInterface *m_groupInterface = nullptr;
    QListWidgetItem *m_createGroupItem = nullptr;
    CreateGroupPage *m_pageCreateGroup = nullptr;
    GroupInfoPage *m_pageGroupInfo = nullptr;
    AddUsersPage *m_pageAddUsers = nullptr;
    QStackedWidget *m_stackWidget = nullptr;
};
