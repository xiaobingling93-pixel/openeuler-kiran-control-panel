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

#include "hard-worker.h"
#include "account.h"
#include "config.h"
#include "logging-category.h"

#include <kiran-system-daemon/accounts-i.h>
#include <qt5-log-i.h>
#include <QDBusConnection>
#include <QDebug>

HardWorker::HardWorker() : QObject(nullptr)
{
}

HardWorker::~HardWorker()
{
}

void HardWorker::doCreateUser(QString userName,
                              int uid,
                              int userType,
                              QString encryptedPasswd,
                              QString homeDir,
                              QString shell,
                              QString iconFile)
{
    auto accountsServiceAPI = DBusWrapper::Account::interface();
    QString userObjPath;
    QString errMsgDetail;

    auto createUserDoneWithError = [this](const QString& errorDetail) -> void
    {
        QString errMsgPrefix = tr("Create User failed");
        QString errMsg = errMsgPrefix;
        if (!errorDetail.isEmpty())
        {
            errMsg.append(",");
            errMsg.append(errorDetail);
        }
        emit sigCreateUserDnoe("", errMsg);
    };

    if (accountsServiceAPI.isNull())
    {
        KLOG_WARNING(qLcAccount) << "create user failed,can not get kiran account service backend!";
        return createUserDoneWithError(tr("Failed to connect to the account management service"));
    }

    /// step1.创建用户
    QDBusPendingReply<QDBusObjectPath> createUserRep;
    createUserRep = accountsServiceAPI->CreateUser(userName,
                                                   userName,
                                                   userType,
                                                   uid);
    createUserRep.waitForFinished();
    if (createUserRep.isError())
    {
        KLOG_WARNING(qLcAccount) << "create user failed," << createUserRep.error();
        errMsgDetail = createUserRep.error().message();
        return createUserDoneWithError(errMsgDetail);
    }

    userObjPath = createUserRep.value().path();
    auto userInterface = DBusWrapper::Account::userInterface(userObjPath);
    auto deleteUserAndReplyError = [this, createUserDoneWithError,
                                    accountsServiceAPI, userInterface](const QString& errorDetail) -> void
    {
        auto uid = userInterface->uid();

        auto reply = accountsServiceAPI->DeleteUser(uid, true);
        reply.waitForFinished();

        createUserDoneWithError(errorDetail);
    };

    /// step2. 设置密码
    QDBusPendingReply<> setpwdRep = userInterface->SetPasswordByPasswd("", encryptedPasswd);
    setpwdRep.waitForFinished();
    if (setpwdRep.isError())
    {
        KLOG_WARNING(qLcAccount) << "set passwd failed," << setpwdRep.error();
        errMsgDetail = setpwdRep.error().message();
        return deleteUserAndReplyError(errMsgDetail);
    }

    /// step3.　设置Home
    if (!homeDir.isEmpty())
    {
        QDBusPendingReply<> setHomeRep = userInterface->SetHomeDirectory(homeDir);
        setHomeRep.waitForFinished();
        if (setHomeRep.isError())
        {
            KLOG_WARNING(qLcAccount) << "set home directory failed," << setHomeRep.error();
            errMsgDetail = setHomeRep.error().message();
            return deleteUserAndReplyError(errMsgDetail);
        }
    }

    /// step4. 设置shell
    QDBusPendingReply<> setShellRep = userInterface->SetShell(shell.isEmpty() ? ACCOUNT_DEFAULT_SHELL : shell);
    setShellRep.waitForFinished();
    if (setShellRep.isError())
    {
        KLOG_WARNING(qLcAccount) << "set shell failed," << setShellRep.error();
        errMsgDetail = setShellRep.error().message();
        return deleteUserAndReplyError(errMsgDetail);
    }

    /// step5. 设置图标
    QDBusPendingReply<> setIconRep = userInterface->SetIconFile(iconFile);
    setIconRep.waitForFinished();
    if (setIconRep.isError())
    {
        KLOG_WARNING(qLcAccount) << "set icon failed," << setIconRep.error();
        errMsgDetail = setIconRep.error().message();
        return deleteUserAndReplyError(errMsgDetail);
    }

    KLOG_INFO(qLcAccount,"create user(%s) is done",userName.toStdString().c_str());
    emit sigCreateUserDnoe(userObjPath, "");
    return;
}

void HardWorker::doUpdatePasswd(QString objPath,
                                QString userName,
                                QString encryptedCurPasswd,
                                QString encryptedPasswd)
{
    auto userProxy = DBusWrapper::Account::userInterface(objPath);

    QDBusPendingReply<> reply = userProxy->SetPasswordByPasswd(encryptedCurPasswd, encryptedPasswd);
    reply.waitForFinished();

    if (reply.isError())
    {
        KLOG_ERROR(qLcAccount) << "set passwd failed," << reply.error();
        QString errMsg = QString("%1,%2").arg(tr(" update password failed")).arg(reply.error().message());
        emit sigUpdatePasswdDone(errMsg);
    }
    else
    {
        KLOG_INFO(qLcAccount) << "update passwd is done";
        emit sigUpdatePasswdDone("");
    }
}

void HardWorker::doUpdateUserProperty(QString objPath,
                                      QString userName,
                                      QString iconfile,
                                      int userType,
                                      bool isLocked)
{
    auto userProxy = DBusWrapper::Account::userInterface(objPath);
    QStringList updateFailedPropertys;

    if (userProxy->icon_file() != iconfile)
    {
        auto reply = userProxy->SetIconFile(iconfile);
        reply.waitForFinished();
        if (reply.isError())
        {
            KLOG_WARNING(qLcAccount) << "update icon file failed," << reply.error();
            updateFailedPropertys.append(tr("icon file"));
        }
    }

    if (userProxy->account_type() != userType)
    {
        auto reply = userProxy->SetAccountType(userType);
        reply.waitForFinished();
        if (reply.isError())
        {
            KLOG_WARNING(qLcAccount) << "update userName type failed," << reply.error();
            updateFailedPropertys.append(tr("userName type"));
        }
    }

    if (userProxy->locked() != isLocked)
    {
        auto reply = userProxy->SetLocked(isLocked);
        reply.waitForFinished();
        if (reply.isError())
        {
            KLOG_WARNING(qLcAccount) << "update locked failed," << reply.error();
            updateFailedPropertys.append(tr("locked"));
        }
    }

    /// 更新属性失败
    if (!updateFailedPropertys.isEmpty())
    {
        QString updateFailed = updateFailedPropertys.join(",");
        KLOG_WARNING(qLcAccount,"failed to update user properties: %s",updateFailed.toStdString().c_str());

        QString msg = QString(tr("Failed to update user properties,%1"))
                          .arg(updateFailed);
        emit sigUpdateUserPropertyDone(msg);
    }
    else
    {
        KLOG_INFO(qLcAccount) << "update user property done";
        emit sigUpdateUserPropertyDone("");
    }
}

void HardWorker::doDeleteUser(int uid)
{
    auto accountsProxy = DBusWrapper::Account::interface();

    auto reply = accountsProxy->DeleteUser(uid, true);
    reply.waitForFinished();

    if (reply.isError())
    {
        KLOG_INFO(qLcAccount) << "delete user error:" << reply.error();

        QString errMsg = QString(tr("Failed to delete user,%1")).arg(reply.error().message());
        emit sigDeleteUserDone(errMsg);
        return;
    }

    emit sigDeleteUserDone("");
}
