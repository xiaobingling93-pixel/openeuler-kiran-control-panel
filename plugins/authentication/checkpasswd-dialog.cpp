/**
 * Copyright (c) 2020 ~ 2024 KylinSec Co., Ltd.
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
#include "checkpasswd-dialog.h"
#include "logging-category.h"

#include <pwd.h>
#include <qt5-log-i.h>
#include <security/pam_appl.h>
#include <unistd.h>

int conv_func(int num_msg, const struct pam_message **msg,
              struct pam_response **resp, void *appdata_ptr)
{
    struct pam_response *reply = NULL;
    int ret;
    int replies;
    char *passwd = static_cast<char *>(appdata_ptr);

    /// 分配回复包
    reply = static_cast<struct pam_response *> ( calloc(num_msg, sizeof(*reply)));
    if (reply == nullptr)
    {
        return PAM_CONV_ERR;
    }

    ret = PAM_SUCCESS;
    // 给每个ECHO_OFF消息填充密码,若出现ECHO_ON消息认证失败，释放
    for (replies = 0; replies < num_msg && ret == PAM_SUCCESS; replies++)
    {
        if (msg[replies]->msg_style == PAM_PROMPT_ECHO_ON)
        {
            goto failed;
        }
        std::string passwdStr(passwd);
        reply[replies].resp = new char[passwdStr.size() + 1]();
        std::copy(passwdStr.begin(), passwdStr.end(), reply[replies].resp);
        reply[replies].resp[passwdStr.size()] = '\0';
        reply[replies].resp_retcode = PAM_SUCCESS;
    }
    *resp = reply;
    return PAM_SUCCESS;

failed:
    /// 释放之前分配的内存
    for (int i = 0; i < replies; i++)
    {
        if (reply[i].resp != nullptr)
        {
            delete[] reply[i].resp;
        }
    }
    free(reply);
    return PAM_CONV_ERR;
}

void no_fail_delay(int status, unsigned int delay, void *appdata_ptr)
{
}

bool _checkUserPassword(const QString &user, const QString &pwd)
{
    KLOG_DEBUG(qLcAuthentication) << "check user password for" << user;

    std::string sPwd = pwd.toStdString();
    struct pam_conv conv = {
        &conv_func,
        const_cast<void*>(static_cast<const void *>(sPwd.c_str()))};

    pam_handle *handler;
    int res;

    res = pam_start("password-auth", user.toStdString().c_str(),
                    &conv,
                    &handler);
    if (res != PAM_SUCCESS)
    {
        KLOG_WARNING(qLcAuthentication) << "check user" << user
                                << "passwd failed,pam_start failed";
        return false;
    }

    pam_set_item(handler, PAM_FAIL_DELAY, reinterpret_cast<const void *>(no_fail_delay));

    res = pam_authenticate(handler, 0);
    if (res != PAM_SUCCESS)
    {
        KLOG_WARNING(qLcAuthentication) << "check user" << user
                                        << "passwd failed,"
                                        << pam_strerror(handler, res) << res;
    }

    pam_end(handler, res);
    KLOG_DEBUG(qLcAuthentication) << "check user passwd finished";
    return res == PAM_SUCCESS;
}

QString getCurrentUser()
{
    auto pwd = getpwuid(getuid());
    return pwd ? pwd->pw_name : "";
}

CheckpasswdDialog::CheckpasswdDialog(QWidget *parent)
    : KiranInputDialog(parent)
{
    setTitle(tr("Check password"));
    setDesc(tr("Check the current password before you enroll the feature"));
    setInputMode(QLineEdit::Password, 32);
}

CheckpasswdDialog::~CheckpasswdDialog()
{
}

bool CheckpasswdDialog::checkPasswd(const QString &passwd)
{
    auto currentUser = getCurrentUser();
    if (currentUser.isEmpty())
    {
        return false;
    }

    return _checkUserPassword(currentUser, passwd);
}