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
#ifndef LICENSEAGREEMENT_H
#define LICENSEAGREEMENT_H

#include <kiranwidgets-qt5/kiran-titlebar-window.h>
#include <QWidget>

namespace Ui
{
class LicenseAgreement;
}

class LicenseAgreement : public KiranTitlebarWindow
{
    Q_OBJECT

public:
    explicit LicenseAgreement(QWidget *parent, Qt::WindowFlags windowFlags = Qt::Window);
    ~LicenseAgreement();
    QString getEulaText();

    // 设置协议内容
    void setEULA();
    void setVersionLicnese();
    void setPrivacyPolicy();

    // 获取协议文件，不存在返回空字符串
    QString getEULAFile();
    QString getLicenseFile();
    QString getPrivacyFile();

public slots:
    void exportLicense();

private:
    QString getLocaleLang();
    QString getReleaseBasePath();

private:
    Ui::LicenseAgreement *ui;
    int m_licenseType;
};

#endif  // LICENSEAGREEMENT_H
