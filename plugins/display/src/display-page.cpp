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

#include "display-page.h"
#include <kiran-push-button.h>
#include <kiran-session-daemon/display-i.h>
#include <qt5-log-i.h>
#include <QButtonGroup>
#include <QResizeEvent>
#include <QTimer>
#include "display_backEnd_proxy.h"
#include "ui_display-page.h"

DisplayPage::DisplayPage(QWidget *parent)
    : QWidget(parent),
      ui(new Ui::DisplayPage),
      m_displayConfig(),
      m_currentMonitorData(nullptr),
      m_displayConfigData(nullptr),
      m_btnGroup(nullptr)
{
    ui->setupUi(this);

    KiranPushButton::setButtonType(ui->applyButton, KiranPushButton::BUTTON_Default);
    KiranPushButton::setButtonType(ui->cancelButton, KiranPushButton::BUTTON_Normal);
    init();
    initConnect();
    refreshWidget();

    // XXX:复制模式下，暂时不显示刷新率
    ui->label_2->setVisible(false);
    ui->comboBox_refreshRate->setVisible(false);
}

DisplayPage::~DisplayPage()
{
    delete ui;
}

void DisplayPage::init()
{
    ui->scrollAreaWidgetContents->setContentsMargins(0, 0, 10, 0);
    m_btnGroup = new QButtonGroup(this);
    m_btnGroup->addButton(ui->pushButton_copy_display, 0);
    m_btnGroup->addButton(ui->pushButton_extend_display, 1);

    m_displayConfig = DisplayConfig::instance();
    m_displayConfigData = m_displayConfig->getDisplayConfigData();
}

void DisplayPage::initConnect()
{
#if (QT_VERSION < QT_VERSION_CHECK(5, 15, 0))
    connect(m_btnGroup, QOverload<int, bool>::of(&QButtonGroup::buttonToggled), this, &DisplayPage::switchDisplayConfigMode);
#else
    connect(m_btnGroup, &QButtonGroup::idToggled, this, &DisplayPage::switchDisplayConfigMode);
#endif
    connect(ui->panel, &DevicePanel::screenItemChecked, this, &DisplayPage::onScreenItemChecked);

    connect(m_displayConfig, &DisplayConfig::dbusPropertyChanged, this, &DisplayPage::handleDbusPropertiesChanged);

    connect(ui->comboBox_resolving, QOverload<const QString &>::of(&QComboBox::currentTextChanged), this,
            &DisplayPage::handleResolvingCurrentTextChanged);
    connect(ui->comboBox_extra_resolving, QOverload<const QString &>::of(&QComboBox::currentTextChanged), this,
            &DisplayPage::handleExtraResolvingCurrentTextChanged);

    // XXX:复制模式下，暂时不显示刷新率
    //     connect(ui->comboBox_refreshRate, QOverload<int>::of(&QComboBox::currentIndexChanged),this,&DisplayPage::handleRefreshRateChanged);
    connect(ui->comboBox_extra_refreshRate, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &DisplayPage::handleExtraRefreshRateChanged);

    connect(ui->comboBox_windowScalingFactor, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &DisplayPage::handleWindowScalingFactor);
    connect(ui->comboBox_extra_windowScalingFactor, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &DisplayPage::handleWindowScalingFactor);

    connect(ui->applyButton, &QPushButton::clicked, this, &DisplayPage::handleApplyButtonClicked);
    connect(ui->cancelButton, &QPushButton::clicked, this, &DisplayPage::handleCancelButtonClicked);

    connect(ui->enabledButton, &QAbstractButton::toggled, this, &DisplayPage::handleEnabledButtonToggled);
    connect(ui->pushButton_extra_primary, &QAbstractButton::toggled, this, &DisplayPage::handlePrimaryDisplayButtonToggled);
}

