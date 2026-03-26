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
 * Author:     yangxiaoqing <yangxiaoqing@kylinsec.com.cn>
 */

#pragma once

#include <QFrame>

namespace Ui
{
class DevicePanel;
}

class DevicePanel : public QFrame
{
    Q_OBJECT

public:
    explicit DevicePanel(QWidget *parent = nullptr);
    ~DevicePanel();

    void changeItemDisabled(const bool &disabled);

protected:
    void paintEvent(QPaintEvent *event) override;

Q_SIGNALS:
    void screenItemChecked(QString monitorPath);

private slots:
    void on_pushButton_left_clicked();
    void on_pushButton_horizontal_clicked(bool checked);
    void on_pushButton_vertical_clicked(bool checked);
    void on_pushButton_right_clicked();
    void on_pushButton_identifying_clicked();

private:
    void updateTransformButtonsVisible(QString monitorPath);

    Ui::DevicePanel *ui;
};
