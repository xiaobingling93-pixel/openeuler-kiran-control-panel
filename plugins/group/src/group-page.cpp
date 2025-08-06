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
#include "group-page.h"
#include "accounts-global-info.h"
#include "add-users-page/add-users-page.h"
#include "create-group-page/create-group-page.h"
#include "group-info-page/group-info-page.h"
#include "group-interface.h"
#include "group-manager.h"
#include "kiran-color-block.h"

#include <kiran-sidebar-item.h>
#include <kiran-sidebar-widget.h>
#include <qt5-log-i.h>
#include <QHBoxLayout>
#include <QIcon>
#include <QListWidgetItem>
#include <QStackedWidget>
#include <QTimer>

#define ITEM_GROUP_OBJ_PATH_ROLE Qt::UserRole + 1

GroupPage::GroupPage(QWidget *parent)
    : QWidget(parent)
{
    m_workThread.start();
    m_groupInterface = GroupManager::instance()->getInterface();
    m_groupInterface->moveToThread(&m_workThread);

    KLOG_INFO() << "WorkThread:" << m_workThread.currentThreadId();
    KLOG_INFO() << "current Thread:" << QThread::currentThreadId();
    initUI();
}

GroupPage::~GroupPage()
{
    if (m_workThread.isRunning())
    {
        m_workThread.quit();
        m_workThread.wait();
    }
}

void GroupPage::initUI()
{
    /*初始化界面主布局*/
    auto contentLayout = new QHBoxLayout(this);
    contentLayout->setSpacing(0);
    contentLayout->setObjectName("GroupContentLayout");
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(4);

    /* 侧边栏 */
    auto siderbar = new KiranColorBlock(this);
    contentLayout->addWidget(siderbar);
    siderbar->setObjectName("siderWidget");
    siderbar->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    siderbar->setFixedWidth(272);

    auto vLayout = new QVBoxLayout(siderbar);
    vLayout->setSpacing(0);
    vLayout->setContentsMargins(0, 0, 0, 0);
    vLayout->setObjectName("SiderbarVLayout");

    m_tabList = new KiranSidebarWidget(siderbar);
    m_tabList->viewport()->setAutoFillBackground(false);
    m_tabList->setFrameShape(QFrame::NoFrame);
    m_tabList->setObjectName("tabList");
    m_tabList->setIconSize(QSize(40, 40));
    vLayout->addWidget(m_tabList);
    initGroupList();

    /*内容区域*/
    KiranColorBlock *stackedColorBlock = new KiranColorBlock(this);
    QHBoxLayout *stackedColorBlockLayout = new QHBoxLayout(stackedColorBlock);

    m_stackWidget = new QStackedWidget(this);
    m_stackWidget->setObjectName("StackWidget");
    stackedColorBlockLayout->addWidget(m_stackWidget);
    contentLayout->addWidget(stackedColorBlock);

    m_pageCreateGroup = new CreateGroupPage(m_stackWidget);
    m_stackWidget->insertWidget(PAGE_CREATE_GROUP, m_pageCreateGroup);
    initPageCreateGroup();

    m_pageGroupInfo = new GroupInfoPage(m_stackWidget);
    m_stackWidget->insertWidget(PAGE_GROUP_INFO, m_pageGroupInfo);
    initPageGroupInfo();

    m_pageAddUsers = new AddUsersPage(m_stackWidget);
    m_stackWidget->insertWidget(PAGE_ADD_USERS, m_pageAddUsers);
    initPageAddUsers();

    m_stackWidget->setCurrentIndex(PAGE_CREATE_GROUP);

    connectToInfoChange();

    QTimer::singleShot(0, this, &GroupPage::setDefaultSiderbarItem);
}

