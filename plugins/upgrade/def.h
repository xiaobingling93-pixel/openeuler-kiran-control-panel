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

#pragma once

#include <kiran-system-daemon/upgrade-i.h>
#include <QMap>
#include <QObject>
#include <QString>

namespace AdvisoryKindHelper
{
inline QMap<AdvisoryKindFlag, QString> getFlagToStringMap()
{
    static QMap<AdvisoryKindFlag, QString> flagToStringMap = {
        {ADVISORY_KIND_ENHANCEMENT, QT_TRANSLATE_NOOP("AdvisoryKindHelper", "Enhancement")},
        {ADVISORY_KIND_BUGFIX, QT_TRANSLATE_NOOP("AdvisoryKindHelper", "Bugfix")},
        {ADVISORY_KIND_SECURITY, QT_TRANSLATE_NOOP("AdvisoryKindHelper", "Security")},
        {ADVISORY_KIND_NEWPACKAGE, QT_TRANSLATE_NOOP("AdvisoryKindHelper", "Newpackage")},
        {ADVISORY_KIND_UNKNOWN, QT_TRANSLATE_NOOP("AdvisoryKindHelper", "Unknown")}};
    return flagToStringMap;
}
inline AdvisoryKindFlags getDefaultFilterTypes()
{
    return ADVISORY_KIND_SECURITY | ADVISORY_KIND_BUGFIX | ADVISORY_KIND_ENHANCEMENT |
           ADVISORY_KIND_NEWPACKAGE | ADVISORY_KIND_UNKNOWN;
}
}  // namespace AdvisoryKindHelper

// 存储可更新包信息结构体
struct UpgradePkgInfo
{
    // 是否被选中
    bool selected;
    // 包ID
    QString id;
    // 包名称
    QString name;
    // 当前版本
    QString currentVersion;
    // 最新版本
    QString latestVersion;
    // 类型
    QString kinds;
    // 大小
    QString size;
};

enum PackageTableField
{
    PACKAGE_TABLE_FIELD_CHECKBOX,
    PACKAGE_TABLE_FIELD_NAME,
    PACKAGE_TABLE_FIELD_CURRENT_VERSION,
    PACKAGE_TABLE_FIELD_LATEST_VERSION,
    PACKAGE_TABLE_FIELD_KINDS,
    PACKAGE_TABLE_FIELD_SIZE,
    PACKAGE_TABLE_FIELD_LAST
};