/**
 * Copyright (c) 2022 ~ 2023 KylinSec Co., Ltd.
 * kiran-control-panel is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 *
 * Author:     yinhongchang <yinhongchang@kylinsec.com.cn>
 */
#include "autostart-page.h"
#include <kiran-message-box.h>
#include <kiran-push-button.h>
#include <qt5-log-i.h>
#include <QBoxLayout>
#include <QDir>
#include <QFileDialog>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QStandardPaths>
#include <QWidget>
#include "autostart-app.h"
#include "kiran-setting-container/kiran-setting-container.h"
#include "kiran-setting-container/kiran-setting-item.h"

#define DESKTOP_DIR "/usr/share/applications"

AutostartPage::AutostartPage(QWidget *parent)
    : QWidget(parent)
{
    initAutoStartApps();
    initAutoStartAppUI();
}

AutostartPage::~AutostartPage()
{
}

void AutostartPage::initAutoStartApps()
{
    if (!initAutoStartAppsConfig())
    {
        return;
    }

    QDir autoStartAppDir(m_localConfigDir);

    QStringList filters;
    filters << QString("*.desktop");

    autoStartAppDir.setFilter(QDir::Files | QDir::NoSymLinks);
    autoStartAppDir.setNameFilters(filters);
    QFileInfoList autoStartAppList = autoStartAppDir.entryInfoList();

    m_autoStartApps.clear();
    for (int i = 0; i < autoStartAppList.size(); i++)
    {
        QFileInfo fileInfo = autoStartAppList.at(i);

        QString autoStartAppFilePath = m_localConfigDir + fileInfo.fileName();
        AutostartApp autoStartApp(autoStartAppFilePath);
        if (autoStartApp.getNoDisplay() || !autoStartApp.getShown())
        {
            continue;
        }
        m_autoStartApps.insert(fileInfo.fileName(), autoStartApp);
    }
}

bool AutostartPage::initAutoStartAppsConfig()
{
    m_localConfigDir = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) + QLatin1String("/autostart/");

    QDir configDir(m_localConfigDir);
    if (!configDir.exists() && !configDir.mkpath(m_localConfigDir))
    {
        KLOG_ERROR() << "create directory " << m_localConfigDir << " failed";
        return false;
    }

    return true;
}

void AutostartPage::initAutoStartAppUI()
{
    auto mainLayout = new QBoxLayout(QBoxLayout::TopToBottom, this);
    mainLayout->setContentsMargins(24, 14, 14, 14);
    mainLayout->setSpacing(0);

    auto label = new QLabel(tr("Boot Setup"), this);
    mainLayout->addWidget(label);

    auto autoStartWidget = new QWidget(this);
    mainLayout->addWidget(autoStartWidget);

    auto autoStartLayout = new QBoxLayout(QBoxLayout::TopToBottom, autoStartWidget);
    autoStartLayout->setSpacing(8);
    autoStartLayout->setContentsMargins(0, 0, 10, 0);

    autoStartLayout->addSpacerItem(new QSpacerItem(10, 10, QSizePolicy::Minimum, QSizePolicy::Fixed));

    autoStartAppContainer = new SettingsContainer(this);
    autoStartLayout->addWidget(autoStartAppContainer);

    autoStartLayout->addStretch();

    for (auto iter = m_autoStartApps.begin(); iter != m_autoStartApps.end(); iter++)
    {
        auto autoStartAppItem = createAutoStartAppItem(iter.key(), iter.value());
        autoStartAppContainer->addItem(autoStartAppItem);
    }

    initAddBtn();
}

void AutostartPage::initAddBtn()
{
    m_autoStartButton = new QPushButton(this);

    KiranPushButton::setButtonType(m_autoStartButton, KiranPushButton::BUTTON_Default);
    m_autoStartButton->setIcon(QIcon(":/kcp-application/images/addition.svg"));
    autoStartAppContainer->addItem(m_autoStartButton);
    connect(m_autoStartButton, &QPushButton::clicked, this, &AutostartPage::selectDesktopForAutoStart);
}

void AutostartPage::changeAutoStartAppStatus(const QString &desktopName, bool checked)
{
    if (!m_autoStartApps.contains(desktopName))
    {
        KLOG_WARNING() << m_localConfigDir << " don't have " << desktopName << ", setConfigDesktop() filed";
        return;
    }

    auto autoStartApp = m_autoStartApps.value(desktopName);
    autoStartApp.setAutoStartAppStatus(checked);
    return;
}

