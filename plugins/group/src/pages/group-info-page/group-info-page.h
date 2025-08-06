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
#ifndef GROUPINFOPAGE_H
#define GROUPINFOPAGE_H

#include <QWidget>

namespace Ui
{
class GroupInfoPage;
}

class UsersContainer;
class KiranTips;
class GroupInfoPage : public QWidget
{
    Q_OBJECT

public:
    GroupInfoPage(QWidget *parent = nullptr);
    ~GroupInfoPage();

    /// 设置当前显示的用户组的DBus对象路径
    /// \param groupObj 用户组DBus对象路径
    void setCurrentShowGroupPath(const QString &groupObj);

    QString getGroupName() { return this->m_curShowGroupName; }

signals:
    void requestAddUsersPage(const QString &groupPath);

    /// 向GroupInterface发送移除成员的信号
    /// \param groupPath 要移除成员的用户组DBus对象路径
    /// \param userName 要被移除的成员名
    void requestRemoveMember(const QString &groupPath, const QString &userName);

    /// 向GroupInterface发送移除用户组的信号
    /// \param gid 要移除的用户组id
    void requestDeleteGroup(int gid, const QString &groupName);

    /// 向GroupInterface发送更改用户组名称的信号
    /// \param groupPath 要更改名称的用户组DBus对象路径
    /// \param groupName 新的用户组名
    void requestChangeGroupName(const QString &groupPath, const QString &groupName);

public Q_SLOTS:
    void changeGroupName();
    void handleMemberRemoved(const QString &errMsg);
    void handleMemberAdded(const QString &errMsg);
    void handleGroupDeleted(const QString &groupPath, const QString &errMsg);
    void handleGroupNameChanged(const QString &groupPath, const QString &errMsg);

private:
    /// 初始化界面
    void initUI();
    /// 　从GroupAdmin服务中重新加载用户信息
    void updateInfo();
    void appendMemberListItem(const QString &userName);

    bool eventFilter(QObject *watched, QEvent *event);

private:
    Ui::GroupInfoPage *ui;
    KiranTips *m_errorTip;
    QString m_curShowGroupPath;
    QString m_curShowGroupName;
    UsersContainer *m_memberContainer;
    int m_gid;
};
#endif  // GROUPINFOPAGE_H
