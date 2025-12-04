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

#include "fonts.h"
#include "appearance-global-info.h"
#include "logging-category.h"
#include "ui_fonts.h"

#include <kiran-log/qt5-log-i.h>
#include <kiran-message-box.h>
#include <kiran-session-daemon/appearance-i.h>
#include <QCompleter>
#include <QFontDatabase>

using namespace std;

#define MIN_FONT_SIZE 7
#define MAX_FONT_SIZE 14

Fonts::Fonts(QWidget* parent)
    : QWidget(parent),
      ui(new Ui::Fonts)
{
    ui->setupUi(this);
    m_comboFontTypesMap = {
        {ui->combo_system, {APPEARANCE_FONT_TYPE_APPLICATION, APPEARANCE_FONT_TYPE_WINDOW_TITLE, APPEARANCE_FONT_TYPE_DESKTOP}},
        {ui->combo_monospace, {APPEARANCE_FONT_TYPE_MONOSPACE}}};

    initUI();
}

Fonts::~Fonts()
{
    delete ui;
}

bool Fonts::initUI()
{
    // 初始化字号选择滑动条
    ui->slider->setRange(MIN_FONT_SIZE, MAX_FONT_SIZE);

    QList<KiranSlider::MarkPoint> markPoints;
    for (int i = 0; i <= MAX_FONT_SIZE; i++)
    {
        KiranSlider::MarkPoint markPoint(i, QString::number(i));
        markPoints << markPoint;
    }

    ui->slider->addMarks(markPoints);

    // 统一QComboBox样式，并初始化可选值列表
    QList<QComboBox*> comboBoxList = this->findChildren<QComboBox*>();
    foreach (QComboBox* comboBox, comboBoxList)
    {
        // comboBox->setStyleSheet("QComboBox {combobox-popup: 0;}");

        auto complete = new QCompleter(comboBox->model());
        complete->setFilterMode(Qt::MatchContains);

        comboBox->setCompleter(complete);
    }

    // 初始化下拉框字体族列表
    QFontDatabase fontDatabase;
    auto fontFamilies = fontDatabase.families();

    QList<QComboBox*> fillCombos({ui->combo_system, ui->combo_monospace});
    for (auto combo = fillCombos.begin();
         combo != fillCombos.end();
         combo++)
    {
        (*combo)->addItems(fontFamilies);
    }

    // 读取一个字体类型的字号作为当前字号滑块值
    auto appearanceInterface = AppearanceGlobalInfo::instance();
    QString fontName;
    int wordSize = 9;
    if (!appearanceInterface->getFont(APPEARANCE_FONT_TYPE_APPLICATION, fontName, wordSize))
    {
        KLOG_ERROR(qLcAppearance) << "load current font word size error!";
    }
    else
    {
        ui->slider->setValue(wordSize);
    }

    // 当前相应的字体类型的字体族作为显示
    updateUiCurrentFontFamily(ui->combo_system);
    updateUiCurrentFontFamily(ui->combo_monospace);

    m_updateFontSizeTimer.setInterval(200);
    m_updateFontSizeTimer.setSingleShot(true);
    connect(&m_updateFontSizeTimer, &QTimer::timeout, this, &Fonts::updateAllFontWordSize);

    // 初始化连接
    initConnections();
    return true;
}

void Fonts::initConnections()
{
    connect(AppearanceGlobalInfo::instance(), &AppearanceGlobalInfo::fontChanged,
            this, &Fonts::onBackendFontChanged);

    for (auto fontTypeCombo : m_comboFontTypesMap.keys())
    {
        connect(fontTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
                this, &Fonts::onCurrentFontFamilyChanged);
    }

    connect(ui->slider, &QSlider::valueChanged, this, &Fonts::onSliderValueChanged);
    connect(ui->btn_resetFontSettings, &QPushButton::clicked, this, &Fonts::resetFontSettings);
}

