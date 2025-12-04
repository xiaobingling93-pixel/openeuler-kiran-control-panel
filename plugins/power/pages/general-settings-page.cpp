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

#include "general-settings-page.h"
#include <kiran-switch-button.h>
#include <qt5-log-i.h>
#include <QGSettings/QGSettings>
#include <QListView>
#include <QSignalBlocker>
#include "dbus/power.h"
#include "kiran-message-box.h"
#include "kiran-session-daemon/power-i.h"
#include "logging-category.h"
#include "ui_general-settings-page.h"

#define MAX_IDLE_TIME 120

#define MATE_SESSION_SCHEMA_ID "org.mate.session"
#define KIRAN_SESSION_SCHEMA_ID "com.kylinsec.kiran.session-manager"
#define KEY_IDLE_DELAY "idleDelay"
#define DEFAULT_IDLE_DELAY 5

#define SCHEMA_KIRAN_SCREENSAVER "com.kylinsec.kiran.screensaver"
#define KEY_IDLE_ACTIVATION_LOCK "idleActivationLock"

GeneralSettingsPage::GeneralSettingsPage(QWidget* parent)
    : QWidget(parent),
      ui(new Ui::GeneralSettingsPage),
      m_powerInterface(PowerInterface::getInstance())
{
    ui->setupUi(this);
    init();
}

GeneralSettingsPage::~GeneralSettingsPage()
{
    delete ui;
}

void GeneralSettingsPage::init()
{
    initSessionSetting();
    initScreenSaverSetting();
    initUI();
    initConnection();
    load();
}

void GeneralSettingsPage::initSessionSetting()
{
    if (QGSettings::isSchemaInstalled(KIRAN_SESSION_SCHEMA_ID))
    {
        m_sessionSettings = new QGSettings(KIRAN_SESSION_SCHEMA_ID, QByteArray(), this);
    }
    else if (QGSettings::isSchemaInstalled(MATE_SESSION_SCHEMA_ID))
    {
        m_sessionSettings = new QGSettings(MATE_SESSION_SCHEMA_ID, QByteArray(), this);
    }

    if (!m_sessionSettings)
    {
        KLOG_ERROR(qLcPower) << "can't find session gsettings";
        return;
    }

    connect(m_sessionSettings, &QGSettings::changed, this, [this](const QString& key)
            {
            if (key != KEY_IDLE_DELAY)
            {
                return;
            }

            int idleTime = m_sessionSettings->get(KEY_IDLE_DELAY).toInt();
            updateSliderIdleTime(idleTime); });
}

void GeneralSettingsPage::initScreenSaverSetting()
{
    if (!QGSettings::isSchemaInstalled(SCHEMA_KIRAN_SCREENSAVER))
    {
        KLOG_ERROR(qLcPower) << SCHEMA_KIRAN_SCREENSAVER << "is not install!";
        return;
    }

    m_screensaverSettings = new QGSettings(SCHEMA_KIRAN_SCREENSAVER, QByteArray(), this);

    connect(m_screensaverSettings, &QGSettings::changed, [this](const QString& key)
            {
            if(key != KEY_IDLE_ACTIVATION_LOCK)
            {
                return ;
            }

            bool value = m_screensaverSettings->get(KEY_IDLE_ACTIVATION_LOCK).toBool();
            m_switchIdlelockScreen->setChecked(value); });
}

