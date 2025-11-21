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

#include "display-config.h"
#include <kiranwidgets-qt5/kiran-message-box.h>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QDBusReply>
#include "display_backEnd_proxy.h"
#include "monitor_backEnd_proxy.h"

DisplayConfig::DisplayConfig(QObject *parent) : m_displayInterface(nullptr)
{
    qDBusRegisterMetaType<DisplayModesStu>();
    qDBusRegisterMetaType<ListDisplayModesStu>();
    init();
}

DisplayConfig *DisplayConfig::instance()
{
    static QMutex mutex;
    static QScopedPointer<DisplayConfig> pInst;

    if (Q_UNLIKELY(!pInst))
    {
        QMutexLocker locker(&mutex);
        if (pInst.isNull())
        {
            pInst.reset(new DisplayConfig());
        }
    }
    return pInst.data();
}

DisplayConfig::~DisplayConfig()
{
    clear();
}

void DisplayConfig::init()
{
    m_displayInterface = QSharedPointer<DisplayBackEndProxy>(new DisplayBackEndProxy(DISPLAY_DBUS_NAME,
                                                                                     DISPLAY_OBJECT_PATH,
                                                                                     QDBusConnection::sessionBus(),
                                                                                     this));
    m_displayConfigData = QSharedPointer<DisplayConfigData>(new DisplayConfigData(DISPLAY_OBJECT_PATH));
    m_displayConfigData->setWindowScalingFactor(m_displayInterface->window_scaling_factor());
    m_displayConfigData->setPrimary(m_displayInterface->primary());

    initConnect();
}

void DisplayConfig::initConnect()
{
    m_timer.setInterval(100);
    m_timer.setSingleShot(true);

    QStringList monitorsPathList = m_displayInterface->ListMonitors();
    foreach (QString monitorPath, monitorsPathList)
    {
        MonitorInterface monitorInterface = QSharedPointer<MonitorBackEndProxy>(new MonitorBackEndProxy(DISPLAY_DBUS_NAME,
                                                                                                        monitorPath,
                                                                                                        QDBusConnection::sessionBus(),
                                                                                                        this));

        connect(monitorInterface.data(), &MonitorBackEndProxy::dbusPropertyChanged, this, [this](const QString &name, const QVariant &value)
                {
                    m_timer.start();
                });
        m_monitorInterfaceList << monitorInterface;
    }

    connect(m_displayInterface.data(), &DisplayBackEndProxy::dbusPropertyChanged, this, [this](const QString &name, const QVariant &value)
            {
                m_timer.start();
            });
    connect(&m_timer, &QTimer::timeout, this, &DisplayConfig::handleDbusPropertiesChanged);

    connect(m_displayInterface.data(), &DisplayBackEndProxy::MonitorsChanged, this, [this](bool placeholder)
            { KLOG_DEBUG() << "MonitorsChanged:" << placeholder; });
}

DisplayInterface DisplayConfig::displayInterface()
{
    if (m_displayInterface.isNull())
    {
        m_displayInterface = QSharedPointer<DisplayBackEndProxy>(new DisplayBackEndProxy(DISPLAY_DBUS_NAME,
                                                                                         DISPLAY_OBJECT_PATH,
                                                                                         QDBusConnection::sessionBus(),
                                                                                         this));
        KLOG_DEBUG() << "m_displayInterface is null";
        KLOG_DEBUG() << "new displayInterface :" << m_displayInterface.data();
        return m_displayInterface;
    }
    else
    {
        KLOG_DEBUG() << "displayInterface is existed :" << m_displayInterface.data();
        return m_displayInterface;
    }
}

MonitorInterfaceList DisplayConfig::monitorInterfaceList()
{
    QList<MonitorInterface> monitorList;
    QStringList monitorsPathList = m_displayInterface->ListMonitors();
    foreach (QString monitorPath, monitorsPathList)
    {
        KLOG_DEBUG() << "monitorPath:" << monitorPath;
        MonitorInterface monitorInterface = QSharedPointer<MonitorBackEndProxy>(new MonitorBackEndProxy(DISPLAY_DBUS_NAME,
                                                                                                        monitorPath,
                                                                                                        QDBusConnection::sessionBus(),
                                                                                                        this));
        KLOG_DEBUG() << "monitor name:" << monitorInterface->name();
        monitorList << monitorInterface;
    }

    return monitorList;
}

MonitorInterface DisplayConfig::getMonitorInterface(const QString &monitorPath)
{
    MonitorInterface monitorInterface = QSharedPointer<MonitorBackEndProxy>(new MonitorBackEndProxy(DISPLAY_DBUS_NAME,
                                                                                                    monitorPath,
                                                                                                    QDBusConnection::sessionBus(),
                                                                                                    this));
    return monitorInterface;
}

