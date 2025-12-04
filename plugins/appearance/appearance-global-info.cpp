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

#include "appearance-global-info.h"
#include "logging-category.h"

#include <kiran-log/qt5-log-i.h>
#include <QMutex>
#include <QScopedPointer>

AppearanceGlobalInfo::AppearanceGlobalInfo(QObject *parent)
    : QObject(parent),
      m_appearanceInterface(DBusWrapper::Appearance::interface())
{
    connect(m_appearanceInterface.data(), &AppearanceInterface::ThemeChanged, this,
            [this](int type, const QString &themeName)
            {
                KLOG_DEBUG(qLcAppearance) << "theme changed,"
                                          << "theme type:" << type
                                          << "theme name:" << themeName;
                emit themeChanged(type, themeName);
            });
    connect(m_appearanceInterface.data(), &AppearanceInterface::desktop_backgroundChanged, this,
            [this](const QString &value)
            {
                KLOG_DEBUG(qLcAppearance) << "background changed,"
                                          << "background:" << value;
                emit desktopBackgroundChanged(value);
            });
    connect(m_appearanceInterface.data(), &AppearanceInterface::lock_screen_backgroundChanged, this,
            [this](const QString &value)
            {
                KLOG_DEBUG(qLcAppearance) << "lock screen background changed,"
                                          << "background:" << value;
                emit lockScreenBackgroundChanged(value);
            });
    connect(m_appearanceInterface.data(), &AppearanceInterface::FontChanged, this,
            [this](int type, const QString &fontInfo)
            {
                KLOG_DEBUG(qLcAppearance) << "font changed,"
                                          << "font type:" << type
                                          << "font info:" << fontInfo;
                QString fontFamily;
                int fontSize;
                if( !parseFontInfo(fontInfo,fontFamily,fontSize) )
                {
                    KLOG_WARNING(qLcAppearance) << "check font changed value failed,invald format";
                    return;
                }

                emit fontChanged(type, fontFamily,fontSize);
            });
    connect(m_appearanceInterface.data(), &AppearanceInterface::AutoSwitchWindowThemeChanged, this,
            [this](bool enable)
            {
                KLOG_DEBUG(qLcAppearance) << "auto switch window theme changed,"
                                          << "enable:" << enable;
                emit AutoSwitchWindowThemeChanged(enable);
            });
}

AppearanceGlobalInfo::~AppearanceGlobalInfo()
{
}

AppearanceGlobalInfo *AppearanceGlobalInfo::instance()
{
    static QMutex mutex;
    static QScopedPointer<AppearanceGlobalInfo> pInst;

    if (Q_UNLIKELY(!pInst))
    {
        QMutexLocker locker(&mutex);
        if (pInst.isNull())
        {
            pInst.reset(new AppearanceGlobalInfo);
        }
    }

    return pInst.data();
}

QList<AppearanceGlobalInfo::ThemeInfo> AppearanceGlobalInfo::getAllThemes(int themeType)
{
    QList<AppearanceGlobalInfo::ThemeInfo> themeInfoList;

    QDBusPendingReply<QString> reply = m_appearanceInterface->GetThemes(themeType);
    reply.waitForFinished();
    if (reply.isError() || !reply.isValid() || reply.count() < 1)
    {
        KLOG_ERROR(qLcAppearance) << "get all theme failed,theme type:" << themeType
                                  << "error: " << reply.error().message();
        return themeInfoList;
    }

    auto themesJson = reply.argumentAt(0).toString();
    QJsonParseError jsonError;
    QJsonDocument jsonDocument = QJsonDocument::fromJson(themesJson.toLocal8Bit().data(), &jsonError);
    if (jsonDocument.isNull() || jsonError.error != QJsonParseError::NoError)
    {
        KLOG_ERROR() << "parse theme json failed," << themesJson << jsonError.errorString() << jsonError.error;
        return themeInfoList;
    }

    if (!jsonDocument.isArray())
    {
        KLOG_ERROR() << "parse theme json failed," << themesJson << "isn't array";
        return themeInfoList;
    }

    QJsonArray array = jsonDocument.array();
    if (array.size() < 1)
    {
        return themeInfoList;
    }

    for (int i = 0; i < array.size(); i++)
    {
        QJsonValue value = array.at(i);
        if (value.type() != QJsonValue::Object)
        {
            continue;
        }

        QJsonObject themeInfoObj = value.toObject();
        if (!themeInfoObj.contains("name") || !themeInfoObj.contains("path"))
        {
            KLOG_WARNING() << "parse theme json failed,missing specific key(name/path)";
            continue;
        }
        if (!themeInfoObj["name"].isString() || !themeInfoObj["path"].isString())
        {
            KLOG_WARNING() << "parse theme json failed,wrong key format(name/path)";
            continue;
        }

        ThemeInfo info;
        info.name = themeInfoObj["name"].toString();
        info.path = themeInfoObj["path"].toString();
        themeInfoList.append(info);
    }

    KLOG_DEBUG(qLcAppearance) << "get all themes,theme type:" << themeType << themesJson;
    return themeInfoList;
}

