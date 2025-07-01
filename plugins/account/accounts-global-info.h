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
#ifndef ACCOUNTSGLOBALINFO_H
#define ACCOUNTSGLOBALINFO_H

#include "kiran-account-service-wrapper.h"
#include <QList>
#include <QObject>

class AccountsGlobalInfo : public QObject
{
    Q_OBJECT
private:
    explicit AccountsGlobalInfo(QObject *parent = nullptr);

public:
    ~AccountsGlobalInfo();

    static AccountsGlobalInfo *instance();
    static QString rsaPublicKey();

    bool init(bool showRoot, bool showPasswordExpirationPolicy);

    // 获取排序之后的用户列表
    QList<QString> getUserObjectPathList();
    QList<QString> getUserNameList();

    //检查是否存在重名用户
    bool checkUserNameAvaliable(const QString &userName);

    QString getCurrentUser();

    // 获取是否显示密码过期策略
    bool getShowPasswordExpirationPolicy();

private:
    void addUserToMap(const QDBusObjectPath &user);
    void deleteUserFromMap(const QDBusObjectPath &user);

signals:
    void UserAdded(const QString& objectPath);
    void UserDeleted(const QString& obj);
    void UserPropertyChanged(QString userPath,
                             QString propertyName,
                             QVariant value);

private Q_SLOTS:
    void handlerPropertyChanged(const QString &propertyName, const QVariant &value);

private:
    DBusWrapper::KiranAccountServicePtr m_accountsInterface;
    QMap<QString,DBusWrapper::KiranAccountServiceUserPtr> m_usersMap; // QMap<DBus对象路径,用户相关接口>
    QString m_curUserName;
    QString m_pubkey;
    bool m_showPasswordExpirationPolicy;
};

#endif  // ACCOUNTSGLOBALINFO_H