void GroupPage::initGroupList()
{
    connect(m_tabList, &KiranSidebarWidget::itemSelectionChanged, [this]()
            {
                QList<QListWidgetItem *> selecteds = m_tabList->selectedItems();
                if (selecteds.size() != 1)
                {
                    return;
                }
                QListWidgetItem *item = selecteds.at(0);
                if (item == m_createGroupItem)
                {
                    //重置创建用户组页面
                    m_pageCreateGroup->reset();
                    //切换到创建用户组
                    m_stackWidget->setCurrentIndex(PAGE_CREATE_GROUP);
                }
                else
                {
                    QString groupObjPath = item->data(ITEM_GROUP_OBJ_PATH_ROLE).toString();
                    // 更新用户组信息页面
                    m_pageGroupInfo->setCurrentShowGroupPath(groupObjPath);
                    //切换到用户组信息
                    m_stackWidget->setCurrentIndex(PAGE_GROUP_INFO);
                } });

    /// 创建用户组按钮
    m_createGroupItem = new QListWidgetItem(tr("Create new group"), m_tabList);
    m_createGroupItem->setIcon(QIcon::fromTheme("krsvg-create-group-icon"));
    m_tabList->addItem(m_createGroupItem);

    QList<QString> groupObjList;
    groupObjList = GroupManager::instance()->getGroupList();

    GroupManager::GroupInfo groupInfo;
    for (auto &iter : groupObjList)
    {
        if (GroupManager::instance()->getGroupInfo(iter, groupInfo))
        {
            if (groupInfo.isNotSystemGroup)
            {
                auto groupName = groupInfo.name;
                auto item = new KiranSidebarItem(groupName, m_tabList);
                item->setIcon(QIcon::fromTheme("krsvg-group-icon"));
                item->setData(ITEM_GROUP_OBJ_PATH_ROLE, iter);
                m_tabList->addItem(item);
            }
        }
    }
}

void GroupPage::appendSidebarItem(const QString &groupPath)
{
    KLOG_INFO() << "current Thread:" << QThread::currentThreadId();

    GroupManager::GroupInfo groupInfo;
    if (GroupManager::instance()->getGroupInfo(groupPath, groupInfo))
    {
        if (groupInfo.isNotSystemGroup)
        {
            auto item = new KiranSidebarItem(groupInfo.name, m_tabList);
            item->setIcon(QIcon::fromTheme("krsvg-group-icon"));
            item->setData(ITEM_GROUP_OBJ_PATH_ROLE, groupPath);
            m_tabList->addItem(item);
            m_tabList->setCurrentItem(item);
        }
    }
}

void GroupPage::deleteSidebarItem(const QString &groupPath)
{
    KLOG_INFO() << "current Thread:" << QThread::currentThreadId();
    auto itemCount = m_tabList->count();
    GroupManager::GroupInfo groupInfo;

    for (int i = 0; i < itemCount; i++)
    {
        auto item = m_tabList->item(i);
        if (item->data(ITEM_GROUP_OBJ_PATH_ROLE) == groupPath)
        {
            delete item;
            break;
        }
    }
    setDefaultSiderbarItem();
}

void GroupPage::updateSidebarItem(const QString &groupPath)
{
    KLOG_INFO() << "current Thread:" << QThread::currentThreadId();
    auto itemCount = m_tabList->count();
    for (int i = 0; i < itemCount; i++)
    {
        auto item = m_tabList->item(i);
        if (item->data(ITEM_GROUP_OBJ_PATH_ROLE) == groupPath)
        {
            GroupManager::GroupInfo groupInfo;
            if (GroupManager::instance()->getGroupInfo(groupPath, groupInfo))
            {
                item->setText(groupInfo.name);
                m_tabList->setCurrentItem(item);

                //  更新用户组信息页面
                m_pageGroupInfo->setCurrentShowGroupPath(groupPath);
                // 切换到用户组信息
                m_stackWidget->setCurrentIndex(PAGE_GROUP_INFO);
            }
        }
    }
}

void GroupPage::initPageCreateGroup()
{
    // 创建用户组
    connect(m_pageCreateGroup, &CreateGroupPage::requestCreateGroup,
            m_groupInterface, &GroupInterface::doCreateGroup);
    connect(m_groupInterface, &GroupInterface::sigCreateGroupDone,
            m_pageCreateGroup, &CreateGroupPage::handleGroupAdded);
}

