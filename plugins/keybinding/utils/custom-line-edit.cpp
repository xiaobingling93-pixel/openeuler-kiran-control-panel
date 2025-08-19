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

#include "custom-line-edit.h"
#include <kiran-log/qt5-log-i.h>
#include <QInputMethodEvent>
#include <QMetaEnum>
#include <QPainter>
#include <QStyleOption>
#include "keybinding_def.h"
#include "keycode-helper.h"
#include "keycode-translator.h"
#include "logging-category.h"

static const QList<int> ignoreKeys =
    {
        Qt::Key_VolumeDown,
        Qt::Key_VolumeMute,
        Qt::Key_VolumeUp,
        Qt::Key_MediaPlay,
        Qt::Key_MediaStop,
        Qt::Key_MediaPrevious,
        Qt::Key_MediaNext,
        Qt::Key_MediaRecord,
        Qt::Key_MediaPause,
        Qt::Key_MediaTogglePlayPause,
        Qt::Key_HomePage,
        Qt::Key_Favorites,
        Qt::Key_Search,
        Qt::Key_Standby,
        Qt::Key_OpenUrl,
        Qt::Key_LaunchMail,
        Qt::Key_LaunchMedia,
        Qt::Key_Launch0,
        Qt::Key_Launch1,
        Qt::Key_Eject,
        Qt::Key_WWW,
        Qt::Key_Explorer,
        Qt::Key_Tools,
        Qt::Key_MicMute};

CustomLineEdit::CustomLineEdit(QWidget *parent) : QLineEdit(parent)
{
    initUI();
}

CustomLineEdit::~CustomLineEdit()
{
}

void CustomLineEdit::initUI()
{
    setFixedHeight(40);
    setFocusPolicy(Qt::ClickFocus);
    setReadOnly(true);
    setObjectName("CustomLineEdit");
    //    setStyleSheet("#CustomLineEdit{border:1px solid #393939;border-radius:6px;padding-left:10px;padding-right:10px;}"
    //                  "#CustomLineEdit:focus{border:1px solid #2eb3ff;}");
}

