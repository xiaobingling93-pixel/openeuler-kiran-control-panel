/**
 * Copyright (c) 2020 ~ 2022 KylinSec Co., Ltd.
 * kiran-control-panel is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 *
 * Author:     liuxinhao <liuxinhao@kylinsec.com.cn>
 */

#ifndef KIRAN_POWER_MANAGER_SRC_PAGES_GENERAL_SETTINGS_GENERAL_SETTINGS_PAGE_H_
#define KIRAN_POWER_MANAGER_SRC_PAGES_GENERAL_SETTINGS_GENERAL_SETTINGS_PAGE_H_

#include <QTimer>
#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui
{
class GeneralSettingsPage;
}
QT_END_NAMESPACE

class PowerInterface;
class KiranSwitchButton;
class QGSettings;
class KWinColorCorrect;
class GeneralSettingsPage : public QWidget
{
    Q_OBJECT

public:
    explicit GeneralSettingsPage(QWidget *parent = nullptr);
    ~GeneralSettingsPage() override;
    QSize sizeHint() const override;

private:
    void init();
    void initSessionSetting();
    void initScreenSaverSetting();
    void initUI();
    void initConnection();
    void load();

    void setUiBrightnessPercent(int percent);
    void updateIdleTimeLabel(int min);

private slots:
    void updateSliderIdleTime(int idleTime);
    void updateEventAction();
    void updateMonitorBrightness();
    void onSliderBrightnessValueChanged(int value);
    void onSliderIdleTimeChanged(int value);
    void updateIdleTime();
    void updateIdleLockEnable(bool enable);
    void updateDisplayIdleDimmedEnable(bool enable);
    void updateSuspendLockEnable(bool enable);
    void updateCurrentComputerMode(int idx);

private:
    Ui::GeneralSettingsPage *ui;
    PowerInterface* m_powerInterface;
    QTimer m_brightnessSettingTimer;
    QTimer m_idleTimeSettingTimer;
    int m_brightnessValue;
    QGSettings* m_sessionSettings = nullptr;
    QGSettings* m_screensaverSettings = nullptr;
    KiranSwitchButton* m_switchIdlelockScreen = nullptr;
    KiranSwitchButton* m_switchDisplayIdleDimmed = nullptr;
    KiranSwitchButton* m_switchSuspendLockScreen = nullptr;
};

#endif  //KIRAN_POWER_MANAGER_SRC_PAGES_GENERAL_SETTINGS_GENERAL_SETTINGS_PAGE_H_
