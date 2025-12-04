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
#include "license-agreement.h"
#include "ui_license-agreement.h"

#include <kiran-log/qt5-log-i.h>
#include <kiran-push-button.h>
#include <kiranwidgets-qt5/kiran-message-box.h>

#include <QDesktopWidget>
#include <QFile>
#include <QFileDialog>
#include <QIcon>
#include <QLocale>
#include <QMessageBox>
#include <QStandardPaths>
#include <QTextDocument>
#include <QtPrintSupport/QPrinter>

#define EULAFILE "/usr/share/kylin-release"
#define LICENSEFILE "/usr/share/doc/kylin-release/LICENSE"
#define PRIVACYFILE "/usr/share/kylin-release/privacy_policy"

enum LicenseType
{
    EULA_LICENSE = 0,
    VERSION_LICENSE,
    PRIVACY_POLICY
};

LicenseAgreement::LicenseAgreement(QWidget *parent, Qt::WindowFlags windowFlags)
    : KiranTitlebarWindow(parent),
      ui(new Ui::LicenseAgreement)
{
    ui->setupUi(getWindowContentWidget());

    setIcon(QIcon(":/images/kylin-about.png"));
    setButtonHints(TitlebarMinimizeButtonHint | TitlebarCloseButtonHint);
    setResizeable(false);
    setTitlebarColorBlockEnable(true);
    ui->text_license->viewport()->setAutoFillBackground(false);

    KiranPushButton::setButtonType(ui->btn_license_close, KiranPushButton::BUTTON_Default);
    KiranPushButton::setButtonType(ui->btn_license_export, KiranPushButton::BUTTON_Normal);

    setWindowModality(Qt::ApplicationModal);

    connect(ui->btn_license_close, &QPushButton::clicked, this, &LicenseAgreement::close);
    connect(ui->btn_license_export, &QPushButton::clicked, this, &LicenseAgreement::exportLicense);
}

LicenseAgreement::~LicenseAgreement()
{
    delete ui;
}

QString LicenseAgreement::getEulaText()
{
    QString text = ui->text_license->toPlainText();
    return text;
}

void LicenseAgreement::exportLicense()
{
    QString eulaText = ui->text_license->toPlainText();
    QString currentHomePath;
    if (m_licenseType == EULA_LICENSE)
        currentHomePath = "/" + QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/EULA.pdf";
    else if (m_licenseType == VERSION_LICENSE)
        currentHomePath = "/" + QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/Version-License.pdf";
    else
        currentHomePath = "/" + QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/Privacy-Policy.pdf";

    QString fileName = QFileDialog::getSaveFileName(this,
                                                    tr("Save"),
                                                    currentHomePath,
                                                    tr("PDF(*.pdf)"));
    if (fileName.isNull())
    {
        return;
    }
    QFile file(fileName);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        KiranMessageBox::KiranStandardButton button = KiranMessageBox::message(NULL, QString(tr("Export License")),
                                                                               QString(tr("Export License failed!")),
                                                                               KiranMessageBox::Ok);
        if (button == KiranMessageBox::Ok)
        {
            return;
        }
    }
    else
    {
        // 将EULA文字转化为PDF
        QPrinter printer(QPrinter::PrinterResolution);
        printer.setOutputFormat(QPrinter::PdfFormat);
        printer.setPageSize(QPageSize(QPageSize::A4));
        printer.setOutputFileName(fileName);

        QTextDocument doc;
        doc.setPlainText(eulaText); /* 可替换为文档内容 */
        QRect pageRect = printer.pageLayout().paintRectPixels(printer.resolution());
        doc.setPageSize(pageRect.size());
        doc.print(&printer);
        file.close();
    }
}

QString LicenseAgreement::getLocaleLang()
{
    QLocale locale = QLocale::system();
    QString lang;
    if (locale.language() == QLocale::Chinese)
        lang = "zh_CN";
    else if (locale.language() == QLocale::English)
        lang = "en_US";
    else
        return QString("");
    return lang;
}

