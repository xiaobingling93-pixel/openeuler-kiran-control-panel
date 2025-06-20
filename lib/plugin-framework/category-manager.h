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

#pragma once

#include <QList>
#include <QObject>

#include "category.h"

class Plugin;

/// @brief 分类管理
/// 管理分类列表以及接受插件发出信号，更新分类下的功能项
class CategoryManager : public QObject
{
    Q_OBJECT
private:
    CategoryManager(QObject* parent = nullptr);

public:
    static CategoryManager* instance();
    ~CategoryManager();

    bool init();
    void dump();

    QList<Category*> getCategorys();
    Category* getCategory(const QString& categoryID);

signals:
    void subItemAdded(const QString& categoryID, const QString& subitemID);
    void subItemDeleted(const QString& categoryID, const QString& subitemID);

private:
    bool loadAllCategory();
    bool loadAllSubItem();
    void connectToPluginsSubItemChanged();

    // 在分类下统一添加和删除功能项的接口，便于统一更新缓存所有的功能项关联关系
    void addSubItemToCategory(Plugin* plugin, KiranControlPanel::SubItemPtr subitem);
    void removeSubItem(const QString& categoryID, Plugin* plugin, const QString& subitemID);

private slots:
    void handlePluginSubItemInfoChanged(const QString& subitemID);
    void handlePluginSubItemChanged();

private:
    static CategoryManager* _instance;
    bool m_isInited = false;
    // 分类有序列表
    QList<Category*> m_categorys;
    // 分类ID对应分类字典
    QMap<QString, Category*> m_categorysMap;
    // 缓存功能项、插件、分类对应关系
    struct SubItemInfoCacheItem
    {
        void* pPlugin;
        QString categoryID;
        QString subItemID;
    };
    QList<SubItemInfoCacheItem> m_subitemInfoCache;
};