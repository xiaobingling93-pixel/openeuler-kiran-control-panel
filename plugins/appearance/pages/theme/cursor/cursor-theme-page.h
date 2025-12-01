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
#pragma once

#include <QMap>
#include <QWidget>

class ExclusionGroup;
class QVBoxLayout;
class ThemePreview;
class CursorThemePage : public QWidget
{
    Q_OBJECT
public:
    explicit CursorThemePage(QWidget* parent = 0);
    ~CursorThemePage();

    void updateCurrentTheme(QString newCursorTheme);

signals:
    void requestReturn();

private:
    void init();
    void initUI();
    void loadCurosrThemes();
    ThemePreview* createPreviewWidget(const QString& themeName,
                                            const QList<QPixmap> pixmaps,
                                            bool selected = false);
private slots:
    void onCurrentItemChanged();

private:
    ExclusionGroup* m_exclusionGroup;
    QVBoxLayout* m_cursorVlayout;
    QString m_currentCursorTheme;
    QMap<QString, QString> m_cursorThemes;
};