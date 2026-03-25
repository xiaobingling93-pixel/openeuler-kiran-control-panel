/**
 * Copyright (c) 2020 ~ 2021 KylinSec Co., Ltd.
 * kiran-cpanel-system is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan
 * PSL v2. You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY
 * KIND, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO
 * NON-INFRINGEMENT, MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE. See the
 * Mulan PSL v2 for more details.
 *
 * Author:     yuanxing <yuanxing@kylinsec.com.cn>
 */

#include "system-information.h"
#include "dbus-wrapper/system-info-dbus.h"
#include "dbus_license_dbus.h"
#include "license-agreement.h"
#include "logging-category.h"
#include "ui_system-information.h"

#include <kiran-message-box.h>
#include <kiran-push-button.h>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDateTime>
#include <QDesktopWidget>
#include <QFont>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QPainter>
#include <QProcess>

#define HOST_NAME "host_name"
#define ARCH "arch"
#define KERNEL_VERSION "kernel_version"
#define KERNEL_NAME "kernal_name"
#define KERNEL_RELEASE "kernel_release"
#define PRODUCT_RELEASE "product_release"

SystemInformation::SystemInformation(QWidget* parent)
    : QWidget(parent), ui(new Ui::SystemInformation), hostNameWidget(nullptr), m_licenseAgreement(nullptr)
{
    ui->setupUi(this);

    m_licenseAgreement = new LicenseAgreement(this);
    init();
}

SystemInformation::~SystemInformation()
{
    delete ui;
    if (hostNameWidget != nullptr)
    {
        delete hostNameWidget;
    }
    if (m_licenseAgreement != nullptr)
    {
        delete m_licenseAgreement;
        m_licenseAgreement = nullptr;
    }
}

void SystemInformation::init()
{
    setMinimumHeight(400);
    initUI();
    // clang-format off
    connect(ui->btn_EULA, &QPushButton::clicked, [this]
    {
        m_licenseAgreement->setEULA();
        m_licenseAgreement->show();
    });
    connect(ui->btn_version_license, &QPushButton::clicked, [this]
    {
        m_licenseAgreement->setVersionLicnese();
        m_licenseAgreement->show();
    });
    connect(ui->btn_privacy_policy, &QPushButton::clicked, [this]
    {
        m_licenseAgreement->setPrivacyPolicy();
        m_licenseAgreement->show();
    });

    // clang-format on
    connect(ui->btn_change_name, &QPushButton::clicked, this, &SystemInformation::handleChangeHostName);

    KiranPushButton::setButtonType(ui->btn_change_name, KiranPushButton::BUTTON_Default);
    KiranPushButton::setButtonType(ui->btn_EULA, KiranPushButton::BUTTON_Default);
    KiranPushButton::setButtonType(ui->btn_version_license, KiranPushButton::BUTTON_Default);
    KiranPushButton::setButtonType(ui->btn_license_show, KiranPushButton::BUTTON_Default);
    KiranPushButton::setButtonType(ui->btn_privacy_policy, KiranPushButton::BUTTON_Default);
}