bool Fonts::updateFontToBackend(int fontType, const QString& fontFamily, int fontSize)
{
    QString fontInfo = QString("%1 %2").arg(fontFamily).arg(fontSize);
    bool res = AppearanceGlobalInfo::instance()->setFont(fontType, fontInfo);
    if (!res)
    {
        KLOG_ERROR(qLcAppearance) << "set font" << fontType << fontInfo << "failed!";
    }
    else
    {
        KLOG_INFO(qLcAppearance) << "set font" << fontType << fontInfo;
    }

    return res;
}

void Fonts::onCurrentFontFamilyChanged()
{
    auto senderComboBox = qobject_cast<QComboBox*>(sender());
    int currentFontSize = ui->slider->value();

    if (m_comboFontTypesMap.find(senderComboBox) == m_comboFontTypesMap.end())
    {
        return;
    }

    auto fontTypes = m_comboFontTypesMap[senderComboBox];
    for (auto fontType : fontTypes)
    {
        updateFontToBackend(fontType, senderComboBox->currentText(), currentFontSize);
    }
}

void Fonts::onSliderValueChanged(int value)
{
    m_updateFontSizeTimer.start();
}

void Fonts::updateUiCurrentFontFamily(QComboBox* combo)
{
    if (m_comboFontTypesMap.find(combo) == m_comboFontTypesMap.end())
    {
        KLOG_WARNING(qLcAppearance) << "can not find ComboBox in ComboFontTypesMap";
        ;
        return;
    }

    QList<int> fontTypes = m_comboFontTypesMap.find(combo).value();
    int fontType = fontTypes.at(0);

    QString fontName;
    int fontSize;
    if (!AppearanceGlobalInfo::instance()->getFont(fontType, fontName, fontSize))
    {
        KLOG_ERROR(qLcAppearance, "get font failed,font type:%d", fontType);
        return;
    }

    auto idx = combo->findText(fontName);
    if (idx == -1)
    {
        KLOG_ERROR(qLcAppearance) << "can not find" << fontName << "in" << combo->objectName();
        return;
    }

    combo->setCurrentText(fontName);
}

void Fonts::updateAllFontWordSize()
{
    int value = ui->slider->value();
    for (auto combo : m_comboFontTypesMap.keys())
    {
        auto family = combo->currentText();
        auto wordSize = value;
        for (auto fontTypeEnum : m_comboFontTypesMap[combo])
        {
            updateFontToBackend(fontTypeEnum, family, wordSize);
        }
    }
}

void Fonts::onBackendFontChanged(int type, const QString& fontFamily, int fontSize)
{
    for (QMap<QComboBox*, QList<int>>::Iterator iter = m_comboFontTypesMap.begin();
         iter != m_comboFontTypesMap.end();
         iter++)
    {
        if (iter.value().contains(type))
        {
            KLOG_INFO() << "backend font changed:" << type << fontFamily << fontSize;
            int fontFamilyIdx = iter.key()->findText(fontFamily);
            if (fontFamilyIdx != -1)
            {
                QSignalBlocker fontFamilyChangedBlocker(iter.key());
                iter.key()->setCurrentIndex(fontFamilyIdx);
            }
            break;
        }
    }

    if (type == APPEARANCE_FONT_TYPE_APPLICATION)
    {
        if ((fontSize >= MIN_FONT_SIZE) && (fontSize <= MAX_FONT_SIZE))
        {
            QSignalBlocker fontSizeSliderBlocker(ui->slider);
            ui->slider->setValue(fontSize);
            ui->slider->ensureLayoutUpdated();
        }
    }
}

void Fonts::resetFontSettings()
{
    for (auto fontTypeList : m_comboFontTypesMap.values())
    {
        for (auto fontType : fontTypeList)
        {
            AppearanceGlobalInfo::instance()->resetFont(fontType);
        }
    }
}

QSize Fonts::sizeHint() const
{
    return {500, 657};
}

QWidget* Fonts::createPage()
{
    return new Fonts();
}
