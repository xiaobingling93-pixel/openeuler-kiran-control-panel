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

#ifndef KIRAN_CONTROL_PANEL_KEYCODE_TRANSLATOR_H
#define KIRAN_CONTROL_PANEL_KEYCODE_TRANSLATOR_H

#include <QKeyEvent>
#include <QObject>

class KeycodeTranslator : public QObject
{
    Q_OBJECT
private:
    KeycodeTranslator(QObject* parent);

public:
    static QString keyStrings2ReadableString(const QStringList& keyStrings);
    static QString readable2BackendKeyString(const QString& readableString);
    static QString backendKeyString2Readable(const QString& keyString);

#if QT_VERSION < QT_VERSION_CHECK(5, 12, 2)
    static int keycode2QtKey(const QKeyEvent* keyEvent);
#else
    static int keycode2QtKey(unsigned long keycode);
#endif
};

#endif  // KIRAN_CONTROL_PANEL_KEYCODE_TRANSLATOR_H