bool AppearanceGlobalInfo::setTheme(int themeType, QString themeName)
{
    QDBusPendingReply<> reply = m_appearanceInterface->SetTheme(themeType, themeName);
    reply.waitForFinished();
    if (reply.isError() || !reply.isValid())
    {
        KLOG_ERROR(qLcAppearance) << "set theme failed,"
                                  << "error:" << reply.error().message();
        return false;
    }

    return true;
}

bool AppearanceGlobalInfo::getTheme(int themeType, QString &theme)
{
    QDBusPendingReply<QString> reply = m_appearanceInterface->GetTheme(themeType);
    reply.waitForFinished();
    if (reply.isError() || !reply.isValid())
    {
        KLOG_DEBUG(qLcAppearance) << "get theme failed!"
                                  << "theme type:" << themeType
                                  << "error:" << reply.error().message();
        return false;
    }
    else if (reply.count() < 1)
    {
        KLOG_WARNING(qLcAppearance) << "get theme failed,reply count is 0!";
        return false;
    }

    theme = reply.argumentAt(0).toString();
    KLOG_DEBUG(qLcAppearance) << "get theme,theme type:" << themeType << theme;

    return true;
}

bool AppearanceGlobalInfo::getAutoSwitchWindowTheme()
{
    return m_appearanceInterface->autoSwitchWindowTheme();
}

void AppearanceGlobalInfo::enableAutoSwitchWindowTheme()
{
    auto reply = m_appearanceInterface->EnableAutoSwitchWindowTheme();
    reply.waitForFinished();
}

QString AppearanceGlobalInfo::getDesktopBackground()
{
    return m_appearanceInterface->desktop_background();
}

bool AppearanceGlobalInfo::setDesktopBackground(QString path)
{
    QDBusPendingReply<> reply = m_appearanceInterface->SetDesktopBackground(path);
    reply.waitForFinished();
    if (reply.isError() || !reply.isValid())
    {
        KLOG_ERROR(qLcAppearance) << "set desktop background failed,error:"
                                  << reply.error().message();
        return false;
    }

    return true;
}

QString AppearanceGlobalInfo::getLockScreenBackground()
{
    return m_appearanceInterface->lock_screen_background();
}

bool AppearanceGlobalInfo::setLockScreenBackground(QString path)
{
    QDBusPendingReply<> reply = m_appearanceInterface->SetLockScreenBackground(path);
    reply.waitForFinished();
    if (reply.isError() || !reply.isValid())
    {
        KLOG_ERROR(qLcAppearance) << "set lock screen background failed, error:"
                                  << reply.error().message();
        return false;
    }

    return true;
}

bool AppearanceGlobalInfo::getFont(int type, QString &fontName, int &fontSize)
{
    QDBusPendingReply<QString> getFontReply = m_appearanceInterface->GetFont(type);
    getFontReply.waitForFinished();
    if (getFontReply.isError() || !getFontReply.isValid())
    {
        KLOG_DEBUG(qLcAppearance) << "get font failed,error: "
                                  << getFontReply.error().message();
        return false;
    }
    else if (getFontReply.count() < 1)
    {
        KLOG_WARNING(qLcAppearance) << "get font failed,reply count is 0!";
        return false;
    }

    auto fontValue = getFontReply.argumentAt(0).toString();
    KLOG_DEBUG(qLcAppearance) << "get font,font type:" << type << "font info:" << fontValue;

    if( !parseFontInfo(fontValue,fontName,fontSize) )
    {
        KLOG_WARNING(qLcAppearance) << "parse font value failed,invalid format:" << fontValue;
        return false;
    }

    return true;
}

bool AppearanceGlobalInfo::setFont(int fontType, const QString &fontInfo)
{
    KLOG_DEBUG(qLcAppearance) << "set font,font type:" << fontType << fontInfo;

    QDBusPendingReply<> reply = m_appearanceInterface->SetFont(fontType, fontInfo);
    reply.waitForFinished();
    if (reply.isError() || !reply.isValid())
    {
        KLOG_WARNING(qLcAppearance) << "set font failed,font type:" << fontType
                                    << "error:" << reply.error().message();
        return false;
    }

    return true;
}

bool AppearanceGlobalInfo::resetFont(int fontType)
{
    KLOG_DEBUG(qLcAppearance) << "reset font,font type:" << fontType;

    QDBusPendingReply<> reply = m_appearanceInterface->ResetFont(fontType);
    reply.waitForFinished();
    if (reply.isError() || !reply.isValid())
    {
        KLOG_WARNING(qLcAppearance) << "reset font failed,font type:" << fontType
                                    << "error:" << reply.error().message();
        return false;
    }

    return true;
}

bool AppearanceGlobalInfo::parseFontInfo(const QString &fontInfo, QString &fontFamily, int &fontSize)
{
#if (QT_VERSION < QT_VERSION_CHECK(5, 14, 0))
    auto list = fontInfo.split(" ", QString::SkipEmptyParts);
#else
    auto list = fontInfo.split(" ", Qt::SkipEmptyParts);
#endif
    if (list.isEmpty())
    {
        return false;
    }

    auto sizeValue = list.takeLast();
    fontSize = sizeValue.toInt();

    auto nameValue = list.join(" ");
    fontFamily = nameValue;

    return true;
}
