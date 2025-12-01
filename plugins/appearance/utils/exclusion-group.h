/**
 * Copyright (c) 2020 ~ 2023 KylinSec Co., Ltd.
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

#include <QSet>
#include <QWidget>

// 此处为了避免菱形继承，互斥项从QWidget中派生
class ExclusionWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ExclusionWidget(QWidget* parent = nullptr);
    ~ExclusionWidget();

    virtual QString getID() const
    {
        return QString();
    }

    virtual void setSelected(bool selected)
    {
        if (m_selected == selected)
        {
            return;
        }

        m_selected = selected;
        emit selectedStatusChanged(m_selected);
    };

    virtual bool getSelected() const
    {
        return m_selected;
    };

signals:
    void selectedStatusChanged(bool selected);

private:
    bool m_selected = false;
};

class ExclusionGroup : public QObject
{
    Q_OBJECT
public:
    explicit ExclusionGroup(QObject* parent = nullptr);
    ~ExclusionGroup();

    void setCurrent(const QString& id);
    void setCurrent(ExclusionWidget* widget);

    ExclusionWidget* getCurrent() const;
    QString getCurrentID() const;
    QSet<QString> getExclusionItemIDs() const;

    void addExclusionItem(ExclusionWidget* widget);
    void removeExclusionItem(ExclusionWidget* widget);

signals:
    void currentItemChanged();

private slots:
    void onItemSelectedChanged(bool selected);

private:
    QSet<ExclusionWidget*> m_exclusionItems;
    ExclusionWidget* m_current = nullptr;
};
