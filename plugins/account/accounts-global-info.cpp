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
#include "accounts-global-info.h"
#include "config.h"
#include "logging-category.h"

#include <kiran-system-daemon/accounts-i.h>
#include <qt5-log-i.h>
#include <unistd.h>
#include <QDBusObjectPath>
#include <QDBusPendingCall>
#include <QList>
#include <QMutex>
#include <QScopedPointer>

AccountsGlobalInfo::AccountsGlobalInfo(QObject *parent)
    : QObject(parent),
      m_accountsInterface(DBusWrapper::Account::interface())
{
}

AccountsGlobalInfo::~AccountsGlobalInfo()
{
}

AccountsGlobalInfo *AccountsGlobalInfo::instance()
{
    static QMutex mutex;
    static QScopedPointer<AccountsGlobalInfo> pInst;

    if (Q_UNLIKELY(!pInst))
    {
        QMutexLocker locker(&mutex);
        if (pInst.isNull())
        {
            pInst.reset(new AccountsGlobalInfo);
        }
    }

    return pInst.data();
}

bool AccountsGlobalInfo::init(bool showRoot, bool showPasswordExpirationPolicy)
{
    if (m_accountsInterface.isNull())
    {
        return false;
    }

    connect(m_accountsInterface.data(), &AccountInterface::UserAdded,
            [this](const QDBusObjectPath &user)
            { addUserToMap(user); });
    connect(m_accountsInterface.data(), &AccountInterface::UserDeleted,
            [this](const QDBusObjectPath &user)
            { deleteUserFromMap(user); });
    connect(m_accountsInterface.data(), &AccountInterface::rsa_public_keyChanged,
            [this](const QString &publicKey)
            { m_pubkey = publicKey; });

    // 判断是否显示ROOT用户
    KLOG_DEBUG(qLcAccount, "show root:%s", showRoot ? "true" : "false");

    // 加载用户
    QDBusPendingReply<QList<QDBusObjectPath>> getUsersReply;
    QList<QDBusObjectPath> objList;
    QList<QDBusObjectPath>::iterator objListIter;
    getUsersReply = m_accountsInterface->GetNonSystemUsers();
    getUsersReply.waitForFinished();
    if (getUsersReply.isError())
    {
        KLOG_ERROR(qLcAccount) << "get non system users error:"
                               << getUsersReply.error();
        return false;
    }
    objList = getUsersReply.value();

    if (showRoot)
    {
        // 将Root用户加入链表中
        auto getRootReply = m_accountsInterface->FindUserById(0);
        getRootReply.waitForFinished();
        if (getRootReply.isError())
        {
            KLOG_ERROR(qLcAccount) << "insert root to users cache failed,"
                                   << "cant find root by id:" << getRootReply.error();
        }
        else
        {
            objList.insert(0, getRootReply.value());
        }
    }

    for (objListIter = objList.begin();
         objListIter != objList.end();
         ++objListIter)
    {
        addUserToMap(*objListIter);
    }

    // 获取当前用户
    do
    {
        uid_t uid = getuid();
        auto findUserReply = m_accountsInterface->FindUserById(uid);
        findUserReply.waitForFinished();

        if (findUserReply.isError())
        {
            KLOG_WARNING(qLcAccount) << "find current user failed!";
            break;
        }

        auto userObjectPath = findUserReply.value().path();
        auto userAPI = DBusWrapper::Account::userInterface(userObjectPath);
        if (userAPI.isNull())
        {
            KLOG_WARNING(qLcAccount) << "get current user name failed";
            break;
        }

        KLOG_DEBUG(qLcAccount) << "current user:" << userAPI->user_name();
        m_curUserName = userAPI->user_name();
    } while (0);

    m_pubkey = m_accountsInterface->rsa_public_key();
    m_showPasswordExpirationPolicy = showPasswordExpirationPolicy;
    return true;
}

QList<QString> AccountsGlobalInfo::getUserObjectPathList()
{
    QList<QString> userObjPathList;
    for (auto iter = m_usersMap.begin();
         iter != m_usersMap.end();
         iter++)
    {
        userObjPathList.append((*iter)->path());
    }
    return userObjPathList;
}

QList<QString> AccountsGlobalInfo::getUserNameList()
{
    QList<QString> userlist;
    for (auto iter = m_usersMap.begin();
         iter != m_usersMap.end();
         iter++)
    {
        userlist.append((*iter)->user_name());
    }
    return userlist;
}

bool AccountsGlobalInfo::checkUserNameAvaliable(const QString &userName)
{
    bool isValid = true;

    for (auto &iter : m_usersMap)
    {
        if (iter->user_name() == userName)
        {
            isValid = false;
            break;
        }
    }

    return isValid;
}

QString AccountsGlobalInfo::getCurrentUser()
{
    return m_curUserName;
}

bool AccountsGlobalInfo::getShowPasswordExpirationPolicy()
{
    return m_showPasswordExpirationPolicy;
}

QString AccountsGlobalInfo::rsaPublicKey()
{
    return AccountsGlobalInfo::instance()->m_pubkey;
}

void AccountsGlobalInfo::addUserToMap(const QDBusObjectPath &user)
{
    if (m_usersMap.find(user.path()) != m_usersMap.end())
    {
        return;
    }

    auto accountAPI = DBusWrapper::Account::userInterface(user.path());
    if (accountAPI.isNull())
    {
        KLOG_WARNING(qLcAccount) << "add user" << user.path() << "to cache failed!";
        return;
    }

    connect(accountAPI.data(),
            &AccountUserInterface::dbusPropertyChanged,
            this,
            &AccountsGlobalInfo::handlerPropertyChanged);

    m_usersMap.insert(user.path(), accountAPI);
    emit UserAdded(user.path());
}

void AccountsGlobalInfo::deleteUserFromMap(const QDBusObjectPath &user)
{
    KLOG_DEBUG(qLcAccount) << "delete user from cache:" << user.path();

    auto userIter = m_usersMap.find(user.path());
    if (userIter == m_usersMap.end())
    {
        KLOG_WARNING(qLcAccount) << "not find user in cache:" << user.path();
        return;
    }

    disconnect(userIter.value().data(),
               &AccountUserInterface::dbusPropertyChanged,
               this,
               &AccountsGlobalInfo::handlerPropertyChanged);

    m_usersMap.erase(userIter);
    emit UserDeleted(user.path());
}

void AccountsGlobalInfo::handlerPropertyChanged(const QString &propertyName, const QVariant &value)
{
    auto userProxy = qobject_cast<AccountUserInterface *>(sender());

    KLOG_DEBUG(qLcAccount) << "property changed:" << userProxy->path();
    KLOG_DEBUG(qLcAccount) << "\tname: " << propertyName;
    KLOG_DEBUG(qLcAccount) << "\tvalue:" << value;

    emit UserPropertyChanged(userProxy->path(), propertyName, value);
}