// 调用refreshWidget时，configData实际上也刷新了
void DisplayPage::refreshWidget()
{
    // 从DBusInterface获取数据，更新displayBufferDate
    m_displayConfigData->setPrimary(m_displayConfig->primary());
    m_displayConfigData->setWindowScalingFactor(m_displayConfig->windowScalingFactor());

    KLOG_DEBUG() << "refreshWidget";
    m_listMonitors = m_displayConfig->listMonitors();

    QList<MonitorInterface> monitorInterfaceList = m_displayConfig->monitorInterfaceList();
    foreach (MonitorInterface monitorInterface, monitorInterfaceList)  // 如果显示器未开启，且没有最佳分辨率，就认为时虚拟机中的异常情况，就认为此显示器不存在。
    {
        if (!monitorInterface->enabled())
        {
            QList<DisplayModesStu> list = monitorInterface->ListPreferredModes();
            if (!list.isEmpty())
            {
                if (list.first().w == 0 && list.first().h == 0)
                {
                    m_listMonitors.removeAll(monitorInterface->path());
                }
            }
        }
    }

    // 如果只有一个屏幕，应当隐藏“复制模式”和“扩展模式”的选项卡。多屏幕扩展模式（包含单屏幕扩展）。多屏幕复制模式。
    QStringList listMonitors = m_listMonitors;
    ui->tabBtnWidget->setVisible(listMonitors.count() > 1);
    ui->enable_widget->setVisible(listMonitors.count() > 1);

    /*
     * 当一个屏幕时，为扩展显示中的特殊情况，单屏扩展。当多个屏幕时，屏幕位置x,y全部一样，表示为‘复制显示’，否则为‘扩展显示’。
     *‘复制显示’和‘扩展显示’的数据加载函数分别为“onTabChanged(0, true) ”和“onTabChanged(1, true)。
     */
    m_displayConfig->isCopyMode() ? switchDisplayConfigMode(ConfigMode::CONFIG_MODE_COPY, true) : switchDisplayConfigMode(ConfigMode::CONFIG_MODE_EXTRA, true);
}

void DisplayPage::switchDisplayConfigMode(int index, const bool &checked)
{
    if (!checked) return;
    if (m_btnGroup && m_btnGroup->checkedId() != index) m_btnGroup->button(index)->setChecked(true);

    if (index == 0)
    {
        m_displayConfig->setConfigMode(ConfigMode::CONFIG_MODE_COPY);
    }
    else
    {
        m_displayConfig->setConfigMode(ConfigMode::CONFIG_MODE_EXTRA);
    }
    ui->stackedWidget->setCurrentIndex(index);
}

void DisplayPage::onScreenItemChecked(QString monitorPath)
{
    int windowScalingFactor = m_displayConfigData->windowScalingFactor();

    if (monitorPath == KIRAN_SCREEN_COPY_MODE_MONITOR_PATH)
    {
        m_currentMonitorData.clear();
        m_curMonitorPath = KIRAN_SCREEN_COPY_MODE_MONITOR_PATH;
        m_currentMonitorData = m_displayConfig->getMonitorConfigData(KIRAN_SCREEN_COPY_MODE_MONITOR_PATH);

        QList<DisplayModesStu> list = intersectionMonitorModes();
        QMap<int, modeInfoPair> map = getResolutionFromModes(list);
        initComboBoxResolution(ui->comboBox_resolving, map);
        if (m_displayConfig->isCopyMode())  // 当前实际模式不是复制模式，而是从扩展模式初次切换到复制模式，此时，不获取的那个前模式。
        {
            DisplayModesStu stu = curIntersectionMonitorMode();
            // 复制模式没有推荐，直接set text。
            ui->comboBox_resolving->setCurrentText(QString("%1x%2").arg(stu.w).arg(stu.h));
            ui->comboBox_refreshRate->setCurrentText(QString("%1HZ").arg(stu.refreshRate));
            ui->comboBox_windowScalingFactor->setCurrentIndex(windowScalingFactor);
        }
    }
    else
    {
        m_curMonitorPath = monitorPath;
        m_currentMonitorData.clear();
        m_currentMonitorData = m_displayConfig->getMonitorConfigData(monitorPath);
        showExtraModeData(monitorPath);
        ui->comboBox_extra_windowScalingFactor->setCurrentIndex(windowScalingFactor);
    }
}

QList<DisplayModesStu> DisplayPage::intersectionMonitorModes()
{
    QMap<QString, DisplayModesStu> ret;
    QStringList listMonitors = m_listMonitors;
    int count = listMonitors.count();
    for (int i = 0; i < count; ++i)
    {
        QString monitorPath = listMonitors.at(i);
        QList<DisplayModesStu> list = m_displayConfig->listModes(monitorPath);

        QMap<QString, DisplayModesStu> map;
        foreach (DisplayModesStu l, list)
        {
            QString resolution = QString("%1x%2").arg(l.w).arg(l.h);
            if (!map.contains(resolution))
            {
                map.insert(resolution, l);
            }
        }

        if (i == 0)
        {
            ret = map;
        }
        else
        {
            QList<QString> retKeys = ret.keys();
            foreach (QString key, retKeys)
            {
                if (!map.contains(key))
                {
                    ret.remove(key);
                }
            }
        }
    }

    return ret.values();
}