void AutostartPage::deleteAutoStartApp(KiranSettingItem *autoStartAppItem, const QString &desktopName)
{
    if (autoStartAppItem == nullptr)
    {
        return;
    }

    autoStartAppContainer->removeItem(autoStartAppItem);

    auto autoStartApp = m_autoStartApps.value(desktopName);
    QString autoStartAppFilePath = autoStartApp.getFilePath();

    m_autoStartApps.erase(m_autoStartApps.find(desktopName));

    if (autoStartAppFilePath.isEmpty() || !QFileInfo::exists(autoStartAppFilePath))
    {
        KLOG_WARNING() << autoStartAppFilePath << " isn't exist.";
        return;
    }

    if (!QFile::remove(autoStartAppFilePath))
    {
        KLOG_WARNING() << autoStartAppFilePath << "  delete failed.";
    }

    return;
}

KiranSettingItem *AutostartPage::createAutoStartAppItem(const QString &desktopName, AutostartApp &autoStartApp)
{
    auto autoStartAppItem = new KiranSettingItem(this);

    autoStartAppItem->setClickable(false);
    autoStartAppItem->setLeftButtonVisible(true, autoStartApp.getIcon());
    autoStartAppItem->setRightButtonVisible(true, QIcon::fromTheme("ksvg-trash"));
    autoStartAppItem->setSwitcherVisible(true);
    autoStartAppItem->setSwitcherChecked(!autoStartApp.getHidden());
    autoStartAppItem->setText(autoStartApp.getName());

    auto changeAutoStartAppStatusSlot = std::bind(&AutostartPage::changeAutoStartAppStatus, this, desktopName, std::placeholders::_2);
    connect(autoStartAppItem, &KiranSettingItem::switchButtonToggled, this, changeAutoStartAppStatusSlot);

    auto deleteAutoStartAppSlot = std::bind(&AutostartPage::deleteAutoStartApp, this, autoStartAppItem, desktopName);
    connect(autoStartAppItem, &KiranSettingItem::rightButtonClicked, this, deleteAutoStartAppSlot);

    return autoStartAppItem;
}

AutoStartAppFlags AutostartPage::addAutoStartApp(const QString &path, const QString &desktopName)
{
    if (m_autoStartApps.contains(desktopName))
    {
        KLOG_WARNING() << desktopName << " has existed in autoStartApps";
        return AUTO_START_APP_EXISTS;
    }

    QFileInfo fileInfo(path);
    QString mFileName = fileInfo.fileName();
    QString filePath = m_localConfigDir + mFileName;

    AutostartApp app(path);
    if (app.getNoDisplay() || !app.getShown())
    {
        KLOG_WARNING() << desktopName << " can't show in this Desktop environment";
        return AUTO_START_APP_NOSUPPORT;
    }

    if (!QFile::copy(path, filePath))
    {
        KLOG_WARNING() << "failed to copy app to autostart";
        return AUTO_START_APP_NOPERMIT;
    }

    AutostartApp autoStartApp(filePath);
    m_autoStartApps.insert(desktopName, autoStartApp);

    auto autoStartAppItem = createAutoStartAppItem(desktopName, autoStartApp);
    autoStartAppContainer->insertItem(autoStartAppContainer->getContainerLayoutSize() - 1, autoStartAppItem);

    return AUTO_START_APP_SUCCESS;
}

void AutostartPage::selectDesktopForAutoStart()
{
    QString filters = tr("Desktop files(*.desktop)");
    QFileDialog desktopFileDialog(this);
    desktopFileDialog.setDirectory(DESKTOP_DIR);
    desktopFileDialog.setModal(true);
    desktopFileDialog.setAcceptMode(QFileDialog::AcceptOpen);
    desktopFileDialog.setViewMode(QFileDialog::Detail);
    desktopFileDialog.setNameFilter(filters);
    desktopFileDialog.setFileMode(QFileDialog::ExistingFile);
    desktopFileDialog.setWindowTitle(tr("select autostart desktop"));
    desktopFileDialog.setLabelText(QFileDialog::Accept, tr("Select"));
    desktopFileDialog.setLabelText(QFileDialog::Reject, tr("Cancel"));
    if (desktopFileDialog.exec() != QDialog::Accepted)
        return;

    QString selectFile = desktopFileDialog.selectedFiles().first();
    QFileInfo fileInfo(selectFile);
    QString desktopName = fileInfo.fileName();

    // 错误标志
    AutoStartAppFlags flag = addAutoStartApp(selectFile, desktopName);

    switch (flag)
    {
    case AUTO_START_APP_EXISTS:
    {
        KiranMessageBox::message(this, tr("Error"), tr("Desktop has existed"), KiranMessageBox::Ok);
        break;
    }
    case AUTO_START_APP_NOPERMIT:
    {
        KiranMessageBox::message(this, tr("Error"), tr("Desktop cant permit to join"), KiranMessageBox::Ok);
        break;
    }
    case AUTO_START_APP_NOSUPPORT:
    {
        KiranMessageBox::message(this, tr("Error"), tr("Desktop dont support"), KiranMessageBox::Ok);
        break;
    }
    default:
        break;
    }

    return;
}