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
#include "input-page.h"
#include "dbus/audio-device-interface.h"
#include "dbus/audio-interface.h"
#include "ui_input-page.h"
#include "logging-category.h"
#include <QSignalBlocker>

#include <kiran-session-daemon/audio-i.h>
#include <qt5-log-i.h>
#include <cstring>

AudioInfo::AudioInfo(const QAudioFormat &format, QObject *parent)
    : QIODevice(parent), m_format(format), m_maxAmplitude(0), m_level(0.0)
{
    switch (m_format.sampleSize())
    {
    case 8:
        switch (m_format.sampleType())
        {
        case QAudioFormat::UnSignedInt:
            m_maxAmplitude = 255;
            break;
        case QAudioFormat::SignedInt:
            m_maxAmplitude = 127;
            break;
        default:
            break;
        }
        break;
    case 16:
        switch (m_format.sampleType())
        {
        case QAudioFormat::UnSignedInt:
            m_maxAmplitude = 65535;
            break;
        case QAudioFormat::SignedInt:
            m_maxAmplitude = 32767;
            break;
        default:
            break;
        }
        break;
    case 32:
        switch (m_format.sampleType())
        {
        case QAudioFormat::UnSignedInt:
            m_maxAmplitude = 0xffffffff;
            break;
        case QAudioFormat::SignedInt:
            m_maxAmplitude = 0x7fffffff;
            break;
        case QAudioFormat::Float:
            m_maxAmplitude = 0x7fffffff;
            break;
        default:
            break;
        }
    default:
        break;
    }
}

AudioInfo::~AudioInfo()
{
}

void AudioInfo::start()
{
    open(QIODevice::WriteOnly);
}

void AudioInfo::stop()
{
    close();
}

qint64 AudioInfo::readData(char *data, qint64 maxlen)
{
    Q_UNUSED(data);
    Q_UNUSED(maxlen);
    return 0;
}

qint64 AudioInfo::writeData(const char *data, qint64 len)
{
    if (m_maxAmplitude)
    {
        Q_ASSERT(m_format.sampleSize() % 8 == 0);
        const int channelBytes = m_format.sampleSize() / 8;
        const int sampleBytes = m_format.channelCount() * channelBytes;
        Q_ASSERT(len % sampleBytes == 0);
        const int numSamples = len / sampleBytes;

        quint32 maxValue = 0;
        const unsigned char *ptr = reinterpret_cast<const unsigned char *>(data);

        for (int i = 0; i < numSamples; ++i)
        {
            for (int j = 0; j < m_format.channelCount(); ++j)
            {
                quint32 value = 0;

                if (m_format.sampleSize() == 8 && m_format.sampleType() == QAudioFormat::UnSignedInt)
                {
                    value = *reinterpret_cast<const quint8 *>(ptr);
                }
                else if (m_format.sampleSize() == 8 && m_format.sampleType() == QAudioFormat::SignedInt)
                {
                    value = qAbs(*reinterpret_cast<const qint8 *>(ptr));
                }
                else if (m_format.sampleSize() == 16 && m_format.sampleType() == QAudioFormat::UnSignedInt)
                {
                    if (m_format.byteOrder() == QAudioFormat::LittleEndian)
                        value = qFromLittleEndian<quint16>(ptr);
                    else
                        value = qFromBigEndian<quint16>(ptr);
                }
                else if (m_format.sampleSize() == 16 && m_format.sampleType() == QAudioFormat::SignedInt)
                {
                    if (m_format.byteOrder() == QAudioFormat::LittleEndian)
                        value = qAbs(qFromLittleEndian<qint16>(ptr));
                    else
                        value = qAbs(qFromBigEndian<qint16>(ptr));
                }
                else if (m_format.sampleSize() == 32 && m_format.sampleType() == QAudioFormat::UnSignedInt)
                {
                    if (m_format.byteOrder() == QAudioFormat::LittleEndian)
                        value = qFromLittleEndian<quint32>(ptr);
                    else
                        value = qFromBigEndian<quint32>(ptr);
                }
                else if (m_format.sampleSize() == 32 && m_format.sampleType() == QAudioFormat::SignedInt)
                {
                    if (m_format.byteOrder() == QAudioFormat::LittleEndian)
                        value = qAbs(qFromLittleEndian<qint32>(ptr));
                    else
                        value = qAbs(qFromBigEndian<qint32>(ptr));
                }
                else if (m_format.sampleSize() == 32 && m_format.sampleType() == QAudioFormat::Float)
                {
                    // const float* floatValue = reinterpret_cast<const float *>(ptr);
                    float floatValue[1] = {0};
                    std::memcpy(floatValue, ptr, sizeof(float));
                    value = qAbs( (*floatValue) * 0x7fffffff);  // assumes 0-1.0
                }

                maxValue = qMax(value, maxValue);
                ptr += channelBytes;
            }
        }

        maxValue = qMin(maxValue, m_maxAmplitude);
        m_level = qreal(maxValue) / m_maxAmplitude;
    }

    emit update();
    return len;
}

