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
 * Author:     luoqing <luoqing@kylinsec.com.cn>
 */

#include "output-page.h"
#include <kiran-push-button.h>
#include <qt5-log-i.h>
#include <QComboBox>
#include <QDir>
#include <QFileInfo>
#include <QMediaPlayer>
#include "config.h"
#include "dbus/audio-device-interface.h"
#include "dbus/audio-interface.h"
#include "kiran-session-daemon/audio-i.h"
#include "logging-category.h"
#include "ui_output-page.h"

OutputPage::OutputPage(QWidget *parent) : QWidget(parent),
                                          ui(new Ui::OutputPage),
                                          m_audioInterface(nullptr),
                                          m_defaultSink(nullptr)
{
    ui->setupUi(this);
    m_audioInterface = AudioInterface::instance();
    m_mediaPlayer = new QMediaPlayer(this);
    init();

    m_dbusServiceWatcher = new QDBusServiceWatcher(this);
    m_dbusServiceWatcher->setConnection(QDBusConnection::sessionBus());
    m_dbusServiceWatcher->addWatchedService(AUDIO_DBUS_NAME);

    m_dbusServiceWatcher->setWatchMode(QDBusServiceWatcher::WatchForUnregistration);
    connect(m_dbusServiceWatcher, &QDBusServiceWatcher::serviceUnregistered, [this](const QString &service)
            {
                KLOG_INFO(qLcAudio) << "dbus service unregistered:" << service;
                disableSettings(); });
}

OutputPage::~OutputPage()
{
    delete m_dbusServiceWatcher;
    delete ui;
}

void OutputPage::init()
{
    ui->outputVolume->setStyleSheet("color:#2eb3ff;");
    ui->volumeSetting->setRange(0, 100);
    ui->volumeSetting->setSingleStep(1);
    ui->volumeSetting->setPageStep(1);

    ui->volumeBalance->setRange(-100, 100);
    ui->volumeBalance->setSingleStep(1);
    ui->volumeBalance->setPageStep(1);

    loadSoundExamples();
    KiranPushButton::setButtonType(ui->btn_play, KiranPushButton::BUTTON_Default);
    connect(ui->btn_play, &QPushButton::clicked, this, &OutputPage::playSoundExample);

    initSettins();
    initConnect();
}

void OutputPage::initSettins()
{
    QDBusPendingReply<QString> dbusReply = m_audioInterface->GetDefaultSink();
    KLOG_INFO(qLcAudio) << "default Sink:" << dbusReply;

    if (!dbusReply.isValid())
    {
        KLOG_INFO(qLcAudio) << "default Sink Path error:" << dbusReply.error();
        disableSettings();
        return;
    }

    QString defaultSinkPath = dbusReply.value();
    if (!defaultSinkPath.isEmpty())
    {
        m_defaultSink = new AudioDeviceInterface(AUDIO_DBUS_NAME, defaultSinkPath, QDBusConnection::sessionBus(), this);
        initCardOptions();
        initActivedPort();

        connect(m_defaultSink, &AudioDeviceInterface::volumeChanged, this, &OutputPage::changeVolumeSlider, Qt::UniqueConnection);
        connect(m_defaultSink, &AudioDeviceInterface::balanceChanged, this, &OutputPage::changeBalanceSlider, Qt::UniqueConnection);
        connect(m_defaultSink, &AudioDeviceInterface::active_portChanged, this, &OutputPage::onActivePortChanged, Qt::UniqueConnection);
    }
    else
    {
        disableSettings();
    }
}

void OutputPage::initCardOptions()
{
    QSignalBlocker blocker(ui->outputCards);
    QList<AudioCardInfo> cardsInfo = m_audioInterface->getCards();
    for (auto card : cardsInfo)
    {
        ui->outputCards->addItem(card.name, card.index);
    }
    int index = ui->outputCards->findData(m_defaultSink->card_index());
    ui->outputCards->setCurrentIndex(index);
}

void OutputPage::initActivedPort()
{
    if (!m_defaultSink->isAvailablePorts())
    {
        // 无激活端口则禁用音量设置和平衡
        KLOG_DEBUG(qLcAudio) << "No available ports for current default sink";
        disableSettings();
        return;
    }

    QSignalBlocker blocker(ui->outputDevices);
    ui->outputDevices->setEnabled(true);

    QList<AudioPortInfo> portsInfo = m_defaultSink->getPortsInfo();
    for (auto portInfo : portsInfo)
    {
        if (portInfo.available != PORT_AVAILABLE_NO)
        {
            ui->outputDevices->addItem(portInfo.description, portInfo.name);
        }
    }

    int currentIndex = ui->outputDevices->findData(m_defaultSink->active_port());
    ui->outputDevices->setCurrentIndex(currentIndex);

    /// 存在激活端口才初始化音量和平衡设置
    initVolumeAndBalance();
}