void GeneralSettingsPage::initUI()
{
    /// 填充选项
    typedef QList<QPair<QString, PowerAction>> Actions;
    // clang-format off
    QList<QPair<QComboBox*,Actions>> actions = {
        {ui->combo_powerButton,{
            {tr("shutdown"), POWER_ACTION_COMPUTER_SHUTDOWN},
            {tr("hibernate"), POWER_ACTION_COMPUTER_HIBERNATE},
            {tr("suspend"), POWER_ACTION_COMPUTER_SUSPEND},
            {tr("display off"), POWER_ACTION_DISPLAY_OFF},
            {tr("do nothing"), POWER_ACTION_NOTHING}}},
        {ui->combo_suspendButton,{
            {tr("suspend"),POWER_ACTION_COMPUTER_SUSPEND},
            {tr("hibernate"),POWER_ACTION_COMPUTER_HIBERNATE},
            {tr("display off"),POWER_ACTION_DISPLAY_OFF},
            {tr("do nothing"),POWER_ACTION_NOTHING}}},
        {ui->combo_closingLid,{
            {tr("suspend"),POWER_ACTION_COMPUTER_SUSPEND},
            {tr("hibernate"),POWER_ACTION_COMPUTER_HIBERNATE},
            {tr("shutdown"),POWER_ACTION_COMPUTER_SHUTDOWN},
            {tr("do nothing"),POWER_ACTION_NOTHING}}}
    };
    // clang-format on

    /// 填充ComboBox
    for (auto& action : actions)
    {
        QComboBox* comboBox = action.first;
        for (const auto& actionIter : action.second)
        {
            comboBox->addItem(actionIter.first, actionIter.second);
        }
    }

    // 初始化计算机模式
    struct PowerProfileInfo
    {
        QString name;
        int index;
    };
    QList<PowerProfileInfo> profiles = {
        {tr("Energy-saving mode"), POWER_PROFILE_MODE_SAVER},
        {tr("Balanced mode"), POWER_PROFILE_MODE_BALANCED},
        {tr("High performance mode"), POWER_PROFILE_MODE_PERFORMANCE}};
    for (auto profile : profiles)
    {
        ui->combo_computerMode->addItem(profile.name, profile.index);
    }

    /// 初始化QSlider,和延迟设置的Timer
    ui->slider_brightness->setMaximum(100);
    ui->slider_brightness->setMinimum(20);
    m_brightnessSettingTimer.setInterval(300);
    m_brightnessSettingTimer.setSingleShot(true);

    ui->slider_idleTime->setMaximum(MAX_IDLE_TIME);
    ui->slider_idleTime->setMinimum(1);
    m_idleTimeSettingTimer.setInterval(300);
    m_idleTimeSettingTimer.setSingleShot(true);

    bool lidIsPresent = m_powerInterface->lidIsPresent();
    ui->widget_lid->setVisible(lidIsPresent);

    // 空闲时是否锁定屏幕及屏保
    m_switchIdlelockScreen = new KiranSwitchButton(this);
    m_switchIdlelockScreen->setAccessibleName("SwitchIdleLockScreen");
    ui->layout_idleLock->addWidget(m_switchIdlelockScreen);

    // 空闲时变暗显示器
    m_switchDisplayIdleDimmed = new KiranSwitchButton(this);
    ui->layout_displayIdleDimmed->addWidget(m_switchDisplayIdleDimmed);

    // 待机唤醒是否需要输入密码
    m_switchSuspendLockScreen = new KiranSwitchButton(this);
    m_switchSuspendLockScreen->setAccessibleName("SwitchSuspendLockScreen");
    ui->layout_suspendLockScreen->addWidget(m_switchSuspendLockScreen);

    // NOTE: 根据#48515缺陷单所做修改,隐藏挂起按钮相关配置项，后续若有需要再进行打开
    ui->widget_suspend->setVisible(false);

    // 先隐藏通用设置->空闲时变暗选项，保留电池设置里得空闲时变暗选项
    ui->widget_displayIdleDimmed->setVisible(false);
}

void GeneralSettingsPage::initConnection()
{
    connect(ui->combo_powerButton, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &GeneralSettingsPage::updateEventAction);
    connect(ui->combo_suspendButton, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &GeneralSettingsPage::updateEventAction);
    connect(ui->combo_closingLid, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &GeneralSettingsPage::updateEventAction);

    connect(ui->combo_computerMode, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &GeneralSettingsPage::updateCurrentComputerMode);

    connect(&m_brightnessSettingTimer, &QTimer::timeout,
            this, &GeneralSettingsPage::updateMonitorBrightness);
    connect(ui->slider_brightness, &QSlider::valueChanged,
            this, &GeneralSettingsPage::onSliderBrightnessValueChanged);

    connect(ui->slider_idleTime, &QSlider::valueChanged,
            this, &GeneralSettingsPage::onSliderIdleTimeChanged);
    connect(&m_idleTimeSettingTimer, &QTimer::timeout,
            this, &GeneralSettingsPage::updateIdleTime);

    connect(m_switchIdlelockScreen, &QAbstractButton::toggled,
            this, &GeneralSettingsPage::updateIdleLockEnable);
    connect(m_switchDisplayIdleDimmed, &QAbstractButton::toggled,
            this, &GeneralSettingsPage::updateDisplayIdleDimmedEnable);
    connect(m_switchSuspendLockScreen, &QAbstractButton::toggled,
            this, &GeneralSettingsPage::updateSuspendLockEnable);
}