void LicenseAgreement::setEULA()
{
    m_licenseType = EULA_LICENSE;
    ui->text_license->clear();

    setTitle(LicenseAgreement::tr("User End License Agreement"));

    QString lang = getLocaleLang();
    QString licenseFile;

    if (!lang.isEmpty())
        licenseFile = QString("%1/EULA-%2").arg(EULAFILE).arg(lang);
    else
        licenseFile = QString("%1/EULA-en_US").arg(EULAFILE);

    KLOG_INFO() << licenseFile;
    QFile file(licenseFile);
    QTextStream textStream;

    if (file.exists())
    {
        if (!file.open(QFile::ReadOnly | QFile::Text))
        {
            KLOG_INFO() << "Can't open " << licenseFile;
            ui->text_license->setText(tr("None"));
            file.close();
            return;
        }
        textStream.setDevice(&file);
    }
    else
    {
        KLOG_INFO() << licenseFile << " is not exists ";
        file.setFileName(QString("%1/EULA").arg(EULAFILE));
        if (!file.open(QFile::ReadOnly | QFile::Text))
        {
            KLOG_INFO() << "Can't open " << file.fileName();
            ui->text_license->setText(tr("None"));
            file.close();
            return;
        }
        textStream.setDevice(&file);
    }
    textStream.setCodec("UTF-8");
    ui->text_license->setText(textStream.readAll());
    file.close();
}

void LicenseAgreement::setVersionLicnese()
{
    m_licenseType = VERSION_LICENSE;
    ui->text_license->clear();

    setTitle(LicenseAgreement::tr("Version License"));

    QFile file(LICENSEFILE);
    QTextStream textStream;

    if (file.exists())
    {
        if (!file.open(QFile::ReadOnly | QFile::Text))
        {
            KLOG_INFO() << "Can't open " << LICENSEFILE;
            goto LOAD_VERSION_LICENSE_FROM_RES;
        }
        textStream.setDevice(&file);
        textStream.setCodec("UTF-8");
        ui->text_license->setText(textStream.readAll());
        file.close();
        return;
    }
    else
    {
        KLOG_INFO() << LICENSEFILE << " is not exists ";
        goto LOAD_VERSION_LICENSE_FROM_RES;
    }

LOAD_VERSION_LICENSE_FROM_RES:
    KLOG_INFO() << "LOAD_VERSION_LICENSE_FROM_RES";
    QString lang = getLocaleLang();
    QString body;
    QString title;
    if (!lang.isEmpty())
    {
        body = QString(":/kcp-system/version-license/gpl-3.0-%1-body.txt").arg(lang);
        title = QString(":/kcp-system/version-license/gpl-3.0-%1-title.txt").arg(lang);
    }
    else
    {
        body = QString(":/kcp-system/version-license/gpl-3.0-en_US-body.txt");
        title = QString(":/kcp-system/version-license/gpl-3.0-en_US-title.txt");
    }

    QFile fileTitle(title);
    QFile fileBody(body);
    if (!fileTitle.exists() || !fileBody.exists())
    {
        KLOG_INFO() << "Version License don't exists";
        ui->text_license->setText(tr("None"));
        return;
    }

    if (!fileTitle.open(QIODevice::ReadOnly | QIODevice::Text) ||
        !fileBody.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        KLOG_INFO() << "can't open Version License";
        ui->text_license->setText(tr("None"));
        return;
    }
    QTextStream textStreamTitle(&fileTitle);
    QTextStream textStreamBody(&fileBody);
    textStreamTitle.setCodec("UTF-8");
    textStreamBody.setCodec("UTF-8");

    ui->text_license->setAlignment(Qt::AlignHCenter);
    while (!textStreamTitle.atEnd())
    {
        QString line = textStreamTitle.readLine();
        ui->text_license->append(line);
    }
    ui->text_license->setAlignment(Qt::AlignLeft);
    ui->text_license->append(textStreamBody.readAll());
    ui->text_license->moveCursor(QTextCursor::Start);
    fileBody.close();
    fileTitle.close();
}

void LicenseAgreement::setPrivacyPolicy()
{
    m_licenseType = PRIVACY_POLICY;
    ui->text_license->clear();
#ifdef DISABLE_KIRANWIDGETS
    setWindowTitle(LicenseAgreement::tr("Privacy Policy"));
#else
    setTitle(LicenseAgreement::tr("Privacy Policy"));
#endif
    QFile file(PRIVACYFILE);
    QTextStream textStream;

    if (!file.exists())
    {
        KLOG_INFO() << PRIVACYFILE << " is not exists ";
        ui->text_license->setText(tr("None"));
        return;
    }

    if (!file.open(QFile::ReadOnly | QFile::Text))
    {
        KLOG_INFO() << "Can't open " << PRIVACYFILE;
        ui->text_license->setText(tr("None"));
        return;
    }
    textStream.setDevice(&file);
    textStream.setCodec("UTF-8");
    ui->text_license->setText(textStream.readAll());
    file.close();
}