void OutputPage::initVolumeAndBalance()
{
    if (ui->outputDevices->isEnabled())
    {
        ui->volumeSetting->setEnabled(true);
        ui->volumeBalance->setEnabled(true);
    }

    QSignalBlocker volumeSettingBlocker(ui->volumeSetting);
    QSignalBlocker volumeBalanceBlocker(ui->volumeBalance);

    double currentVolumeDouble = m_defaultSink->volume() * 100;
    int currentVolume = round(currentVolumeDouble);
    ui->volumeSetting->setValue(currentVolume);
    ui->outputVolume->setText(QString::number(currentVolume) + "%");

    double currentBalanceDouble = m_defaultSink->balance() * 100;
    ui->volumeBalance->setValue(round(currentBalanceDouble));

    KLOG_DEBUG(qLcAudio) << "current output volume:" << currentVolume;
    KLOG_DEBUG(qLcAudio) << "current output balance:" << round(currentBalanceDouble);
}

void OutputPage::initConnect()
{
    connect(m_audioInterface, &AudioInterface::SinkAdded, this, &OutputPage::addSink);
    connect(m_audioInterface, &AudioInterface::SinkDelete, this, &OutputPage::deleteSink);
    connect(m_audioInterface, &AudioInterface::DefaultSinkChange, this, &OutputPage::defaultSinkChanged, Qt::QueuedConnection);

    connect(ui->outputCards, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &OutputPage::changeDefaultOutputCard);
    connect(ui->outputDevices, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &OutputPage::setActivePort);

    connect(ui->volumeSetting, &QSlider::valueChanged, this, &OutputPage::setVolume);
    connect(ui->volumeBalance, &QSlider::valueChanged, this, &OutputPage::setBalance);
}

void OutputPage::loadSoundExamples()
{
    static QMap<QString, QString> examplesTransMap = {
        {"bark", QT_TR_NOOP("bark")},
        {"drip", QT_TR_NOOP("drip")},
        {"glass", QT_TR_NOOP("glass")},
        {"sonar", QT_TR_NOOP("sonar")},
    };
    QDir examplsDir(AUDIO_SOUNDS_DIR);
    for (auto fileName : examplsDir.entryList(QDir::Files))
    {
        QString filePath = examplsDir.absoluteFilePath(fileName);
        QFileInfo fileInfo(filePath);
        if (fileInfo.suffix() != "ogg")
        {
            continue;
        }

        auto baseName = fileInfo.baseName();
        ui->combo_sound->addItem(tr(baseName.toStdString().c_str()), filePath);
    }
}

void OutputPage::onActivePortChanged(const QString &value)
{
    KLOG_INFO(qLcAudio) << "output device (active Port) changed :" << value;
    QSignalBlocker blocker(ui->outputDevices);
    ui->outputDevices->clear();

    initActivedPort();
}

void OutputPage::changeVolumeSlider(double value)
{
    QSignalBlocker blocker(ui->volumeSetting);  // 为了避免拖动的同时设置位置会出现问题
    int currentVolume = round(value * 100);
    ui->outputVolume->setText(QString::number(currentVolume) + "%");
    ui->volumeSetting->setValue(currentVolume);
}

void OutputPage::changeBalanceSlider(double value)
{
    QSignalBlocker blocker(ui->volumeBalance);
    int currentBalance = round(value * 100);
    ui->volumeBalance->setValue(currentBalance);
}

void OutputPage::setActivePort(int index)
{
    QString portName = ui->outputDevices->itemData(index, Qt::UserRole).toString();
    if (!m_defaultSink || portName.isNull())
    {
        KLOG_INFO(qLcAudio) << QString("set default sink active port: %1 failed").arg(portName);
        return;
    }

    m_defaultSink->SetActivePort(portName);
    KLOG_INFO(qLcAudio) << " set default sink Active Port:" << portName;
}

void OutputPage::setVolume(int value)
{
    double volumeValue = ui->volumeSetting->sliderPosition() / 100.0;
    if (!m_defaultSink)
    {
        KLOG_INFO(qLcAudio) << "set volume failed, default Sink is null";
        return;
    }
    m_defaultSink->SetVolume(volumeValue);
    KLOG_DEBUG(qLcAudio) << "set volume:" << volumeValue;
}

void OutputPage::setBalance(int value)
{
    double balanceValue = value / 100.0;
    if (!m_defaultSink)
    {
        KLOG_INFO(qLcAudio) << "set balance failed, default Sink is null";
        return;
    }
    m_defaultSink->SetBalance(balanceValue);
    KLOG_DEBUG(qLcAudio) << "set balance" << balanceValue;
}