void CustomLineEdit::keyReleaseEvent(QKeyEvent *event)
{
    QList<int> keycodes;
    int qtkey = 0;

    KLOG_DEBUG(qLcKeybinding) << "Key Release Event: Key:" << event->key() << "Text:" << event->text() << "Modifiers:" << event->modifiers();

    // 忽略无效按键
    if (event->key() == 0 || event->key() == Qt::Key_unknown)
    {
        return;
    }

    // 忽略单独按下Backspace
    if (event->key() == Qt::Key_Backspace && event->modifiers() == Qt::NoModifier)
    {
        return;
    }

    // 忽略数字键盘按键
    if (KeycodeHelper::isKeypad(KeycodeHelper::keycode2Keysym(event->nativeScanCode())))
    {
        KLOG_WARNING(qLcKeybinding) << "Not support keypad key which keycode is" << event->nativeScanCode();
        return;
    }

    // 忽略多媒体按键
    if (ignoreKeys.contains(event->key()))
    {
        KLOG_WARNING(qLcKeybinding) << "Not support media key which keycode is" << event->key();
        return;
    }

    // 处理含shift修饰的快捷键组合，按键不经过shift转化，将原始按键keycode转化为对应的Qt::Key
    if (event->modifiers() & Qt::ShiftModifier)
    {
#if QT_VERSION < QT_VERSION_CHECK(5, 12, 2)
        qtkey = KeycodeTranslator::keycode2QtKey(event);
#else
        qtkey = KeycodeTranslator::keycode2QtKey(event->nativeScanCode());
#endif
        KLOG_INFO(qLcKeybinding) << "convert KeyCode:" << event->nativeScanCode() << "to Qt::Key:" << qtkey;
    }

    // 特殊处理修饰键为主键的情况
    // QKeySequence无法转换Qt::Key_Shift等修饰键对应的Qt::Key
    if (event->key() == Qt::Key_Shift ||
        event->key() == Qt::Key_Alt ||
        event->key() == Qt::Key_Meta ||
        event->key() == Qt::Key_Control)
    {
        keycodes << getModifierKeycodes(event->modifiers());
        auto keyString = keycode2KeyString(keycodes);

        auto key = qtkey ? qtkey : event->key();

        switch (key)
        {
        case Qt::Key_Shift:
            keyString << MODIFIER_KEY_SHIFT;
            break;
        case Qt::Key_Alt:
        {
            /** NOTE: 去重
             * 若按下shift+Alt（Alt作为主键）：
             * keyrelease信息为：event->key()=Qt::Key_Meta，经过上面keycode2QtKey()函数转换后，主键为Qt::Key_Alt;
             *                  event->modifiers()=ShiftModifier|AltModifier|MetaModifier
             * 其中，修饰键包含MetaModifier原因：通过查看xmodmap -pm， Alt 键会被映射为Meta键
             */
            if (event->modifiers() & Qt::ShiftModifier)
            {
                // 去除Meta和Alt修饰键
                auto modifiersFiltered = event->modifiers();
                modifiersFiltered &= ~Qt::MetaModifier;
                modifiersFiltered &= ~Qt::AltModifier;
                KLOG_DEBUG(qLcKeybinding) << "remove Alt and Meta modifier:" << modifiersFiltered;
                keycodes = getModifierKeycodes(modifiersFiltered);
                keyString = keycode2KeyString(keycodes);
            }
            keyString << MODIFIER_KEY_ALT;
            break;
        }
        case Qt::Key_Meta:
            keyString << MODIFIER_KEY_META;
            break;
        case Qt::Key_Control:
            keyString << MODIFIER_KEY_CTRL;
            break;
        default:
            break;
        }

        emit inputKeybinding(keyString);
        return;
    }

    // 特殊处理super键为主键的情况：
    /** NOTE:
     * 1.单独按下super键：qt会识别Qt::Key_Super_L为主键，修饰键为Qt::MetaModifier
     * 2.先按下其他修饰键，再按下super键：qt会识别Qt::Key_Super_L为主键，修饰键为Qt::MetaModifier+其他修饰键
     * super作为主键的情况下，修饰键都会包含Qt::MetaModifier，为了防止后续转化为可读字串时出现两个Super，将Qt::MetaModifier去除
     */
    if (event->key() == Qt::Key_Super_L || event->key() == Qt::Key_Super_R)
    {
        // 添加除Qt::MetaModifier之外的修饰键
        auto modifiersWithoutMeta = event->modifiers();
        modifiersWithoutMeta &= ~Qt::MetaModifier;

        keycodes << getModifierKeycodes(modifiersWithoutMeta);

        auto keyString = keycode2KeyString(keycodes);

        // 添加主键
        keyString.append(event->key() == Qt::Key_Super_L ? "Super_L" : "Super_R");

        emit inputKeybinding(keyString);
        return;
    }

    // 添加修饰键
    keycodes << getModifierKeycodes(event->modifiers());

    // 添加主键
    auto key = qtkey ? qtkey : event->key();
    if (!keycodes.contains(key))
    {
        keycodes.append(key);
    }

    if (keycodes.size() > 0)
    {
        emit inputKeybinding(keycode2KeyString(keycodes));
    }
}

void CustomLineEdit::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        update();
        clear();
    }
    QWidget::mousePressEvent(event);
}

void CustomLineEdit::focusInEvent(QFocusEvent *e)
{
    Q_UNUSED(e);
    update();
    grabKeyboard();
    m_flag = true;
}
void CustomLineEdit::focusOutEvent(QFocusEvent *e)
{
    releaseKeyboard();
    update();
}

QStringList CustomLineEdit::keycode2KeyString(const QList<int> &keycodes)
{
    if (keycodes.isEmpty())
        return QStringList();

    QStringList keyStrings;
    for (auto keycode : keycodes)
    {
        keyStrings << QKeySequence(keycode).toString();
    }
    return keyStrings;
}

QList<int> CustomLineEdit::getModifierKeycodes(Qt::KeyboardModifiers modifiers)
{
    if (modifiers == Qt::NoModifier)
        return QList<int>();

    QList<int> keycodes;

    // 定义修饰键
    static QList<Qt::KeyboardModifier> modifierOrder = {Qt::ControlModifier,
                                                        Qt::AltModifier,
                                                        Qt::ShiftModifier,
                                                        Qt::MetaModifier};

    for (auto mod : modifierOrder)
    {
        if (modifiers & mod)
        {
            keycodes.append(mod);
        }
    }
    return keycodes;
}
