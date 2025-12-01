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

#ifndef ICONTHEMES_H
#define ICONTHEMES_H

#include <QList>
#include <QMap>
#include <QWidget>

// 图标主题及对应翻译
const QMap<QString, QString> iconThemeWhiteList = {
    {"Spring", QObject::tr("Spring")},
    {"Summer", QObject::tr("Summer")}};

namespace Ui
{
class IconThemePage;
}

class ExclusionGroup;
class ThemePreview;
class IconThemePage : public QWidget
{
    Q_OBJECT
public:
    explicit IconThemePage(QWidget* parent = 0);
    ~IconThemePage();

    void updateCurrentTheme(QString newIconTheme);

signals:
    void requestReturn();

private slots:
    void onCurrentItemChanged();

private:
    void init();
    void initUI();
    void loadIconThemes();
    ThemePreview* createPreviewWidget(const QString& themeName,
                                            const QList<QPixmap> pixmaps,
                                            bool selected = false);

private:
    static const QStringList m_fallbackIcons;
    static const QMap<QString, QStringList> m_specifyIcons;
    Ui::IconThemePage* ui;
    QStringList m_iconThemes;
    QStringList m_iconThemesPath;
    QString m_currentIconTheme;
    ExclusionGroup* m_exclusionGroup;
};

#endif  // ICONTHEMES_H
