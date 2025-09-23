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
 * Author:     yuanxing <yuanxing@kylinsec.com.cn>
 */

#include "change-host-name-widget.h"
#include "dbus-wrapper/system-info-dbus.h"
#include "ui_change-host-name-widget.h"

#include <QDesktopWidget>
#include <QMessageBox>
#include <QScreen>

#include <kiran-push-button.h>
#include <kiranwidgets-qt5/kiran-message-box.h>

ChangeHostNameWidget::ChangeHostNameWidget(QWidget* parent)
    : KiranTitlebarWindow(parent,Qt::Dialog),
      ui(new Ui::ChangeHostNameWidget)
{
    ui->setupUi(getWindowContentWidget());
    setButtonHints(TitlebarMinimizeButtonHint | TitlebarCloseButtonHint);
    setContentWrapperMarginBottom(0);
    setResizeable(false);
    setTitlebarColorBlockEnable(true);

    KiranPushButton::setButtonType(ui->btn_save, KiranPushButton::BUTTON_Default);
    initUI();
    connect(ui->btn_cancel, SIGNAL(clicked()), this, SLOT(close()));
    connect(ui->btn_save, SIGNAL(clicked()), this, SLOT(setNewHostName()));
    connect(ui->lineEdit_input_name, SIGNAL(textChanged(QString)), SLOT(setInputEditStatus()));
}

ChangeHostNameWidget::~ChangeHostNameWidget()
{
    delete ui;
}

bool ChangeHostNameWidget::getLineEditStatus()
{
    if (ui->lineEdit_input_name->text() != NULL)
        return true;
    else
        return false;
}

void ChangeHostNameWidget::initUI()
{
    setTitle("");
    setIcon(QIcon(":/images/kylin-about.png"));

    ui->btn_save->setEnabled(false);
}

void ChangeHostNameWidget::setNewHostName()
{
    QString errorMsg;
    QString newName = ui->lineEdit_input_name->text();
    if (!SystemInfoDBus::setHostName(newName, errorMsg))  // 调用Dbus接口不成功，弹出警告窗口
    {
        QString title = QString(tr("Warning"));
        if( errorMsg.isEmpty())
        {
            errorMsg = tr("Unknown error");
        }
        QString text = QString(tr("Change host name failed, %1").arg(errorMsg));
        KiranMessageBox::message(nullptr, title, text, KiranMessageBox::Ok);
    }
    else
    {
        emit hostnameChanged(newName);
    }
    this->close();
}

void ChangeHostNameWidget::setInputEditStatus()
{
    if (ui->lineEdit_input_name->text().isEmpty())
    {
        ui->btn_save->setEnabled(false);
    }
    else
    {
        ui->btn_save->setEnabled(true);
    }
}