QMap<int, modeInfoPair> DisplayPage::getResolutionFromModes(const QList<DisplayModesStu> &modes)
{
    QMap<int, modeInfoPair> map;
    QMap<QString, QList<double> *> t_textMap;

    foreach (DisplayModesStu stu, modes)
    {
        QString text = QString("%1x%2").arg(stu.w).arg(stu.h);
        if (t_textMap.contains(text))
        {
            double refreshRate = stu.refreshRate;
            if (!t_textMap[text]->contains(refreshRate))
                t_textMap[text]->append(refreshRate);
            continue;
        }

        map.insert(stu.w * stu.h, modeInfoPair(QSize(stu.w, stu.h), QList<double>() << stu.refreshRate));
        t_textMap.insert(text, &map[stu.w * stu.h].second);
    }

    return map;
}

void DisplayPage::initExtraComboBoxResolution(QComboBox *comboBox, const QMap<int, modeInfoPair> &map)
{
    comboBox->clear();
    QString recommend;
    QList<DisplayModesStu> list = m_displayConfig->listPreferredModes(m_curMonitorPath);
    if (!list.isEmpty())
    {
        recommend = QString("%1x%2").arg(list.first().w).arg(list.first().h);
    }

    QMapIterator<int, modeInfoPair> i(map);
    i.toBack();
    while (i.hasPrevious())
    {
        i.previous();
        modeInfoPair pair = i.value();
        QString text = QString("%1x%2").arg(pair.first.width()).arg(pair.first.height());
        if (text == recommend) text += tr(" (recommended)");
        QVariant var;
        var.setValue(pair);
        comboBox->addItem(text, var);
    }
}

void DisplayPage::initExtraComboBoxRefreshRate(QComboBox *comboBox, const QList<double> &refreshRateList)
{
    comboBox->clear();

    double recommendRefreshRate = 0;
    QList<DisplayModesStu> list = m_displayConfig->listPreferredModes(m_curMonitorPath);
    if (!list.isEmpty())
    {
        recommendRefreshRate = list.first().refreshRate;
    }

    QString strPostfix = tr(" (recommended)");
    QList<double> t_refreshRateList = refreshRateList;
    std::sort(t_refreshRateList.begin(), t_refreshRateList.end(), std::greater<double>());
    foreach (double r, t_refreshRateList)
    {
        QString text = QString("%1HZ").arg(QString::asprintf("%.2f", r));
        if (QString::asprintf("%.2f", r) == QString::asprintf("%.2f", recommendRefreshRate))
        {
            text.append(strPostfix);
        }
        comboBox->addItem(text, r);
    }

    for (int i = 0; i < comboBox->count(); i++)
    {
        double refreshRate = comboBox->itemData(i).toDouble();
        if (QString::asprintf("%.2f", refreshRate) == QString::asprintf("%.2f", recommendRefreshRate))
        {
            comboBox->setCurrentIndex(i);
            break;
        }
    }
}

void DisplayPage::selectResolutionComboboxItem(QComboBox *comboBox, const int &w, const int &h)
{
    if (!comboBox) return;

    int count = comboBox->count();
    for (int i = 0; i < count; ++i)
    {
        modeInfoPair pair = comboBox->itemData(i).value<modeInfoPair>();
        if (pair.first.width() == w && pair.first.height() == h)
        {
            comboBox->setCurrentIndex(i);
            return;
        }
    }
}

void DisplayPage::selectRefreshRateComboboxItem(QComboBox *comboBox, const double &r)
{
    if (!comboBox) return;

    int count = comboBox->count();
    for (int i = 0; i < count; ++i)
    {
        double t_r = comboBox->itemData(i).toDouble();
        if (r == t_r)
        {
            comboBox->setCurrentIndex(i);
            return;
        }
    }
}

bool DisplayPage::extraPrimaryBtnStatus(const bool &onlyEnableScreen, const bool &enable)
{
    if (!onlyEnableScreen && enable) return true;
    return false;
}

DisplayModesStu DisplayPage::curIntersectionMonitorMode()
{
    DisplayModesStu ret;
    QStringList listMonitors = m_listMonitors;
    if (listMonitors.count() > 0) ret = DBusInterface::Monitor<DisplayModesStu>(listMonitors.first(), "GetCurrentMode");

    return ret;
}

