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

#include "deps-dialog.h"
#include "ui_deps-dialog.h"

#include <kiran-push-button.h>
#include <palette.h>

using namespace Kiran::Theme;

DepsDialog::DepsDialog(QWidget *parent)
    : KiranTitlebarWindow(parent, Qt::Dialog),
      ui(new Ui::DepsDialog)
{
    ui->setupUi(getWindowContentWidget());

    setTitle(tr("Dependency Information"));
    setWindowModality(Qt::ApplicationModal);
    setButtonHints(TitlebarCloseButtonHint);
    setTitlebarColorBlockEnable(true);

    // 为依赖信息控件设置边框
    ui->textBrowser->viewport()->setAutoFillBackground(false);
    auto borderColor = DEFAULT_PALETTE()->getColor(Palette::ColorGroup::ACTIVE, Palette::ColorRole::BORDER);
    ui->textBrowser->setStyleSheet(QString("QTextBrowser { border: 1px solid %1; border-radius: 6px; }").arg(borderColor.name()));

    ui->icon_tip->setIcon(QIcon::fromTheme("ksvg-tip"));
    KiranPushButton::setButtonType(ui->btn_confirm, KiranPushButton::BUTTON_Default);

    connect(ui->btn_confirm, &QPushButton::clicked, this, [this]()
            {
                emit confirmed();
                close();
            });
    connect(ui->btn_cancel, &QPushButton::clicked, this, &DepsDialog::close);
}

DepsDialog::~DepsDialog()
{
    delete ui;
}

void DepsDialog::setPkgDepsInfo(const QString &pkgDepsInfo)
{
    if (pkgDepsInfo.isEmpty())
    {
        ui->textBrowser->setPlainText(tr("No dependency information"));
        return;
    }
    ui->textBrowser->setPlainText(pkgDepsInfo);
}

void DepsDialog::clearDeps()
{
    ui->textBrowser->clear();
}