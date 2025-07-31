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

#include "keycode-helper.h"

#include <X11/Xlib.h>
#include <X11/XKBlib.h>

namespace KeycodeHelper
{
unsigned long keycode2Keysym(unsigned long keycode)
{
    KeySym keysym = NoSymbol;
    Display *display = QX11Info::display();
    if (display == nullptr)
    {
        KLOG_WARNING(qLcKeybinding) << "can't open display!";
        return keysym;
    }

    keysym = XkbKeycodeToKeysym(display, keycode, 0, 0);
    if (keysym != NoSymbol)
    {
        KLOG_INFO(qLcKeybinding) << "convert KeyCode:" << keycode << "to KeySym:" << keysym;
    }
    else
    {
        KLOG_INFO(qLcKeybinding) << "no corresponding" << keycode << "KeySym found.";
    }
    return keysym;
}
}  // namespace KeycodeHelper
