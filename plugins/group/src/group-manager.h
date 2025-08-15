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

#include <QDBusObjectPath>
#include <QList>
#include <QObject>
#include <QSharedPointer>

class KSDGroupAdminProxy;
class KSDGroupAdminListProxy;
class GroupInterface;
class GroupManager : public QObject
{
    Q_OBJECT
private:
    explicit GroupManager(QObject *parent = nullptr);

public:
    struct GroupInfo
    {
        QString name;
        qlonglong gid;
        QStringList users;
        bool isNotSystemGroup = false;
    };

public:
    ~GroupManager();

    static GroupManager *instance();

    /**
     * @brief 初始化，加载用户组列表
     * @return
     */
    bool init();

    GroupInterface *getInterface();

    /**
     * @brief  获取排序之后的用户组列表
     * @return QList<QString> 用户DBusObjectPath列表
     */
    QList<QString> getGroupList();

    /// @brief 获取用户组信息
    /// @param groupPath 用户组DBus对象路径
    /// @param groupInfo 存储用户组信息
    /// @return 是否获取成功
    bool getGroupInfo(const QString &groupPath, GroupManager::GroupInfo &groupInfo);

    /**
     * @brief 检查是否存在重名用户组
     * @param groupName 需检查的用户组名
     * @return 是否可用
     */
    bool checkGroupNameAvaliable(const QString &groupName);

protected slots:
    void handlerGroupChanged(const QDBusObjectPath &group);

private:
    void addGroupToMap(const QDBusObjectPath &group);
    void deleteGroupFromMap(const QDBusObjectPath &group);

signals:
    void GroupAdded(const QString &groupPath);
    void GroupDeleted(const QString &groupPath);
    void GroupChanged(const QString &groupPath);

private:
    KSDGroupAdminProxy *m_groupAdminProxy;
    QMap<QString, QSharedPointer<KSDGroupAdminListProxy>> m_groupsMap;  // QMap<DBus对象路径,用户相关接口>
    GroupInterface *m_groupInterface;
};
