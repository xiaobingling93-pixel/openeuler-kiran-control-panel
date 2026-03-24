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
#include "group-manager.h"
#include "group-interface.h"
#include "ksd_group_admin_list_proxy.h"
#include "ksd_group_admin_proxy.h"

#include <kiran-system-daemon/groups-i.h>
#include <qt5-log-i.h>
#include <QDBusObjectPath>
#include <QDBusPendingCall>
#include <QMutex>
#include <QScopedPointer>

GroupManager::GroupManager(QObject *parent)
    : QObject(parent),
      m_groupAdminProxy(new KSDGroupAdminProxy(GROUPS_DBUS_NAME, GROUPS_OBJECT_PATH, QDBusConnection::systemBus()))
{
}

GroupManager::~GroupManager()
{
    if (m_groupAdminProxy)
    {
        delete m_groupAdminProxy;
        m_groupAdminProxy = nullptr;
    }

    m_groupsMap.clear();
}

GroupManager *GroupManager::instance()
{
    static QMutex mutex;
    static QScopedPointer<GroupManager> pInst;

    if (Q_UNLIKELY(!pInst))
    {
        QMutexLocker locker(&mutex);
        if (pInst.isNull())
        {
            pInst.reset(new GroupManager);
        }
    }

    return pInst.data();
}

bool GroupManager::init()
{
    connect(m_groupAdminProxy, &KSDGroupAdminProxy::GroupAdded, [this](const QDBusObjectPath &group)
            { addGroupToMap(group); });
    connect(m_groupAdminProxy, &KSDGroupAdminProxy::GroupDeleted, [this](const QDBusObjectPath &group)
            { deleteGroupFromMap(group); });

    QList<QDBusObjectPath> groups;
    QDBusPendingReply<QList<QDBusObjectPath>> pendingReply;
    QList<QDBusObjectPath> objList;
    QList<QDBusObjectPath>::iterator objListIter;

    // 获取所有用户组
    pendingReply = m_groupAdminProxy->ListCachedGroups();
    pendingReply.waitForFinished();
    if (pendingReply.isError())
    {
        KLOG_ERROR() << "GetCachedGroups Error:"
                     << pendingReply.error();
        return false;
    }

    objList = pendingReply.value();
    // 将用户组列表添加到表中
    for (objListIter = objList.begin();
         objListIter != objList.end();
         ++objListIter)
    {
        addGroupToMap(*objListIter);
    }
    return true;
}

QList<QString> GroupManager::getGroupList()
{
    QList<QString> groupObjPathList;
    for (auto iter = m_groupsMap.begin();
         iter != m_groupsMap.end();
         iter++)
    {
        groupObjPathList.append((*iter)->path());
    }
    return groupObjPathList;
}

bool GroupManager::getGroupInfo(const QString &groupPath, GroupManager::GroupInfo &groupInfo)
{
    KSDGroupAdminListProxy interface(GROUPS_DBUS_NAME, groupPath, QDBusConnection::systemBus());
    if (!interface.isValid())
    {
        KLOG_WARNING() << "Failed to get group info:" << interface.lastError().message();
        return false;
    }
    KLOG_DEBUG() << "Get group info of" << groupPath << "from dbus:"
                 << "name:" << interface.name()
                 << "gid:" << interface.gid()
                 << "users:" << interface.users();

    groupInfo.name = interface.name();
    groupInfo.gid = interface.gid();
    groupInfo.users = interface.users();
    groupInfo.isNotSystemGroup = interface.gid() >= 1000 && interface.gid() < 65534;
    return true;
}

bool GroupManager::checkGroupNameAvaliable(const QString &groupName)
{
    bool isValid = true;

    for (auto iter : m_groupsMap)
    {
        if (iter.data()->name() == groupName)  // KSDGroupAdminListProxy的成员函数groupName()
        {
            isValid = false;
            break;
        }
    }

    return isValid;
}

void GroupManager::addGroupToMap(const QDBusObjectPath &group)
{
    if (m_groupsMap.find(group.path()) != m_groupsMap.end())
    {
        return;
    }

    auto groupProxy = QSharedPointer<KSDGroupAdminListProxy>::create(GROUPS_DBUS_NAME,
                                                                     group.path(),
                                                                     QDBusConnection::systemBus(),
                                                                     this);

    connect(groupProxy.data(),
            &KSDGroupAdminListProxy::GroupChanged,
            this,
            &GroupManager::handlerGroupChanged);

    m_groupsMap.insert(group.path(), groupProxy);

    emit GroupAdded(group.path());
}

void GroupManager::deleteGroupFromMap(const QDBusObjectPath &group)
{
    if (m_groupsMap.find(group.path()) == m_groupsMap.end())
    {
        return;
    }

    auto groupProxy = m_groupsMap.take(group.path());
    disconnect(groupProxy.data(),
               &KSDGroupAdminListProxy::GroupChanged,
               this,
               &GroupManager::handlerGroupChanged);

    emit GroupDeleted(group.path());
}

void GroupManager::handlerGroupChanged(const QDBusObjectPath &group)
{
    emit GroupChanged(group.path());
}