void DisplayPage::initComboBoxResolution(QComboBox *comboBox, const QMap<int, modeInfoPair> &map)
{
    comboBox->clear();

    QMapIterator<int, modeInfoPair> i(map);
    i.toBack();
    while (i.hasPrevious())
    {
        i.previous();
        modeInfoPair pair = i.value();
        QString text = QString("%1x%2").arg(pair.first.width()).arg(pair.first.height());
        QVariant var;
        var.setValue(pair);
        comboBox->addItem(text, var);
    }
}

void DisplayPage::confirmSaveMessageBox()
{
    KiranMessageBox box(this);
    box.setTitle(tr("Is the display normal?"));

    QPushButton saveBtn;
    saveBtn.setText(tr("Save current configuration(K)"));
    // saveBtn.setFixedSize(QSize(200, box.buttonSize().height()));
    saveBtn.setFixedHeight(box.buttonSize().height());

    QPushButton cancelBtn;
    cancelBtn.setText(tr("Restore previous configuration(R)"));
    // cancelBtn.setFixedSize(QSize(200, box.buttonSize().height()));
    cancelBtn.setFixedHeight(box.buttonSize().height());

    box.addButton(&saveBtn, QDialogButtonBox::AcceptRole);
    box.addButton(&cancelBtn, QDialogButtonBox::RejectRole);
    saveBtn.setShortcut(Qt::CTRL + Qt::Key_K);
    cancelBtn.setShortcut(Qt::CTRL + Qt::Key_R);

    QString text = tr("The display will resume the previous configuration in %1 seconds");
    int countdown = 30;
    QTimer timer;
    timer.setInterval(1000);
    QObject::connect(&timer, &QTimer::timeout, [&]()
                     {
                         box.setText(text.arg(countdown--));
                         if (countdown < 0) box.reject();
                     });
    timer.start();

    box.setText(text.arg(countdown--));
    box.exec();
    if (box.clickedButton() == &saveBtn)
    {
        int flag = 0;
        // 后期可能根据var中返回的值来判断异常。
        QVariant var = DBusInterface::Display("Save", QVariantList(), &flag);
        if (flag < 0)
        {
            KiranMessageBox box;
            box.setTitle(QObject::tr("Tips"));

            QPushButton btn;
            btn.setText(QObject::tr("OK(K)"));
            // btn.setFixedSize(QSize(200, box.buttonSize().height()));
            btn.setFixedHeight(box.buttonSize().height());
            btn.setShortcut(Qt::CTRL + Qt::Key_K);
            box.addButton(&btn, QDialogButtonBox::AcceptRole);
            box.setText(QObject::tr("Failed to apply display settings!%1").arg(var.toString()));
            box.exec();
        }
    }
    else
    {
        int flag = 0;
        QVariant var = DBusInterface::Display("RestoreChanges", QVariantList(), &flag);
        if (flag < 0)
        {
            KiranMessageBox box;
            box.setTitle(QObject::tr("Tips"));

            QPushButton btn;
            btn.setText(QObject::tr("OK(K)"));
            // btn.setFixedSize(QSize(200, box.buttonSize().height()));
            btn.setFixedHeight(box.buttonSize().height());
            btn.setShortcut(Qt::CTRL + Qt::Key_K);
            box.addButton(&btn, QDialogButtonBox::AcceptRole);
            box.setText(QObject::tr("Fallback display setting failed! %1").arg(var.toString()));
            box.exec();
        }
    }
}

