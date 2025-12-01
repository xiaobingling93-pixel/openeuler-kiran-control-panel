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
#include "preview-settings.h"

namespace Kiran
{
namespace Decoration
{
static QVector<DecorationButtonType> s_decorationButtonsLeft = {
    DecorationButtonType::Menu, DecorationButtonType::ApplicationMenu,
    DecorationButtonType::OnAllDesktops};

static QVector<DecorationButtonType> s_decorationButtonsRight = {
    DecorationButtonType::ContextHelp, DecorationButtonType::Minimize,
    DecorationButtonType::Maximize, DecorationButtonType::Close};

PreviewSettings::PreviewSettings(DecorationSettings *parent)
    : DecorationSettingsPrivate(parent),
      m_alphaChannelSupported(true),
      m_onAllDesktopsAvailable(true),
      m_closeOnDoubleClickOnMenu(false),
      m_borderSize(BorderSize::None),
      m_decorationButtonsLeft(s_decorationButtonsLeft),
      m_decorationButtonsRight(s_decorationButtonsRight)
{
}

PreviewSettings::~PreviewSettings() {}

bool PreviewSettings::isAlphaChannelSupported() const
{
    return m_alphaChannelSupported;
}

bool PreviewSettings::isOnAllDesktopsAvailable() const
{
    return m_onAllDesktopsAvailable;
}

bool PreviewSettings::isCloseOnDoubleClickOnMenu() const
{
    return m_closeOnDoubleClickOnMenu;
}

QVector<DecorationButtonType> PreviewSettings::decorationButtonsLeft() const
{
    return m_decorationButtonsLeft;
}

QVector<DecorationButtonType> PreviewSettings::decorationButtonsRight() const
{
    return m_decorationButtonsRight;
}

BorderSize PreviewSettings::borderSize() const
{
    return m_borderSize;
}
}  // namespace Decoration
}  // namespace Kiran