bool SystemInformation::initUI()
{
    updateSystemInformation();

    QString licenseDesc;
    if (!checkLicensEnable() || !getLicenseDesc(licenseDesc))
    {
        ui->widget_license->setVisible(false);
    }
    else
    {
        ui->lab_license_status->setText(licenseDesc);
        connect(ui->btn_license_show, &QPushButton::clicked, this, &SystemInformation::handleShowLicenseDialog);
    }

    QList<QLabel*> labels = {ui->lab_name_info, ui->lab_core_version_info, ui->lab_system_arch_info, ui->lab_system_version_info, ui->lab_license_status};
    for (QLabel* label : labels)
    {
        label->setStyleSheet("color:#919191;font-family: \"Noto Sans CJK SC regular\";");
    }

    QList<KiranFrame*> kiranFrames = findChildren<KiranFrame*>();
    for (int i = 0; i < kiranFrames.count(); i++)
    {
        KiranFrame* frame = kiranFrames.at(i);
        frame->setRadius(6);
        frame->setDrawBroder(false);
    }

    // 如果协议文件不存在，则隐藏协议相关控件
    if (m_licenseAgreement->getEULAFile().isEmpty())
    {
        ui->widget_EULA->hide();
    }

    // #126107 商业版本不能以开源许可证声明，不再显示本版协议
    ui->widget_version_license->hide();

#ifdef SYSTEM_PRIVACY_POLICY
    if (m_licenseAgreement->getPrivacyFile().isEmpty())
    {
        ui->widget_privacy_policy->hide();
    }
#else
    // #35818 在系统中不再单独提供隐私协议
    ui->widget_privacy_policy->hide();
#endif

    return true;
}

bool SystemInformation::hasUnsavedOptions()
{
    if (hostNameWidget != nullptr && hostNameWidget->getLineEditStatus())
    {
        return true;
    }
    else
        return false;
}

void SystemInformation::updateSystemInformation()
{
    QString systemInfoJson;
    bool bRes = SystemInfoDBus::getSystemInfo(SYSTEMINFO_TYPE_SOFTWARE, systemInfoJson);
    if (!bRes)
    {
        ui->lab_name_info->setText(tr("Unknow"));
        ui->lab_core_version_info->setText(tr("Unknow"));
        ui->lab_system_arch_info->setText(tr("Unknow"));
        ui->lab_system_version_info->setText(tr("Unknow"));
        ui->btn_change_name->hide();
    }
    else
    {
        QString hostname, arch, systemVersion, kernelVersion;

        parseSoftwareInfoJson(systemInfoJson,
                              hostname,
                              arch,
                              systemVersion,
                              kernelVersion);

        ui->lab_name_info->setText(hostname);
        ui->lab_system_arch_info->setText(arch);
        ui->lab_system_version_info->setText(systemVersion);
        ui->lab_core_version_info->setText(kernelVersion);

        KLOG_DEBUG(qLcSystem) << "update system information:"
                              << "\n"
                              << "\t hostname: " << hostname << "\n"
                              << "\t arch: " << arch << "\n"
                              << "\t system version: " << systemVersion << "\n"
                              << "\t kernel version: " << kernelVersion;
    }
}

void SystemInformation::parseSoftwareInfoJson(QString jsonString,
                                              QString& hostName,
                                              QString& arch,
                                              QString& systemVersion,
                                              QString& kernelVersion)
{
    QJsonParseError jsonError;
    QJsonDocument jsonDocument = QJsonDocument::fromJson(jsonString.toLocal8Bit().data(), &jsonError);
    if (jsonDocument.isNull() || jsonError.error != QJsonParseError::NoError || !jsonDocument.isObject())
    {
        KLOG_ERROR() << " please check the activation information string " << jsonString.toLocal8Bit().data();
        return;
    }
    QJsonObject rootObject = jsonDocument.object();
    if (rootObject.contains("host_name") && rootObject["host_name"].isString())
    {
        hostName = rootObject["host_name"].toString();
    }
    if (rootObject.contains("arch") && rootObject["arch"].isString())
    {
        arch = rootObject["arch"].toString();
    }
    if (rootObject.contains("product_release") && rootObject["product_release"].isString())
    {
        systemVersion = rootObject["product_release"].toString();
    }
    if (rootObject.contains("kernal_name") && rootObject["kernal_name"].isString() &&
        rootObject.contains("kernel_release") && rootObject["kernel_release"].isString())
    {
        kernelVersion = rootObject["kernal_name"].toString() + " " + rootObject["kernel_release"].toString();
    }
}

bool SystemInformation::checkLicensEnable()
{
    QDBusConnection dbusConn = QDBusConnection::systemBus();
    return dbusConn.interface()->isServiceRegistered("com.kylinsec.Kiran.LicenseManager");
}