void DisplayConfig::handleSaveBtnClicked()
{
}

void DisplayConfig::handleCancelBtnClicked()
{
}

void DisplayConfig::handleDbusPropertiesChanged()
{
    foreach (MonitorInterface monitorInterface, m_monitorInterfaceList)
    {
        monitorInterface.clear();
    }

    m_monitorInterfaceList.clear();
    QStringList monitorsPathList = m_displayInterface->ListMonitors();
    foreach (QString monitorPath, monitorsPathList)
    {
        MonitorInterface monitorInterface = QSharedPointer<MonitorBackEndProxy>(new MonitorBackEndProxy(DISPLAY_DBUS_NAME,
                                                                                                        monitorPath,
                                                                                                        QDBusConnection::sessionBus(),
                                                                                                        this));

        connect(monitorInterface.data(), &MonitorBackEndProxy::dbusPropertyChanged, this, [this]()
                { m_timer.start(); });
        m_monitorInterfaceList << monitorInterface;
    }

    emit dbusPropertyChanged();
}

bool DisplayConfig::isCopyMode()
{
    MonitorInterfaceList monitorList = monitorInterfaceList();
    int count = monitorList.count();
    if (count == 1) return false;  //如果只有一个屏幕，则为扩展模式。

    foreach (MonitorInterface monitor, monitorList)
    {
        if (!monitor->enabled())
        {
            return false;  //如果存在未开启的显示器，则不为复制模式。
        }
    }

    int x = 0;
    int y = 0;
    unsigned int w = 0;
    unsigned int h = 0;

    if (count > 0)
    {
        MonitorInterface monitor = monitorList.first();
        x = monitor->x();
        y = monitor->y();

        DisplayModesStu displayModeStu = monitor->GetCurrentMode();
        w = displayModeStu.w;
        h = displayModeStu.h;
    }

    foreach (MonitorInterface monitor, monitorList)
    {
        DisplayModesStu displayModeStu = monitor->GetCurrentMode();
        if (x != monitor->x() || y != monitor->y() || w != displayModeStu.w || h != displayModeStu.h)
        {
            return false;
        }
    }

    return true;
}

MonitorConfigDataPtr DisplayConfig::initCopyMode()
{
    foreach (MonitorConfigDataPtr bufferData, m_monitorConfigDataMap)
    {
        bufferData.clear();
    }
    m_monitorConfigDataMap.clear();

    QString text;
    int rotation = 0;
    int reflect = 0;

    auto monitorList = monitorInterfaceList();
    foreach (MonitorInterface monitor, monitorList)
    {
        text += (text.isEmpty() ? "" : "|") + monitor->name();
    }

    if (monitorList.count() > 0)
    {
        rotation = monitorList.first()->rotation();
        reflect = monitorList.first()->reflect();
    }

    MonitorConfigDataPtr monitorConfigData = QSharedPointer<MonitorConfigData>(new MonitorConfigData(KIRAN_SCREEN_COPY_MODE_MONITOR_PATH));

    monitorConfigData->setName(text);
    monitorConfigData->setPosition(0,0);
    monitorConfigData->setWidth(1920);
    monitorConfigData->setHeight(1080);
    monitorConfigData->setRotation(DisplayRotationType(rotation));
    monitorConfigData->setReflect(DisplayReflectTypes(reflect));
    monitorConfigData->setEnabled(true);

    m_currentConfigMode = ConfigMode::CONFIG_MODE_COPY;
    m_monitorConfigDataMap.insert(KIRAN_SCREEN_COPY_MODE_MONITOR_PATH, monitorConfigData);

    return monitorConfigData;
}

MonitorConfigDataList DisplayConfig::initExtraMode()
{
    foreach (MonitorConfigDataPtr bufferData, m_monitorConfigDataMap)
    {
        bufferData.clear();
    }
    m_monitorConfigDataMap.clear();

    MonitorConfigDataList list;
    int offset = 0;
    bool isCopy = isCopyMode();

    auto monitorList = monitorInterfaceList();

    foreach (MonitorInterface monitor, monitorList)
    {
        QString monitorPath = monitor->path();
        MonitorConfigDataPtr monitorConfigData = QSharedPointer<MonitorConfigData>(new MonitorConfigData(monitorPath));

        monitorConfigData->setName(monitor->name());
        monitorConfigData->setPosition(monitor->x() + offset, monitor->y());
        monitorConfigData->setRotation(DisplayRotationType(monitor->rotation()));
        monitorConfigData->setReflect(DisplayReflectTypes(monitor->reflect()));
        monitorConfigData->setEnabled(monitor->enabled());

        DisplayModesStu displayModeStu;
        if (monitor->enabled())
        {
            displayModeStu = monitor->GetCurrentMode();
        }
        //当显示器关闭时，大小将为0，此时使用默认大小1920x1080, 强制让其放置在右侧。
        if (displayModeStu.w == 0 || displayModeStu.h == 0)
        {
            monitorConfigData->setX(99999);
            displayModeStu.w = 1920;
            displayModeStu.h = 1080;
        }
        monitorConfigData->setWidth(displayModeStu.w);
        monitorConfigData->setHeight(displayModeStu.h);
        monitorConfigData->setRefreshRate(displayModeStu.refreshRate);

        if (isCopy)
            offset += monitorConfigData->width();  //如果点击扩展页面时，当前正处于复制模式，那所有x的值都往右边位移。

        list << monitorConfigData;

        m_monitorConfigDataMap.insert(monitorPath, monitorConfigData);
    }
    m_currentConfigMode = ConfigMode::CONFIG_MODE_EXTRA;
    return list;
}

