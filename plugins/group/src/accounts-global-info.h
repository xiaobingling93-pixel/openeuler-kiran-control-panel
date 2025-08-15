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

#ifndef ACCOUNTSGLOBALINFO_H
#define ACCOUNTSGLOBALINFO_H

#include <QDBusObjectPath>
#include <QList>
#include <QObject>
#include <QSharedPointer>

#define ACCOUNTS_DBUS_NAME "com.kylinsec.Kiran.SystemDaemon.Accounts"
#define ACCOUNTS_OBJECT_PATH "/com/kylinsec/Kiran/SystemDaemon/Accounts"
#define ACCOUNTS_DBUS_INTERFACE_NAME "com.kylinsec.Kiran.SystemDaemon.Accounts"

class KSDAccountsProxy;
class KSDAccountsUserProxy;
class AccountsGlobalInfo : public QObject
{
    Q_OBJECT
private:
    explicit AccountsGlobalInfo(QObject *parent = nullptr);

public:
    ~AccountsGlobalInfo();

    static AccountsGlobalInfo *instance();

    /**
     * @brief 初始化，加载用户列表
     * @return
     */
    bool init();

    /**
     * @brief  获取排序之后的用户列表
     * @return QList<QString> 用户DBusObjectPath列表
     */
    QList<QString> getUserList();

    /**
     * @brief 获取用户名
     * @param userPath 用户DBusObjectPath
     * @param userName 存储用户名
     * @return 是否获取成功
     */
    bool getUserName(const QString &userPath, QString &userName);

private:
    void addUserToMap(const QDBusObjectPath &user);
    void deleteUserFromMap(const QDBusObjectPath &user);

signals:
    void UserAdded(const QDBusObjectPath &obj);
    void UserDeleted(const QDBusObjectPath &obj);
    void UserPropertyChanged(const QString &userPath,
                             const QString &propertyName,
                             const QVariant &value);

private Q_SLOTS:
    void handlerPropertyChanged(const QString &propertyName, const QVariant &value);

private:
    KSDAccountsProxy *m_accountsInterface;
    QMap<QString, QSharedPointer<KSDAccountsUserProxy>> m_usersMap;  // QMap<DBus对象路径,用户相关接口>
};

#endif  // ACCOUNTSGLOBALINFO_H
