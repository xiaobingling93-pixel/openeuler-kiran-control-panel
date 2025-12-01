/**
 * Copyright (c) 2020 ~ 2025 KylinSec Co., Ltd.
 * kwindecoration-preview is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 *
 * Author:     liuxinhao <liuxinhao@kylinsec.com.cn>
 */
#pragma once
#include <QObject>
#include "decoration-def.h"

namespace Kiran
{
namespace Decoration
{
// 预览设置项
// 目前仅提供静态配置，不允许修改
class PreviewSettings : public QObject,
                        public DecorationSettingsPrivate
{
    Q_OBJECT
public:
    explicit PreviewSettings(DecorationSettings *parent);
    ~PreviewSettings() override;
    bool isAlphaChannelSupported() const override;
    bool isOnAllDesktopsAvailable() const override;
    bool isCloseOnDoubleClickOnMenu() const override;
    QVector<DecorationButtonType> decorationButtonsLeft() const override;
    QVector<DecorationButtonType> decorationButtonsRight() const override;
    BorderSize borderSize() const override;

private:
    bool m_alphaChannelSupported;
    bool m_onAllDesktopsAvailable;
    bool m_closeOnDoubleClickOnMenu;
    BorderSize m_borderSize;
    QVector<DecorationButtonType> m_decorationButtonsLeft;
    QVector<DecorationButtonType> m_decorationButtonsRight;
};
}  // namespace Decoration
}  // namespace Kiran