QSharedPointer<DisplayConfigData> DisplayConfig::getDisplayConfigData()
{
    if (m_displayConfigData.isNull())
        return nullptr;
    else
        return m_displayConfigData;
}

QSharedPointer<MonitorConfigData> DisplayConfig::getMonitorConfigData(const QString &monitorPath)
{
    //查找失败，返回nullptr
    return m_monitorConfigDataMap.value(monitorPath);
}

void DisplayConfig::setConfigMode(ConfigMode configMode)
{
    m_currentConfigMode = configMode;
    emit configModeChanged(configMode);
}

ConfigMode DisplayConfig::currentConfigMode()
{
    return m_currentConfigMode;
}

bool DisplayConfig::applyChanges()
{
    if (m_currentConfigMode == ConfigMode::CONFIG_MODE_EXTRA)
    {
        foreach (auto monitorConfigData, m_monitorConfigDataMap)
        {
            if (monitorConfigData->path() != KIRAN_SCREEN_COPY_MODE_MONITOR_PATH)
            {
                KLOG_DEBUG() << "monitorConfigData name:" << monitorConfigData->name();
                KLOG_DEBUG() << "monitorConfigData resolving:" << monitorConfigData->resolving();
                KLOG_DEBUG() << "monitorConfigData refreshRate:" << monitorConfigData->refreshRate();
                KLOG_DEBUG() << "monitorConfigData enabled:" << monitorConfigData->enabled();
                KLOG_DEBUG() << "monitorConfigData reflect:" << monitorConfigData->reflect();
                KLOG_DEBUG() << "monitorConfigData rotation:" << monitorConfigData->rotation();
                KLOG_DEBUG() << "monitorConfigData positon:" << monitorConfigData->position();

                QString monitorPath = monitorConfigData->path();
                DBusInterface::Monitor<QVariant>(monitorPath, "Enable", QVariantList() << monitorConfigData->enabled());
                DBusInterface::Monitor<QVariant>(monitorPath, "SetPosition", QVariantList() << monitorConfigData->position().x() << monitorConfigData->position().y());
                QVariant var;
                var.setValue(QDBusArgument() << ushort(monitorConfigData->rotation()));
                DBusInterface::Monitor<QVariant>(monitorPath, "SetRotation", QVariantList() << var);

                var.setValue(QDBusArgument() << ushort(monitorConfigData->reflect()));
                DBusInterface::Monitor<QVariant>(monitorPath, "SetReflect", QVariantList() << var);

                uint32_t width = uint32_t(monitorConfigData->width());
                uint32_t height = uint32_t(monitorConfigData->height());

                DBusInterface::Monitor<QVariant>(monitorPath, "SetMode", QVariantList() << width << height << monitorConfigData->refreshRate());
            }
        }
        DBusInterface::Display("SetPrimary", QVariantList() << m_displayConfigData->primary());
        KLOG_DEBUG() << "displayConfigData primary:" << m_displayConfigData->primary();
    }
    else
    {
        QStringList monitors = m_displayInterface->ListMonitors();
        MonitorConfigDataPtr monitorConfigData = m_monitorConfigDataMap.value(KIRAN_SCREEN_COPY_MODE_MONITOR_PATH);
        foreach (QString monitorPath, monitors)
        {
            //复制模式下，不显示刷新率，保存时，每个显示器使用自身推荐的刷新率，返回列表的第一项为推荐刷新率
            QList<DisplayModesStu> stuList = DBusInterface::Monitor<QList<DisplayModesStu> >(monitorPath, "ListModes");
            foreach (auto stu, stuList)
            {
                QSize size(stu.w, stu.h);
                if (size == monitorConfigData->resolving())
                {
                    monitorConfigData->setRefreshRate(stu.refreshRate);
                    break;
                }
            }
            DBusInterface::Monitor<QVariant>(monitorPath, "Enable", QVariantList() << true);
            DBusInterface::Monitor<QVariant>(monitorPath, "SetPosition", QVariantList() << monitorConfigData->position().x() << monitorConfigData->position().y());
            QVariant var;
            var.setValue(QDBusArgument() << ushort(monitorConfigData->rotation()));
            DBusInterface::Monitor<QVariant>(monitorPath, "SetRotation", QVariantList() << var);

            var.setValue(QDBusArgument() << ushort(monitorConfigData->reflect()));
            DBusInterface::Monitor<QVariant>(monitorPath, "SetReflect", QVariantList() << var);

            uint32_t width = uint32_t(monitorConfigData->width());
            uint32_t height = uint32_t(monitorConfigData->height());
            DBusInterface::Monitor<QVariant>(monitorPath, "SetMode", QVariantList() << width << height << monitorConfigData->refreshRate());
        }
    }

    KLOG_DEBUG() << "displayConfigData windowScalingFactor:" << m_displayConfigData->windowScalingFactor();
    DBusInterface::Display("SetWindowScalingFactor", QVariantList() << m_displayConfigData->windowScalingFactor());
    DBusInterface::Display("ApplyChanges");

    //    emit applyChanges();
    //    emit saved();
    return true;
}

