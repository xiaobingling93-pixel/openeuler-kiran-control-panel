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

#ifndef APPEARANCEGLOBALINFO_H
#define APPEARANCEGLOBALINFO_H

#include <QObject>
#include "appearance.h"

class AppearanceGlobalInfo : public QObject
{
    Q_OBJECT
public:
    AppearanceGlobalInfo(QObject *parent = nullptr);
    ~AppearanceGlobalInfo();

    static AppearanceGlobalInfo *instance();

    struct ThemeInfo{
        QString name;
        QString path;
    };
    QList<ThemeInfo> getAllThemes(int themeType);

    bool setTheme(int themeType, QString themeName);
    bool getTheme(int type, QString &theme);
    bool getAutoSwitchWindowTheme();
    void enableAutoSwitchWindowTheme();

    QString getDesktopBackground();
    bool setDesktopBackground(QString);

    QString getLockScreenBackground();
    bool setLockScreenBackground(QString);

    bool getFont(int type,QString& fontName,int& fontSize);
    bool setFont(int fontType,const QString& fontInfo);
    bool resetFont(int fontType);

private:
    bool parseFontInfo(const QString& fontInfo,QString& fontFamily,int& fontSize);

signals:
    void themeChanged(int type, const QString &theme_name);
    void fontChanged(int type, const QString &fontFamily,int fontSize);
    void desktopBackgroundChanged(const QString &value) const;
    void lockScreenBackgroundChanged(const QString &value) const;
    void AutoSwitchWindowThemeChanged(bool autoSwitch);

private:
    DBusWrapper::Appearance::AppearanceInterfacePtr m_appearanceInterface;
};

#endif  // APPEARANCEGLOBALINFO_H