InputPage::InputPage(QWidget *parent) : QWidget(parent), ui(new Ui::InputPage)
{
    ui->setupUi(this);
    init();
}

InputPage::~InputPage()
{
    delete ui;
}

void InputPage::init()
{
    m_audioInterface = AudioInterface::instance();

    ui->inputVolume->setStyleSheet("color:#2eb3ff;");
    ui->volumeSetting->setRange(0, 100);
    ui->volumeSetting->setSingleStep(1);
    ui->volumeSetting->setPageStep(1);

    initSettings();
    initConnet();
}

void InputPage::initSettings()
{
    QDBusPendingReply<QString> dbusReply = m_audioInterface->GetDefaultSource();
    KLOG_DEBUG(qLcAudio) << "default Source Path" << dbusReply;

    if (!dbusReply.isValid())
    {
        disableSettings();
        return;
    }

    QString defaultSourcePath = dbusReply.value();
    if (!defaultSourcePath.isEmpty())
    {
        m_defaultSource = new AudioDeviceInterface(AUDIO_DBUS_NAME, defaultSourcePath, QDBusConnection::sessionBus(), this);
        initCardOptions();
        initActivedPort();
        connect(m_defaultSource, &AudioDeviceInterface::volumeChanged, this, &InputPage::onVolumeChanged,Qt::UniqueConnection);
        connect(m_defaultSource, &AudioDeviceInterface::active_portChanged, this, &InputPage::onActivePortChanged,Qt::UniqueConnection);
    }
    else
    {
        disableSettings();
    }
}

void InputPage::initCardOptions()
{
    QSignalBlocker blocker(ui->inputCards);
    QList<AudioCardInfo> cardsInfo = m_audioInterface->getCards();
    for (auto card : cardsInfo)
    {
        ui->inputCards->addItem(card.name, card.index);
    }
    int index = ui->inputCards->findData(m_defaultSource->card_index());
    ui->inputCards->setCurrentIndex(index);
}

void InputPage::initActivedPort()
{
    if (!m_defaultSource->isAvailablePorts())
    {
        KLOG_INFO(qLcAudio) << "No available ports for current default source";
        disableSettings();
        return;
    }

    QSignalBlocker blocker(ui->inputDevices);
    ui->inputDevices->setEnabled(true);
    m_isValidPort = true;

    QList<AudioPortInfo> portsInfo = m_defaultSource->getPortsInfo();
    for (auto portInfo : portsInfo)
    {
        if (portInfo.available != PORT_AVAILABLE_NO)
        {
            ui->inputDevices->addItem(portInfo.description, portInfo.name);
        }
    }
    int currentIndex = ui->inputDevices->findData(m_defaultSource->active_port());
    ui->inputDevices->setCurrentIndex(currentIndex);

    // 端口可用后才初始化音量设置和音量反馈
    initVolume();
    initVoulumeFeedBack();
}

void InputPage::initVolume()
{
    if (ui->inputDevices->isEnabled())
    {
        ui->volumeSetting->setEnabled(true);
    }

    QSignalBlocker blocker(ui->volumeSetting);

    double currentVolumeDouble = m_defaultSource->volume() * 100;
    int currentVolume = round(currentVolumeDouble);
    ui->volumeSetting->setValue(currentVolume);
    ui->inputVolume->setText(QString::number(currentVolume) + "%");

    KLOG_DEBUG(qLcAudio) << "current input volume:" << currentVolume;
}

