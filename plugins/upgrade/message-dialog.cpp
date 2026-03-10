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

#include "message-dialog.h"
#include <QVBoxLayout>
#include "widgets/custom-plain-text-edit.h"

#include <kiran-color-block.h>

MessageDialog::MessageDialog(QWidget *parent)
    : KiranTitlebarWindow(parent, Qt::Dialog),
      m_textBrowser(nullptr)
{
    setWindowModality(Qt::ApplicationModal);
    setButtonHints(TitlebarCloseButtonHint);
    setTitlebarColorBlockEnable(true);
    setFixedSize(500, 500);

    // 获取窗口内容部件
    auto contentWidget = getWindowContentWidget();

    // 创建布局
    auto layout = new QVBoxLayout(contentWidget);
    layout->setContentsMargins(4, 4, 4, 4);
    layout->setSpacing(0);

    // 内容区域
    auto messageWidget = new KiranColorBlock(this);
    layout->addWidget(messageWidget);

    auto messageLayout = new QVBoxLayout(messageWidget);
    messageLayout->setContentsMargins(0, 0, 0, 0);
    messageLayout->setSpacing(0);

    // 创建 CustomPlainTextEdit 控件
    m_textBrowser = new CustomPlainTextEdit(messageWidget);
    messageLayout->addWidget(m_textBrowser);
}

MessageDialog::~MessageDialog()
{
}

void MessageDialog::setMessage(const QString &info)
{
    m_textBrowser->clear();
    m_textBrowser->setPlainText(info);
}

void MessageDialog::clearMessage()
{
    m_textBrowser->clear();
}