bool SystemInformation::getLicenseDesc(QString& licenseStatus)
{
    DBusLicenseObject dBusLicenseObject("com.kylinsec.Kiran.LicenseManager",
                                        "/com/kylinsec/Kiran/LicenseObject/KylinSecOS",
                                        QDBusConnection::systemBus());
    auto reply = dBusLicenseObject.GetLicense();
    reply.waitForFinished();
    if (reply.isError())
    {
        KLOG_ERROR() << "KylinSecOS GetLicense failed:" << reply.error();
        return false;
    }

    QString licenseJson = reply.value();
    QJsonDocument jsonDocument = QJsonDocument::fromJson(licenseJson.toUtf8());
    QJsonObject rootObj = jsonDocument.object();
    QStringList keys = rootObj.keys();

    QSet<QString> keySet = {"expired_time", "activation_status"};
    for (auto key : keySet)
    {
        if (!keys.contains(key))
        {
            KLOG_ERROR() << "KylinSecOS GetLicense missing key:" << key;
            return false;
        }
    }

    bool expired = false;
    QString statusDesc("");

    QVariant expiredTimeVar = rootObj["expired_time"].toVariant();
    qlonglong expiredTimeSinceEpoch = expiredTimeVar.toULongLong();

    QVariant activationStatusVar = rootObj["activation_status"].toVariant();
    qulonglong activationStatus = activationStatusVar.toULongLong();

    QDateTime expiredTime = QDateTime::fromSecsSinceEpoch(expiredTimeSinceEpoch);
    if (activationStatus == 0)  // 未激活
    {
        statusDesc = tr("UnActivated");
        expired = true;
    }
    else
    {
        QDateTime currentDateTime = QDateTime::currentDateTime();

        if (currentDateTime > expiredTime)  // 激活码已过期
        {
            statusDesc = tr("Activation code has expired");
            expired = true;
        }
        else if (expiredTime.date().year() >= 2100)  // 永久激活
        {
            statusDesc = tr("Permanently activated");
            expired = false;
        }
        else  // 已激活
        {
            statusDesc = tr("Activated");
            expired = false;
        }
    }

    licenseStatus = QString("<font color=%1>%2</font>").arg(expired ? "#ff3838" : "#5ab940").arg(statusDesc);
    return true;
}

/**
 * @brief SystemInformation::onBtnchangeHostName: 当点击更改用户名后的槽函数
 */
void SystemInformation::handleChangeHostName()
{
    if (hostNameWidget == nullptr)
    {
        hostNameWidget = new ChangeHostNameWidget(this);
    }
    connect(hostNameWidget, &ChangeHostNameWidget::hostnameChanged,
            ui->lab_name_info, &QLabel::setText, Qt::UniqueConnection);
    hostNameWidget->setAttribute(Qt::WA_QuitOnClose, false);
    hostNameWidget->installEventFilter(this);
    hostNameWidget->raise();
    hostNameWidget->show();
}

void SystemInformation::handleShowLicenseDialog()
{
    if (!QProcess::startDetached("/usr/bin/ksl-os-gui", QStringList()))
    {
        KiranMessageBox::message(this, tr("Error"), tr("Failed to open the license activator"), KiranMessageBox::Ok);
    }
}

/**
 * @brief 事件监听，当收到激活向导窗口或者授权信息窗口的关闭事件时，释放窗口内存
 * @param  obj  事件对象
 * @param  obj  事件
 * @return 是否过滤
 */
bool SystemInformation::eventFilter(QObject* obj, QEvent* event)
{
    if (obj == hostNameWidget && event->type() == QEvent::Close)
    {
        hostNameWidget->deleteLater();
        hostNameWidget = nullptr;
    }
    return false;
}

QSize SystemInformation::sizeHint() const
{
    return {500, 657};
}