void GroupPage::initPageGroupInfo()
{
    connect(m_pageGroupInfo, &GroupInfoPage::requestAddUsersPage, [this](QString groupPath)
            { m_stackWidget->setCurrentIndex(PAGE_ADD_USERS);
            m_pageAddUsers->updateUsersList(groupPath); });

    // 从用户组移除用户
    connect(m_pageGroupInfo, &GroupInfoPage::requestRemoveMember,
            m_groupInterface, &GroupInterface::doRemoveMemberFromGroup);
    connect(m_groupInterface, &GroupInterface::sigRemoveMemberFromGroupDone,
            m_pageGroupInfo, &GroupInfoPage::handleMemberRemoved);

    // 添加用户到用户组
    connect(m_groupInterface, &GroupInterface::sigAddUserToGroupDone,
            m_pageGroupInfo, &GroupInfoPage::handleMemberAdded);

    // 删除用户组
    connect(m_pageGroupInfo, &GroupInfoPage::requestDeleteGroup,
            m_groupInterface, &GroupInterface::doDeleteGroup);
    connect(m_groupInterface, &GroupInterface::sigDeleteGroupDone,
            m_pageGroupInfo, &GroupInfoPage::handleGroupDeleted);

    // 更改用户组名
    connect(m_pageGroupInfo, &GroupInfoPage::requestChangeGroupName,
            m_groupInterface, &GroupInterface::doChangeGroupName);
    connect(m_groupInterface, &GroupInterface::sigChangeGroupNameDone,
            m_pageGroupInfo, &GroupInfoPage::handleGroupNameChanged);
}

void GroupPage::initPageAddUsers()
{
    connect(m_pageAddUsers, &AddUsersPage::requestGroupInfoPage, [this]()
            { m_stackWidget->setCurrentIndex(PAGE_GROUP_INFO); });

    // 添加用户到用户组
    connect(m_pageAddUsers, &AddUsersPage::requestAddUserToGroup,
            m_groupInterface, &GroupInterface::doAddUserToGroup);
    connect(m_groupInterface, &GroupInterface::sigAddUserToGroupDone,
            m_pageAddUsers, &AddUsersPage::updateUI);
}

void GroupPage::connectToInfoChange()
{
    connect(GroupManager::instance(), &GroupManager::GroupAdded, this, &GroupPage::addGroup);
    connect(GroupManager::instance(), &GroupManager::GroupDeleted, this, &GroupPage::deleteGroup);
    connect(GroupManager::instance(), &GroupManager::GroupChanged, this, &GroupPage::handleGroupChanged);
}

void GroupPage::setDefaultSiderbarItem()
{
    m_tabList->setCurrentRow(0);
}

QSize GroupPage::sizeHint() const
{
    return {780, 657};
}

void GroupPage::jumpToPage(StackWidgetPageEnum page)
{
    if (page == GroupPage::PAGE_CREATE_GROUP)
    {
        m_tabList->setCurrentRow(0);
    }
    else if (page == GroupPage::PAGE_GROUP_INFO)
    {
        // 默认跳转至第一个用户组界面，若无用户组，则跳转至创建界面
        m_tabList->count() >= 2 ? m_tabList->setCurrentRow(1) : m_tabList->setCurrentRow(0);
    }
}

void GroupPage::addGroup(const QString &groupPath)
{
    KLOG_DEBUG() << "on group added, add group" << groupPath << "to sidebar";
    appendSidebarItem(groupPath);
}

void GroupPage::deleteGroup(const QString &groupPath)
{
    KLOG_DEBUG() << "on group deleted, delete group" << groupPath << "from sidebar";
    deleteSidebarItem(groupPath);
}

void GroupPage::handleGroupChanged(const QString &groupPath)
{
    KLOG_DEBUG() << "on group changed, update group" << groupPath << "from sidebar";
    updateSidebarItem(groupPath);
}