void DisplayConfig::clear()
{
}

QString DisplayConfig::primary()
{
    return m_displayInterface->primary();
}

int DisplayConfig::windowScalingFactor()
{
    return m_displayInterface->window_scaling_factor();
}

QStringList DisplayConfig::listMonitors()
{
    return m_displayInterface->ListMonitors();
}

QList<DisplayModesStu> DisplayConfig::listModes(const QString &monitorPath)
{
    MonitorInterface currentMonitorInterface = getMonitorInterface(monitorPath);
    QList<DisplayModesStu> list = currentMonitorInterface->ListModes();
    return list;
}

QList<DisplayModesStu> DisplayConfig::listPreferredModes(const QString &monitorPath)
{
    MonitorInterface monitorInterface = getMonitorInterface(monitorPath);
    QList<DisplayModesStu> list = monitorInterface->ListPreferredModes();
    return list;
}

QVariant DBusInterface::Display(const QString &function, const QVariantList &paras, int *flag, const bool &showErrorBox)
{
    //构造一个method_call消息，服务名称为：com.kscmms.security.center.qtdbus，对象路径为：/message, 接口名称为com.kscmms.security.center.qtdbus.sf，method名称为 setLoginFailedOpr
    QDBusMessage message = QDBusMessage::createMethodCall(DISPLAY_DBUS_NAME, DISPLAY_OBJECT_PATH, DISPLAY_DBUS_INTERFACE_NAME,
                                                          function);
    if (!paras.isEmpty()) message.setArguments(paras);
    //发送消息
    QDBusMessage response = QDBusConnection::sessionBus().call(message, QDBus::Block, 3000);
    //判断method是否被正确返回
    if (response.type() == QDBusMessage::ReplyMessage)
    {
        //从返回参数获取返回值
        if (!response.arguments().isEmpty())
            return response.arguments().takeFirst();
    }
    else
    {
        KLOG_ERROR() << "dbus interface failed:"
                     << "\t"
                     << "function: " << function << "\t"
                     << "paras:    " << paras << "\t"
                     << "response: " << response.errorMessage();
        if (flag) *flag = -1;

        if (showErrorBox)
        {
            KiranMessageBox box;
            box.setTitle(QObject::tr("Tips"));

            QPushButton btn;
            btn.setText(QObject::tr("OK(K)"));
            btn.setFixedSize(QSize(200, box.buttonSize().height()));
            btn.setShortcut(Qt::CTRL + Qt::Key_K);
            box.addButton(&btn, QDialogButtonBox::AcceptRole);
            box.setText(response.errorMessage());
            box.exec();
        }

        return response.errorMessage();
    }

    return QVariant();
}

QVariant DBusInterface::MonitorProperty(const QString &dbusPath, const char *name)
{
    QDBusInterface remoteApp(DISPLAY_DBUS_NAME, dbusPath, "com.kylinsec.Kiran.SessionDaemon.Display.Monitor");
    return remoteApp.property(name);
}

QVariant DBusInterface::MonitorSetProperty(const QString &dbusPath, const char *name, const QVariant &value)
{
    QDBusInterface remoteApp(DISPLAY_DBUS_NAME, dbusPath, "com.kylinsec.Kiran.SessionDaemon.Display.Monitor");
    return remoteApp.setProperty(name, value);
}

QVariant DBusInterface::DisplayProperty(const char *name)
{
    QDBusInterface remoteApp(DISPLAY_DBUS_NAME, DISPLAY_OBJECT_PATH, DISPLAY_DBUS_INTERFACE_NAME);
    return remoteApp.property(name);
}
