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
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QIcon>
#include <QLocale>
#include <QMessageBox>
#include <QStandardPaths>
#include <QTextDocument>
#include <QtPrintSupport/QPrinter>

#define KYLIN_RELEASE_PATH "/usr/share/kylin-release"
#define KYLINSEC_RELEASE_PATH "/usr/share/kylinsec-release"
#define LICENSEFILE_KYLIN "/usr/share/doc/kylin-release/LICENSE"
#define LICENSEFILE_KYLINSEC "/usr/share/doc/kylinsec-release/LICENSE"

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

QString LicenseAgreement::getReleaseBasePath()
{
    QDir dirKylinsec(KYLINSEC_RELEASE_PATH);
    if (dirKylinsec.exists())
    {
        return KYLINSEC_RELEASE_PATH;
    }
    QDir dirKylin(KYLIN_RELEASE_PATH);
    if (dirKylin.exists())
    {
        return KYLIN_RELEASE_PATH;
    }
    // 如果都不存在，默认返回 kylinsec-release
    return KYLINSEC_RELEASE_PATH;
}

QString LicenseAgreement::getLicenseFile()
{
    QString licenseFile = "";
    QFile fileKylinsec(LICENSEFILE_KYLINSEC);
    if (fileKylinsec.exists())
    {
        licenseFile = LICENSEFILE_KYLINSEC;
    }
    else
    {
        QFile fileKylin(LICENSEFILE_KYLIN);
        if (fileKylin.exists())
        {
            licenseFile = LICENSEFILE_KYLIN;
        }
    }
    // 如果都不存在，返回空字符串
    KLOG_DEBUG() << "License file: " << licenseFile;
    return licenseFile;
}

QString LicenseAgreement::getPrivacyFile()
{
    QString basePath = getReleaseBasePath();
    if (basePath.isEmpty())
    {
        KLOG_WARNING() << "Release path not found";
        return QString();
    }

    QString privacyFile = "";
    QString lang = getLocaleLang();
    if (!lang.isEmpty())
        privacyFile = QString("%1/privacy_policy-%2").arg(basePath).arg(lang);
    else
        privacyFile = QString("%1/privacy_policy-en_US").arg(basePath);

    if (!QFile::exists(privacyFile))
    {
        privacyFile = QString("%1/privacy_policy").arg(basePath);
        if (!QFile::exists(privacyFile))
        {
            KLOG_WARNING() << "privacy file not found";
            return QString();
        }
    }

    KLOG_DEBUG() << "Privacy file: " << privacyFile;
    return privacyFile;
}

QString LicenseAgreement::getEULAFile()
{
    QString eulaFile = "";
    auto eulaDir = getReleaseBasePath();
    if (eulaDir.isEmpty())
    {
        KLOG_WARNING() << "Release path not found";
        return QString();
    }

    QString lang = getLocaleLang();
    if (!lang.isEmpty())
        eulaFile = QString("%1/EULA-%2").arg(eulaDir).arg(lang);
    else
        eulaFile = QString("%1/EULA-en_US").arg(eulaDir);

    if (!QFile::exists(eulaFile))
    {
        eulaFile = QString("%1/EULA").arg(eulaDir);
        if (!QFile::exists(eulaFile))
        {
            KLOG_WARNING() << "EULA file not found";
            return QString();
        }
    }

    KLOG_DEBUG() << "EULA file: " << eulaFile;
    return eulaFile;
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

    auto eulaFile = getEULAFile();
    if (eulaFile.isEmpty())
    {
        KLOG_WARNING() << "EULA file not found";
        ui->text_license->setText(tr("None"));
        return;
    }

    QFile file(eulaFile);
    QTextStream textStream;
    if (!file.open(QFile::ReadOnly | QFile::Text))
    {
        KLOG_WARNING() << "Can't open " << eulaFile;
        ui->text_license->setText(tr("None"));
        return;
    }
    textStream.setDevice(&file);
    textStream.setCodec("UTF-8");
    ui->text_license->setText(textStream.readAll());
    file.close();
}

void LicenseAgreement::setVersionLicnese()
{
    m_licenseType = VERSION_LICENSE;
    ui->text_license->clear();

    setTitle(LicenseAgreement::tr("Version License"));

    auto licenseFile = getLicenseFile();
    if (!licenseFile.isEmpty())
    {
        QFile file(licenseFile);
        if (file.open(QFile::ReadOnly | QFile::Text))
        {
            QTextStream textStream;
            textStream.setDevice(&file);
            textStream.setCodec("UTF-8");
            ui->text_license->setText(textStream.readAll());
            file.close();
            return;
        }
        else
        {
            KLOG_WARNING() << "Can't open " << licenseFile;
        }
    }
    else
    {
        KLOG_WARNING() << "License file not found";
    }

    // 从资源文件加载版本许可证
    KLOG_DEBUG() << "Load version license from resources";
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
        KLOG_WARNING() << "Version License don't exists";
        ui->text_license->setText(tr("None"));
        return;
    }

    if (!fileTitle.open(QIODevice::ReadOnly | QIODevice::Text) ||
        !fileBody.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        KLOG_WARNING() << "can't open Version License";
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

    auto privateFile = getPrivacyFile();
    if (privateFile.isEmpty())
    {
        KLOG_WARNING() << "Privacy file not found";
        ui->text_license->setText(tr("None"));
        return;
    }

    QFile file(privateFile);
    QTextStream textStream;

    if (!file.open(QFile::ReadOnly | QFile::Text))
    {
        KLOG_WARNING() << "Can't open " << privateFile;
        ui->text_license->setText(tr("None"));
        return;
    }
    textStream.setDevice(&file);
    textStream.setCodec("UTF-8");
    ui->text_license->setText(textStream.readAll());
    file.close();
}
