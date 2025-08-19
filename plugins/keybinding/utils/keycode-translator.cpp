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

#include "keycode-translator.h"
#include "logging-category.h"

#include <qt5-log-i.h>
#include <QApplication>
#include <QMetaEnum>
#include "keycode-helper.h"

// clang-format off
static const QMap<QString, QString> SpecialKeyMap = {
        {"Up", "↑"},
        {"Left", "←"},
        {"Down", "↓"},
        {"Right", "→"}
};
// clang-format on

KeycodeTranslator::KeycodeTranslator(QObject *parent)
    : QObject(parent)
{
}

QString KeycodeTranslator::keyStrings2ReadableString(const QStringList &keyStrings)
{
    QStringList readableStrings = keyStrings;
    for (auto keyStr : readableStrings)
    {
        // 特殊按键经QMetaEnum翻译之后再经过SpecialKeyMap翻译
        if (SpecialKeyMap.contains(keyStr))
        {
            readableStrings.replace(readableStrings.indexOf(keyStr), SpecialKeyMap.value(keyStr));
        }
    }

    return readableStrings.join("");
}

QString KeycodeTranslator::readable2BackendKeyString(const QString &readableString)
{
    auto keystrings = readableString.split('+');
    for (int i = 0; i < keystrings.count(); i++)
    {
        auto key = keystrings.at(i);
        // special key
        if (SpecialKeyMap.values().contains(key))
        {
            keystrings.replace(i, SpecialKeyMap.key(key));
        }
    }

    return keystrings.join("+");
}

#if QT_VERSION < QT_VERSION_CHECK(5, 12, 2)
#include <QtGui/private/qkeymapper_p.h>
int KeycodeTranslator::keycode2QtKey(const QKeyEvent *keyEvent)
{
    QKeyEvent fakeEvent(*keyEvent);
    fakeEvent.setModifiers(Qt::NoModifier);

    auto keys = QKeyMapper::instance()->possibleKeys(&fakeEvent);

    if (keys.isEmpty())
    {
        return 0;
    }
    else
    {
        return keys.first();
    }
}
#else
#include <QtXkbCommonSupport/private/qxkbcommon_p.h>
int KeycodeTranslator::keycode2QtKey(unsigned long keycode)
{
    auto keysym = KeycodeHelper::keycode2Keysym(keycode);
    auto qtkey = QXkbCommon::keysymToQtKey(keysym, Qt::KeyboardModifier::NoModifier);
    return qtkey;
}
#endif

QString KeycodeTranslator::backendKeyString2Readable(const QString &keyString)
{
    QString readableString;

    if (keyString.isEmpty())
    {
        readableString = tr("None");
    }
    else if (keyString.contains("disable", Qt::CaseInsensitive))
    {
        readableString = tr("disabled");
    }
    else
    {
        auto temp = keyString;
#if (QT_VERSION < QT_VERSION_CHECK(5, 14, 0))
        auto keyList = temp.split("+", QString::SkipEmptyParts);
#else
        auto keyList = temp.split("+", Qt::SkipEmptyParts);
#endif
        for (int i = 0; i < keyList.size(); i++)
        {
            if (SpecialKeyMap.contains(keyList.at(i)))
            {
                keyList.replace(i, SpecialKeyMap.value(keyList.at(i)));
            }
        }
        readableString = keyList.join('+');
    }

    return readableString;
}
