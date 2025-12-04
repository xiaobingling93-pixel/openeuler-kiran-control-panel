/**
 * Copyright (c) 2023 ~ 2024 KylinSec Co., Ltd.
 * ks-sc is licensed under Mulan PSL v2.
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


#include "defaultapp.h"
#include <qt5-log-i.h>
#include <QBoxLayout>
#include <QListWidget>
#include "app-manager.h"
#include "kiran-setting-container/kiran-setting-container.h"
#include "ui_defaultapp.h"


DefaultApp::DefaultApp(QWidget* parent)
    : QWidget(parent), ui(new Ui::DefaultApp), m_appManager(new AppManager)
{
    ui->setupUi(this);
    initConfig();
    initUI();
    initConnect();
}

void DefaultApp::initConnect()
{
    for (auto iter = m_comboBox.begin(); iter != m_comboBox.end(); iter++)
    {
        connect(iter.value(), QOverload<const QString &>::of(&QComboBox::currentTextChanged), this,
            &DefaultApp::handleCurrentTextChanged);
    }
}

void DefaultApp::handleCurrentTextChanged(const QString &text)
{
    QComboBox* qComboBox = qobject_cast<QComboBox*>(sender());
    EnumMimeType enumMimeType = m_comboBox.key(qComboBox);

    if (m_applications.contains(enumMimeType))
    {
        QMap<QString,XdgDesktopFilePtr> applicationInfo = m_applications[enumMimeType];
        if (applicationInfo.contains(text))
        {
            m_appManager->slotTextChanged(enumMimeType, applicationInfo[text].data());
        }
    }

}


void DefaultApp::fillDefaultAppComboBox(EnumMimeType enumMimeType)
{
    QVector<XdgDesktopFilePtr> m_appInfos = m_appManager->getApplications(enumMimeType);
    QComboBox* qcomboBox = m_comboBox.value(enumMimeType);
    QMap<QString,XdgDesktopFilePtr> applicationInfo;
    for (int index = 0; index < m_appInfos.size(); index++)
    {
        // 获取desktop的中文名和图标
        XdgDesktopFile* deskInfo = m_appInfos[index].data();
        qcomboBox->addItem(deskInfo->icon(), deskInfo->name());

        applicationInfo[deskInfo->name()] = m_appInfos[index];
        m_applications.insert(enumMimeType, applicationInfo);
    }
}

void DefaultApp::initConfig()
{
    m_comboBox.insert(DA_TYPE_WEB_BROWSER, ui->comboBox_web);
    m_comboBox.insert(DA_TYPE_EMAIL, ui->comboBox_email);
    m_comboBox.insert(DA_TYPE_TEXT, ui->comboBox_text);
    m_comboBox.insert(DA_TYPE_MEDIA, ui->comboBox_music);
    m_comboBox.insert(DA_TYPE_VIDEO, ui->comboBox_video);
    m_comboBox.insert(DA_TYPE_IMAGE, ui->comboBox_image);
}


void DefaultApp::initUI()
{
    for (EnumMimeType enumMimeType = DA_TYPE_WEB_BROWSER;
         enumMimeType < DA_TYPE_LAST;
         enumMimeType = static_cast<EnumMimeType>(enumMimeType + 1))
    {
        fillDefaultAppComboBox(enumMimeType);
    }
}



DefaultApp::~DefaultApp()
{
    delete m_appManager;
    delete ui;
}