void OutputPage::playSoundExample()
{
    m_mediaPlayer->stop();

    auto soundExamplePath = ui->combo_sound->currentData().toString();
    if (soundExamplePath.isEmpty())
    {
        KLOG_INFO(qLcAudio) << "sound example path is empty";
        return;
    }

    m_mediaPlayer->setMedia(QUrl::fromLocalFile(soundExamplePath));
    m_mediaPlayer->play();
}

/**
 * NOTE:
 * 目前切换声卡实际只是切换了defaultSink
 */
void OutputPage::changeDefaultOutputCard(int index)
{
    int cardIndex = ui->outputCards->itemData(index, Qt::UserRole).toInt();
    KLOG_INFO(qLcAudio) << "change default output card, current output card Index:" << cardIndex;
    QDBusPendingReply<QStringList> getSinks = m_audioInterface->GetSinks();
    QStringList sinksList = getSinks.value();

    int sinkIndex = -1;
    for (auto sink : sinksList)
    {
        AudioDeviceInterface audioSink(AUDIO_DBUS_NAME, sink, QDBusConnection::sessionBus(), this);
        if (cardIndex == (int)audioSink.card_index() && audioSink.isAvailablePorts())
        {
            sinkIndex = audioSink.index();
            break;
        }
    }

    if (sinkIndex == -1)
    {
        KLOG_INFO(qLcAudio) << "The sink with an available port corresponding to the card index was not found";
        KLOG_INFO(qLcAudio) << "set default sink failed";
        disableSettings();
        return;
    }

    QDBusPendingReply<QString> dbusReply = m_audioInterface->GetDefaultSink();
    QString defaultSinkPath = dbusReply.value();
    AudioDeviceInterface defaultSink(AUDIO_DBUS_NAME, defaultSinkPath, QDBusConnection::sessionBus(), this);
    if (sinkIndex == (int)defaultSink.index())
    {
        KLOG_DEBUG(qLcAudio) << "current default sink:" << sinkIndex;
        reload();
        return;
    }

    setDefaultSink(sinkIndex);
}

void OutputPage::disableSettings()
{
    KLOG_INFO(qLcAudio) << "disbale settings";
    QSignalBlocker outputDevicesBlocker(ui->outputDevices);
    QSignalBlocker volumeSettingBlocker(ui->volumeSetting);
    QSignalBlocker volumeBalanceBlocker(ui->volumeBalance);

    ui->outputDevices->insertItem(0, tr("No output device detected"));
    ui->outputDevices->setCurrentIndex(0);

    ui->volumeSetting->setValue(0);
    ui->outputVolume->setText(QString::number(0) + "%");
    ui->volumeBalance->setValue(0);

    ui->outputDevices->setEnabled(false);
    ui->volumeSetting->setEnabled(false);
    ui->volumeBalance->setEnabled(false);
}

void OutputPage::setDefaultSink(int sinkIndex)
{
    /**
     * NOTE:
     * 由于SetDefaultSink不一定生效，且没有返回值表明是否切换DefaultSink成功。
     * 调用SetDefaultSink后统一禁用音量设置，等待 DefaultSinkChange 信号的接收
     * 接收到DefaultSinkChange信号后，确认SetDefaultSink生效后（即切换sink成功），界面再打开和更新设置
     */

    m_audioInterface->SetDefaultSink(sinkIndex);
    KLOG_INFO(qLcAudio) << QString("set default sink:%1").arg(sinkIndex);
    disableSettings();
}

void OutputPage::reload()
{
    KLOG_INFO(qLcAudio) << "reload output device and settings";
    // delete and restart init defaultSink
    clear();
    initSettins();
}

void OutputPage::clear()
{
    if (m_defaultSink != nullptr)
    {
        m_defaultSink->deleteLater();
        m_defaultSink = nullptr;
    }
    QSignalBlocker outputDevicesBlocker(ui->outputDevices);
    ui->outputDevices->clear();

    QSignalBlocker outputCardsBlocker(ui->outputCards);
    ui->outputCards->clear();
}

/**
 * NOTE:
 * 一个sink对应一个输出设备，例如耳机、扬声器，
 * 一个card对应一个声卡
 * card和sink应该是属于多对多的关系
 */
// 默认sink变了，重新比对card_index,重新加载sink和界面
void OutputPage::defaultSinkChanged(int index)
{
    KLOG_INFO(qLcAudio) << "default sink changed";
    reload();
}

void OutputPage::addSink(int index)
{
    KLOG_DEBUG(qLcAudio) << "sink added:" << index;
    reload();
}

// 当pulseAudio被kill时，会发出SinkDelete和SourceDelete信号
void OutputPage::deleteSink(uint index)
{
    KLOG_DEBUG(qLcAudio) << "sink delete:" << index;
    reload();
}

QSize OutputPage::sizeHint() const
{
    return {500, 657};
}
