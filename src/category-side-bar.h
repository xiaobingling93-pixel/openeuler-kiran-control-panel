/**
 * Copyright (c) 2020 ~ 2022 KylinSec Co., Ltd.
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

#ifndef KCP_CATEGORY_CATEGORY_WIDGET_H
#define KCP_CATEGORY_CATEGORY_WIDGET_H

#include <kiran-color-block.h>
#include <QPropertyAnimation>

class QButtonGroup;
class QAbstractButton;
class QVBoxLayout;
class QFrame;
class QGraphicsDropShadowEffect;
class CategoryItem;
class CategorySideBar : public KiranColorBlock
{
    Q_OBJECT
public:
    CategorySideBar(QWidget* parent = nullptr);
    ~CategorySideBar();

    QString getCurrentCateogryID();
    void setCurrentCategoryID(const QString& categoryID);

signals:
    void currentCategoryIndexChanged(const QString& prev, const QString& cur);

private:
    void init();
    void loadCategories();

private slots:
    void handleCategoryItemToggled(QAbstractButton* btn, bool checked);

protected:
    bool event(QEvent* event) override;

private:
    QButtonGroup* m_categoryBtnGroup;
    QWidget* m_contentWidget = nullptr;
    QVBoxLayout* m_contentLayout = nullptr;

    QString m_curCategoryID;
    bool m_isExpaned = false;

    /// @brief QMap<Category ID,Categorys Sidebar Item>
    QMap<QString, CategoryItem*> m_categorysIDMap;
};

#endif  // KCP_CATEGORY_CATEGORY_WIDGET_H