void InputPage::initConnet()
{
    connect(ui->inputCards, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &InputPage::changeDefaultInputCard);
    connect(ui->inputDevices, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &InputPage::setActivePort);
    connect(ui->volumeSetting, &QSlider::valueChanged, this, &InputPage::setVolume);

    connect(m_audioInterface, &AudioInterface::SourceAdded, this, &InputPage::addSource);
    connect(m_audioInterface, &AudioInterface::SourceDelete, this, &InputPage::deleteSource);
    connect(m_audioInterface, &AudioInterface::DefaultSourceChange, this, &InputPage::onDefaultSourceChanged, Qt::QueuedConnection);
}

void InputPage::disableSettings()
{
    QSignalBlocker inputDevicesBlocker(ui->inputDevices);
    QSignalBlocker volumeSettingBlocker(ui->volumeSetting);

    m_isValidPort = false;
    ui->inputDevices->insertItem(0, tr("No input device detected"));
    ui->inputDevices->setCurrentIndex(0);

    ui->volumeSetting->setValue(0);
    ui->inputVolume->setText(QString::number(0) + "%");

    ui->inputDevices->setEnabled(false);
    ui->volumeSetting->setEnabled(false);

    ui->volumeScale->setPercent(0);

    clearFeedBack();
}

void InputPage::onActivePortChanged(const QString &value)
{
    KLOG_INFO(qLcAudio) << "input device (active port) changed :" << value;
    QSignalBlocker blocker(ui->inputDevices);
    ui->inputDevices->clear();
    clearFeedBack();

    initActivedPort();
}

void InputPage::onVolumeChanged(double value)
{
    QSignalBlocker blocker(ui->volumeSetting);
    int currentVolume = round(value * 100);
    ui->inputVolume->setText(QString::number(currentVolume) + "%");
    ui->volumeSetting->setValue(currentVolume);
    KLOG_DEBUG(qLcAudio) << "input volume changed:" << currentVolume;
}

/**
 * NOTE:
 * 目前切换输入声卡实际只是切换了defaultSource
 * 一个card可以有多个Soucre，选择了card的后，按以下步骤设置默认DefaultSource
 * 1、遍历一个card的所有Source
 * 2、获取单个Source->getPortsInfo，是否有可用的port，如果有,则按遍历顺序选择第一个port可用的Source
 * 3、如果没有port可用的Source，则设置失败
 */
void InputPage::changeDefaultInputCard(int index)
{
    int cardIndex = ui->inputCards->itemData(index, Qt::UserRole).toInt();
    KLOG_INFO(qLcAudio) << "change default input card, current input card Index:" << cardIndex;
    QDBusPendingReply<QStringList> getSources = m_audioInterface->GetSources();
    QStringList sourcesList = getSources.value();

    int sourceIndex = -1;
    for (auto source : sourcesList)
    {
        AudioDeviceInterface audioSource(AUDIO_DBUS_NAME, source, QDBusConnection::sessionBus(), this);
        if ((cardIndex == (int)audioSource.card_index()) &&
            (audioSource.isAvailablePorts()))
        {
            sourceIndex = audioSource.index();
            break;
        }
    }

    if (sourceIndex == -1)
    {
        KLOG_INFO(qLcAudio) << "The source with an available port corresponding to the card index was not found";
        KLOG_INFO(qLcAudio) << "set default source failed";
        disableSettings();
        return;
    }

    QDBusPendingReply<QString> dbusReply = m_audioInterface->GetDefaultSource();
    QString defaultSourcePath = dbusReply.value();
    AudioDeviceInterface defaultSource(AUDIO_DBUS_NAME, defaultSourcePath, QDBusConnection::sessionBus(), this);

    if (sourceIndex == (int)defaultSource.index())
    {
        KLOG_INFO(qLcAudio) << "current default source:" << sourceIndex;
        reload();
        return;
    }

    setDefaultSource(sourceIndex);
}

void InputPage::setDefaultSource(int sourceIndex)
{
    /**
     * NOTE:
     * 由于SetDefaultSource不一定生效，且没有返回值表明是否切换DefaultSource成功。
     * 调用SetDefaultSource后统一禁用音量设置，等待 DefaultSinkChange 信号的接收
     * 接收到DefaultSourceChange信号后，确认SetDefaultSource生效后（即切换Source成功），界面再打开和更新设置
     */
    m_audioInterface->SetDefaultSource(sourceIndex);
    KLOG_INFO(qLcAudio) << QString("set default sourcee:%1").arg(sourceIndex);
    disableSettings();
}

