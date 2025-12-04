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


#ifndef __DEFAULTAPP_H__
#define __DEFAULTAPP_H__

#include <QWidget>
#include <QComboBox>
#include "kiran-setting-container/kiran-setting-item.h"
#include "libqtxdg.h"
#include "utils.h"

namespace Ui
{
class DefaultApp;
}

class AppManager;
class DefaultApp : public QWidget
{
    Q_OBJECT

public:
    explicit DefaultApp(QWidget* parent = nullptr);
    ~DefaultApp();
    void initUI();
    void initConfig();
    void initConnect();

    void fillDefaultAppComboBox(EnumMimeType enumMimeType);

private slots:
    void handleCurrentTextChanged(const QString &text);

private:
    Ui::DefaultApp* ui;
    AppManager* m_appManager;
    QMap<EnumMimeType, QMap<QString,XdgDesktopFilePtr>> m_applications;
    QMap<EnumMimeType, QComboBox*> m_comboBox;
};


#endif // __DEFAULTAPP_H__