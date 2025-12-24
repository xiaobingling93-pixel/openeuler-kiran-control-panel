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

#ifndef SYSTEMINFORMATION_H
#define SYSTEMINFORMATION_H

#include <QPaintEvent>
#include <QWidget>
#include "change-host-name-widget.h"
namespace Ui
{
class SystemInformation;
}

class LicenseAgreement;
class SystemInformation : public QWidget
{
    Q_OBJECT

public:
    explicit SystemInformation(QWidget *parent = 0);
    ~SystemInformation();
    void init();
    bool initUI();
    bool hasUnsavedOptions();

private:
    void updateSystemInformation();
    void parseSoftwareInfoJson(QString jsonString,
                               QString &hostName,
                               QString &arch,
                               QString &systemVersion,
                               QString &kernelVersion);

    bool checkLicensEnable();
    bool getLicenseDesc(QString &licenseStatus);

private slots:
    void handleChangeHostName(void);
    void handleShowLicenseDialog();

protected:
    bool eventFilter(QObject *obj, QEvent *event);
    virtual QSize sizeHint() const;

private:
    Ui::SystemInformation *ui;

    ChangeHostNameWidget *hostNameWidget;
    LicenseAgreement *m_licenseAgreement;
};

#endif  // SYSTEMINFORMATION_H
