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

#include <QObject>

class GroupInterface : public QObject
{
    Q_OBJECT
public:
    explicit GroupInterface();

    ~GroupInterface();

public slots:

    /// 通过groupAdmin创建用户组
    /// \param groupName         用户组名
    void doCreateGroup(const QString &groupName, const QStringList &users);

    /// 通过groupAdmin删除用户组
    /// \param groupid  删除用户组的ID
    void doDeleteGroup(int gid, const QString &groupName);

    /// 通过groupAdminList将用户添加到用户组
    /// \param groupPath  需要执行添加成员操作的用户组DBus对象路径
    /// \param userName   要添加的用户名
    void doAddUserToGroup(const QString &groupPath, const QStringList &userNameList);

    /// 通过groupAdminList将成员移出用户组
    /// \param groupPath  需要执行移出成员操作的用户组DBus对象路径
    /// \param userName   要移出的成员名列表
    void doRemoveMemberFromGroup(const QString &groupPath, const QString &userName);

    /// 通过groupAdminList修改组名
    /// \param groupPath  需要修改组名的用户组DBus对象路径
    /// \param groupName  目标组名
    void doChangeGroupName(const QString &groupPath, const QString &groupName);

signals:

    /// 创建用户组完成信号
    /// \param userPath 创建完成的用户组DBus对象路径,若创建用户都失败，则为空
    /// \param errMsg   错误消息框，收到弹出提示框
    void sigCreateGroupDone(const QString &userPath, const QString &errMsg);

    /// 删除用户组完成信号
    /// \param errMsg 错误消息，不为空表示失败，弹出提示框
    void sigDeleteGroupDone(const QString &groupName, const QString &errMsg);

    /// 添加用户完成信号
    /// \param errMsg 错误信息，不为空表示失败，弹出提示框
    void sigAddUserToGroupDone(const QString &errMsg);

    /// 移出成员完成信号
    /// \param errMsg 错误信息，不为空表示失败，弹出提示框
    void sigRemoveMemberFromGroupDone(const QString &errMsg);

    /// 修改组名完成信号
    /// \param errMsg 错误信息，不为空表示失败，弹出提示框
    void sigChangeGroupNameDone(const QString &groupPath, const QString &errMsg);
};
