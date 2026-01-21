/**
 * Copyright (c) 2020 ~ 2025 KylinSec Co., Ltd.
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

#include "custom-text-browser.h"
#include <palette.h>
#include <QPainter>
#include <QPainterPath>

using namespace Kiran::Theme;

CustomTextBrowser::CustomTextBrowser(QWidget *parent)
    : QTextBrowser(parent)
{
    viewport()->setAutoFillBackground(false);
}

void CustomTextBrowser::paintEvent(QPaintEvent *event)
{
    QTextBrowser::paintEvent(event);

    // 获取当前主题的边框颜色
    auto kiranPalette = DEFAULT_PALETTE();
    QColor borderColor = kiranPalette->getColor(Palette::ColorGroup::ACTIVE, Palette::ColorRole::BORDER);

    QPainter painter(viewport());
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(QPen(borderColor, 1));

    // 绘制圆角边框
    QRect viewportRect = viewport()->rect();
    QPainterPath path;
    path.addRoundedRect(viewportRect.adjusted(0, 0, -1, -1), 6, 6);
    painter.drawPath(path);
}
