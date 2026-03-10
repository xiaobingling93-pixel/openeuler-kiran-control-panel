/**
 * Copyright (c) 2020 ~ 2025 KylinSec Co., Ltd.
 * kiran-control-panel is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 *
 * Author:     yuanxing <yuanxing@kylinsec.com.cn>
 */

#pragma once

#include <kiran-titlebar-window.h>
#include <QString>

class CustomPlainTextEdit;

class MessageDialog : public KiranTitlebarWindow
{
    Q_OBJECT
public:
    explicit MessageDialog(QWidget *parent = nullptr);
    ~MessageDialog();

    void setMessage(const QString &info);
    void clearMessage();

private:
    CustomPlainTextEdit *m_textBrowser;
};