void GeneralSettingsPage::load()
{
    // 传入Event Action ComboBox，以及该Action所属的Event事件枚举
    // 从后端拿到该传入Event枚举当前的Action，并更新当前Event Action ComboBox当前项
    auto updateEventActionComboCurrent = [this](QComboBox* comboBox, PowerEvent event) -> void
    {
        auto getEventActionReply = m_powerInterface->GetEventAction(event);
        getEventActionReply.waitForFinished();
        if (getEventActionReply.isError())
        {
            QString errMsg = getEventActionReply.error().message();
            KLOG_WARNING(qLcPower, "get event(%d) current action failed,%s", event, errMsg.toStdString().c_str());
            return;
        }

        auto action = getEventActionReply.value();
        auto comboBoxIdx = comboBox->findData(action);
        if (comboBoxIdx == -1)
        {
            KLOG_WARNING(qLcPower) << "combobox(" << comboBox->objectName() << ") can't find this action(" << action << ")!";
            return;
        }
        QSignalBlocker blocker(comboBox);
        comboBox->setCurrentIndex(comboBoxIdx);
    };

    // 按下电源按钮执行操作
    updateEventActionComboCurrent(ui->combo_powerButton, POWER_EVENT_PRESSED_POWEROFF);

    // 按下挂起按钮执行操作
    updateEventActionComboCurrent(ui->combo_suspendButton, POWER_EVENT_PRESSED_SUSPEND);

    // 关闭盖子执行操作
    updateEventActionComboCurrent(ui->combo_closingLid, POWER_EVENT_LID_CLOSED);

    // 计算机模式
    QSignalBlocker blocker(ui->combo_computerMode);
    int activePorfileComboxIdx = ui->combo_computerMode->findData(m_powerInterface->activeProfile());
    if (activePorfileComboxIdx != -1)
    {
        ui->combo_computerMode->setCurrentIndex(activePorfileComboxIdx);
    }
    else
    {
        KLOG_ERROR() << "can not fidn current active computer mode in combobox:" << m_powerInterface->activeProfile();
    }

    // 显示器亮度调整
    auto monitorBrightnessReply = m_powerInterface->GetBrightness(POWER_DEVICE_TYPE_MONITOR);
    auto monitorBrightnessPercent = monitorBrightnessReply.value();
    if (monitorBrightnessReply.isError())
    {
        KLOG_WARNING(qLcPower) << "can't get monitor brightness!" << monitorBrightnessReply.error();
    }
    else
    {
        // 亮度为-1表示亮度调整不可用
        QSignalBlocker sliderBrightnessBlock(ui->slider_brightness);
        if (monitorBrightnessPercent == -1)
        {
            ui->slider_brightness->setMaximum(ui->slider_brightness->maximum());
            ui->slider_brightness->setEnabled(false);
        }
        else
        {
            setUiBrightnessPercent(monitorBrightnessPercent);
        }
    }

    // 多久判定为空闲
    QSignalBlocker sliderIdleTimeBlocker(ui->slider_idleTime);
    if (m_sessionSettings)
    {
        int idleTime = m_sessionSettings->get(KEY_IDLE_DELAY).toInt();
        updateIdleTimeLabel(idleTime);
        ui->slider_idleTime->setValue(idleTime);
    }
    else
    {
        ui->slider_idleTime->setEnabled(false);
    }

    // 空闲时是否锁定屏幕
    QSignalBlocker idleLockScreenBlocker(m_switchIdlelockScreen);
    if (m_screensaverSettings)
    {
        bool idleActivationLock = m_screensaverSettings->get(KEY_IDLE_ACTIVATION_LOCK).toBool();
        m_switchIdlelockScreen->setChecked(idleActivationLock);
        m_switchIdlelockScreen->setCheckable(true);
    }
    else
    {
        m_switchIdlelockScreen->setChecked(false);
        m_switchIdlelockScreen->setCheckable(false);
    }

    // 空闲时变暗显示器
    QSignalBlocker displayDimmedIdleBlocker(m_switchDisplayIdleDimmed);
    auto displayDimmedIdle = m_powerInterface->displayIdleDimmedEnabled();
    m_switchDisplayIdleDimmed->setChecked(displayDimmedIdle);
    m_switchDisplayIdleDimmed->setCheckable(true);

    // 待机唤醒时需要输入密码
    QSignalBlocker suspendLockScreenBlocker(m_switchSuspendLockScreen);
    auto lockwhenSuspend = m_powerInterface->screenLockedWhenSuspend();
    m_switchSuspendLockScreen->setChecked(lockwhenSuspend);
}

void GeneralSettingsPage::updateSliderIdleTime(int idleTime)
{
    QSignalBlocker blocker(ui->slider_idleTime);
    ui->slider_idleTime->setValue(idleTime);
}

