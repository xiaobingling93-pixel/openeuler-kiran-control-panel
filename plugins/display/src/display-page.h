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

#include <QComboBox>
#include <QWidget>
#include "display-config.h"

namespace Ui
{
class DisplayPage;
}

class DisplayBackEndProxy;
class QButtonGroup;
class DisplayPage : public QWidget
{
    Q_OBJECT

public:
    explicit DisplayPage(QWidget *parent = 0);
    ~DisplayPage();

    QSize sizeHint() const override;

private slots:
    void handleApplyButtonClicked();
    void handleCancelButtonClicked();

    void switchDisplayConfigMode(int index, const bool &checked);
    void onScreenItemChecked(QString monitorPath);

    void handleResolvingCurrentTextChanged(const QString &text);
    void handleExtraResolvingCurrentTextChanged(const QString &text);

    void handleRefreshRateChanged(int index);
    void handleExtraRefreshRateChanged(int index);
    void handleWindowScalingFactor(int index);

    void handleDbusPropertiesChanged();
    void handleEnabledButtonToggled(bool checked);
    void handlePrimaryDisplayButtonToggled(bool checked);

private:
    QList<DisplayModesStu> intersectionMonitorModes();
    QMap<int, modeInfoPair> getResolutionFromModes(const QList<DisplayModesStu> &modes);
    DisplayModesStu curIntersectionMonitorMode();

private:
    void init();
    void initConnect();
    void refreshWidget();

    void confirmSaveMessageBox();

    void showExtraModeData(const QString &monitorPath);

    void initComboBoxResolution(QComboBox *comboBox, const QMap<int, modeInfoPair> &map);
    void initComboBoxRefreshRate(QComboBox *comboBox, const QList<double> &refreshRateList);
    void initExtraComboBoxResolution(QComboBox *comboBox, const QMap<int, modeInfoPair> &map);
    void initExtraComboBoxRefreshRate(QComboBox *comboBox, const QList<double> &refreshRateList);

    void selectResolutionComboboxItem(QComboBox *comboBox, const int &w, const int &h);
    void selectRefreshRateComboboxItem(QComboBox *comboBox, const double &r);

    bool extraPrimaryBtnStatus(const bool &onlyEnableScreen, const bool &enable);

private:
    Ui::DisplayPage *ui;

    DisplayConfig *m_displayConfig;

    MonitorConfigDataPtr m_currentMonitorData;
    DisplayConfigDataPtr m_displayConfigData;

    QString m_curMonitorPath;
    QButtonGroup *m_btnGroup;
    QStringList m_listMonitors;  //用于处理虚拟机中，没有勾选显示器，xrandr返回两个显示器，但是部分显示器的dbus却无法调用的情况。
};