void DisplayPage::showExtraModeData(const QString &monitorPath)
{
    QList<DisplayModesStu> list = m_displayConfig->listModes(monitorPath);
    QMap<int, modeInfoPair> map = getResolutionFromModes(list);

    // Get current data from cache
    //----------
    ui->comboBox_extra_resolving->blockSignals(true);
    initExtraComboBoxResolution(ui->comboBox_extra_resolving, map);
    QSize resolving = m_currentMonitorData->resolving();
    selectResolutionComboboxItem(ui->comboBox_extra_resolving, resolving.width(), resolving.height());
    ui->comboBox_extra_resolving->blockSignals(false);

    ui->comboBox_extra_refreshRate->blockSignals(true);
    modeInfoPair pair = ui->comboBox_extra_resolving->currentData().value<modeInfoPair>();
    initExtraComboBoxRefreshRate(ui->comboBox_extra_refreshRate, pair.second);
    selectRefreshRateComboboxItem(ui->comboBox_extra_refreshRate, m_currentMonitorData->refreshRate());
    ui->comboBox_extra_refreshRate->blockSignals(false);

    //--------
    QString clickedName = m_currentMonitorData->name();
    QString primaryName = m_displayConfigData->primary();

    ui->enabledButton->setChecked(m_currentMonitorData->enabled());
    if (ui->enabledButton->isChecked())
        ui->pushButton_extra_primary->setChecked(primaryName == clickedName);

    // 多屏幕扩展模式，只有一个屏幕可用时，该屏幕不现实‘关闭’‘设为主屏幕’两项。
    QStringList enablePaths;  // 可用的屏幕的路径集合
    QStringList listMonitors = m_listMonitors;
    foreach (QString monitor, listMonitors)
    {
        MonitorConfigDataPtr monitorBufferData = m_displayConfig->getMonitorConfigData(monitor);
        if (monitorBufferData->enabled()) enablePaths << monitor;
    }
    if (enablePaths.count() <= 1 && enablePaths.contains(m_curMonitorPath))
    {
        // 当只剩一个开启的显示器时，选择为主显示器
        if (ui->enabledButton->isChecked())
            ui->pushButton_extra_primary->setChecked(true);
    }
    else
    {
        ui->enabledButton->setEnabled(true);
        ui->pushButton_extra_primary->setEnabled(extraPrimaryBtnStatus(false, ui->enabledButton->isChecked()));
    }
}

void DisplayPage::initComboBoxRefreshRate(QComboBox *comboBox, const QList<double> &refreshRateList)
{
    comboBox->clear();

    QList<double> t_refreshRateList = refreshRateList;
    std::sort(t_refreshRateList.begin(), t_refreshRateList.end(), std::greater<double>());
    foreach (double r, t_refreshRateList)
    {
        QString text = QString("%1HZ").arg(QString::asprintf("%.2f", r));
        comboBox->addItem(text, r);
    }
}

void DisplayPage::handleResolvingCurrentTextChanged(const QString &text)
{
    if (!text.isEmpty())
    {
        modeInfoPair pair = ui->comboBox_resolving->currentData().value<modeInfoPair>();
        m_currentMonitorData->setResolving(pair.first);
        initComboBoxRefreshRate(ui->comboBox_refreshRate, pair.second);
    }
}

void DisplayPage::handleExtraResolvingCurrentTextChanged(const QString &text)
{
    if (!text.isEmpty())
    {
        modeInfoPair pair = ui->comboBox_extra_resolving->currentData().value<modeInfoPair>();
        m_currentMonitorData->setResolving(pair.first);
        initExtraComboBoxRefreshRate(ui->comboBox_extra_refreshRate, pair.second);
    }
}

void DisplayPage::handleRefreshRateChanged(int index)
{
    QVariant data = ui->comboBox_refreshRate->currentData();
    if (data.isValid())
        m_currentMonitorData->setRefreshRate(data.toDouble());
}

void DisplayPage::handleExtraRefreshRateChanged(int index)
{
    QVariant data = ui->comboBox_extra_refreshRate->currentData();
    if (data.isValid())
    {
        m_currentMonitorData->setRefreshRate(data.toDouble());
    }
}

void DisplayPage::handleWindowScalingFactor(int index)
{
    m_displayConfigData->setWindowScalingFactor(index);
}

void DisplayPage::handleDbusPropertiesChanged()
{
    KLOG_DEBUG() << "DbusPropertiesChanged";
    refreshWidget();
}

void DisplayPage::handleEnabledButtonToggled(bool checked)
{
    m_currentMonitorData->setEnabled(checked);
    if (checked == false)
        ui->pushButton_extra_primary->setChecked(false);
    ui->pushButton_extra_primary->setEnabled(extraPrimaryBtnStatus(!ui->enabledButton->isEnabled(), checked));

    ui->comboBox_extra_resolving->setEnabled((checked));
    ui->comboBox_extra_refreshRate->setEnabled(checked);
    ui->comboBox_extra_windowScalingFactor->setEnabled(checked);
    ui->panel->changeItemDisabled(checked);
}

void DisplayPage::handlePrimaryDisplayButtonToggled(bool checked)
{
    if (checked)
        m_displayConfigData->setPrimary(m_currentMonitorData->name());
}

QSize DisplayPage::sizeHint() const
{
    return QSize(679, 765);
}

void DisplayPage::handleApplyButtonClicked()
{
    m_displayConfig->applyChanges();
    confirmSaveMessageBox();
    refreshWidget();
}

void DisplayPage::handleCancelButtonClicked()
{
    if (Q_UNLIKELY(nullptr == parentWidget()))
    {
        this->close();
    }
    else
    {
        this->window()->close();
    }
}