void GeneralSettingsPage::updateEventAction()
{
    PowerEvent event;
    PowerAction action;

    if (!sender())
    {
        KLOG_ERROR(qLcPower) << "update event action to backend error,cannot be directly called!";
        return;
    }

    QComboBox* comboBox = qobject_cast<QComboBox*>(sender());
    if (comboBox == ui->combo_powerButton)
    {
        event = POWER_EVENT_PRESSED_POWEROFF;
    }
    else if (comboBox == ui->combo_suspendButton)
    {
        event = POWER_EVENT_PRESSED_SUSPEND;
    }
    else if (comboBox == ui->combo_closingLid)
    {
        event = POWER_EVENT_LID_CLOSED;
    }
    else
    {
        KLOG_ERROR(qLcPower) << "update event action to backend failed,"
                             << "can't get power event enum" << comboBox;
        return;
    }

    action = (PowerAction)comboBox->currentData(Qt::UserRole).toInt();

    auto reply = m_powerInterface->SetEventAction(event, action);
    reply.waitForFinished();

    if (reply.isError())
    {
        KLOG_ERROR(qLcPower) << "update event action error,"
                             << reply.error()
                             << "event:" << event
                             << "action:" << action;
        return;
    }

    KLOG_INFO(qLcPower, "update event(%d) action(%d) to backend", event, action);
}

void GeneralSettingsPage::updateMonitorBrightness()
{
    KLOG_INFO(qLcPower) << "update monitor brightness" << m_brightnessValue;

    auto reply = m_powerInterface->SetBrightness(POWER_DEVICE_TYPE_MONITOR, m_brightnessValue);
    reply.waitForFinished();

    if (reply.isError())
    {
        KiranMessageBox::message(this, tr("ERROR"), reply.error().message(), KiranMessageBox::Ok);
        KLOG_ERROR(qLcPower) << "update monitor brightness failed,"
                             << reply.error().message();
    }
}

void GeneralSettingsPage::onSliderBrightnessValueChanged(int value)
{
    setUiBrightnessPercent(value);
    m_brightnessSettingTimer.start();
}

void GeneralSettingsPage::onSliderIdleTimeChanged(int value)
{
    updateIdleTimeLabel(value);
    m_idleTimeSettingTimer.start();
}

void GeneralSettingsPage::updateIdleTime()
{
    if (!m_sessionSettings)
    {
        KLOG_WARNING(qLcPower) << "update idle time failed,can't get valid session settings";
        return;
    }

    int value = ui->slider_idleTime->value();
    m_sessionSettings->set(KEY_IDLE_DELAY, value);

    KLOG_INFO(qLcPower) << "update idle time " << value;
}

void GeneralSettingsPage::updateIdleLockEnable(bool enable)
{
    if (!m_screensaverSettings)
    {
        KLOG_WARNING(qLcPower) << "update idle lock enable failed,"
                               << "can't get valid screensaver settings";
        return;
    }

    m_screensaverSettings->set(KEY_IDLE_ACTIVATION_LOCK, enable);

    KLOG_INFO(qLcPower) << "update idle lock enable" << enable;
}

void GeneralSettingsPage::updateDisplayIdleDimmedEnable(bool enable)
{
    m_powerInterface->EnableDisplayIdleDimmed(enable);
    KLOG_INFO(qLcPower) << "update display idle dimmed enable" << enable;
}

void GeneralSettingsPage::updateSuspendLockEnable(bool checked)
{
    m_powerInterface->LockScreenWhenSuspend(checked);
    KLOG_INFO(qLcPower) << "update suspend lock enable" << checked;
}

void GeneralSettingsPage::updateCurrentComputerMode(int idx)
{
    auto computerMode = ui->combo_computerMode->itemData(idx);
    auto reply = m_powerInterface->SwitchProfile(computerMode.toInt());
    reply.waitForFinished();
    if (reply.isError())
    {
        KLOG_ERROR() << "set current computer mode" << computerMode.toInt()
                     << "failed," << reply.error();
    }
    else
    {
        KLOG_DEBUG() << "set current computer mode" << computerMode.toInt();
    }
}

void GeneralSettingsPage::setUiBrightnessPercent(int percent)
{
    if (percent < ui->slider_brightness->minimum())
    {
        percent = ui->slider_brightness->minimum();
    }

    ui->slider_brightness->setValue(percent);
    m_brightnessValue = percent;
    ui->label_brightnessPercent->setText(QString("%1%").arg(percent));
}

QSize GeneralSettingsPage::sizeHint() const
{
    return {500, 657};
}

void GeneralSettingsPage::updateIdleTimeLabel(int min)
{
    QString idleTime;

    int hour = min / 60;
    int minute = min % 60;
    QStringList temp;
    if (hour)
    {
        temp.append(tr("%1hour").arg(hour));
    }
    if (minute)
    {
        temp.append(tr("%1minute").arg(minute));
    }
    idleTime = temp.join(" ");

    ui->label_idleTime->setText(idleTime);
}