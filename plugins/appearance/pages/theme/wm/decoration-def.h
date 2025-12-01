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
 * Author:     liuxinhao <liuxinhao@kylinsec.com.cn>
 */
#pragma once
#include <KDecoration2/DecoratedClient>
#include <KDecoration2/Decoration>
#include <KDecoration2/DecorationButton>
#include <KDecoration2/DecorationSettings>
#include <KDecoration2/DecorationThemeProvider>
#include <KDecoration2/Private/DecoratedClientPrivate>
#include <KDecoration2/Private/DecorationBridge>
#include <KDecoration2/Private/DecorationSettingsPrivate>

#define DECORATION_PLUGIN_NAME "org.kde.kdecoration2"

namespace Kiran
{
namespace Decoration
{
typedef KDecoration2::ColorRole ColorRole;
typedef KDecoration2::BorderSize BorderSize;
typedef KDecoration2::ColorGroup ColorGroup;
typedef KDecoration2::Decoration Decoration;
typedef KDecoration2::DecoratedClient DecoratedClient;
typedef KDecoration2::DecorationButton DecorationButton;
typedef KDecoration2::DecorationBridge DecorationBridge;
typedef KDecoration2::DecorationSettings DecorationSettings;
typedef KDecoration2::DecorationButtonType DecorationButtonType;
typedef KDecoration2::DecoratedClientPrivate DecoratedClientPrivate;
typedef KDecoration2::DecorationThemeProvider DecorationThemeProvider;
typedef KDecoration2::DecorationThemeMetaData DecorationThemeMetaData;
typedef KDecoration2::DecorationSettingsPrivate DecorationSettingsPrivate;
typedef KDecoration2::ApplicationMenuEnabledDecoratedClientPrivate ApplicationMenuEnabledDecoratedClientPrivate;
}  // namespace Decoration
}  // namespace Kiran