void InputPage::setVolume(int value)
{
    double volumeValue = value / 100.0;
    if (m_defaultSource != nullptr)
    {
        m_defaultSource->SetVolume(volumeValue);
        KLOG_DEBUG(qLcAudio) << "set input Volume:" << volumeValue;
    }
    else
    {
        KLOG_INFO(qLcAudio) << "set input volume failed, default source is null";
    }
}

void InputPage::onDefaultSourceChanged(int index)
{
    KLOG_DEBUG(qLcAudio) << "Default Source Changed:" << index;
    // delete and restart init defaultSource
    reload();
}

void InputPage::setActivePort(int index)
{
    QString portName = ui->inputDevices->itemData(index, Qt::UserRole).toString();
    if ((m_defaultSource != nullptr) && !portName.isNull())
    {
        m_defaultSource->SetActivePort(portName);
        KLOG_INFO(qLcAudio) << " set default source Active Port:" << portName;
    }
    else
    {
        KLOG_INFO(qLcAudio) << QString("set default source active port: %1 failed").arg(portName);
    }
}

// 暂时没有处理Source增加减少的需求
void InputPage::addSource(int index)
{
    KLOG_INFO(qLcAudio) << "Source Added:" << index;
    reload();
}

void InputPage::deleteSource(int index)
{
    KLOG_INFO(qLcAudio) << "Source Delete:" << index;
    reload();
}

QSize InputPage::sizeHint() const
{
    return {500, 657};
}

// 通过QMultimedia模块获取反馈音量
// 首先使用QAudioFormat类设置音频流参数信息，然后从QIODevice中读取PCM数据
// 最后使用QAudioInput类接收从输入设备来的音频数据
void InputPage::initVoulumeFeedBack()
{
    initAudioFormat();
    m_audioInfo = new AudioInfo(m_format, this);
    connect(m_audioInfo, &AudioInfo::update, this, &InputPage::refreshFeedBack);
    initAudioInput();
}

void InputPage::initAudioFormat()
{
    m_format.setSampleRate(8000);
    m_format.setChannelCount(2);
    m_format.setSampleSize(16);
    m_format.setSampleType(QAudioFormat::SignedInt);
    m_format.setByteOrder(QAudioFormat::LittleEndian);
    m_format.setCodec("audio/pcm");

    m_device = QAudioDeviceInfo::defaultInputDevice();
    // Constructs a copy of other.
    QAudioDeviceInfo info(m_device);
    if (!info.isFormatSupported(m_format))
    {
        KLOG_WARNING() << "Default format not supported - trying to use nearest";
        m_format = info.nearestFormat(m_format);
    }
}

void InputPage::initAudioInput()
{
    m_audioInput = new QAudioInput(m_device, m_format, this);
    m_audioInfo->start();
    m_audioInput->start(m_audioInfo);
}

// XXX: QIODevice一直在监听PCM数据，可以优化一下,或许100ms获取一次数据
void InputPage::refreshFeedBack()
{
    ui->volumeScale->setPercent(m_audioInfo->level());
}

void InputPage::reload()
{
    KLOG_INFO(qLcAudio) << "reload input settings";
    clear();
    initSettings();
}

void InputPage::clear()
{
    ui->volumeScale->setPercent(0);

    if (m_defaultSource != nullptr)
    {
        m_defaultSource->deleteLater();
        m_defaultSource = nullptr;
    }

    QSignalBlocker inputDevicesBlocker(ui->inputDevices);
    QSignalBlocker inputCardsBlocker(ui->inputCards);
    ui->inputDevices->clear();
    ui->inputCards->clear();

    clearFeedBack();
}

void InputPage::clearFeedBack()
{
    if (m_audioInfo != nullptr)
    {
        m_audioInfo->stop();
        m_audioInfo->deleteLater();
        m_audioInfo = nullptr;
    }

    if (m_audioInput != nullptr)
    {
        m_audioInput->deleteLater();
        m_audioInput = nullptr;
    }
}
