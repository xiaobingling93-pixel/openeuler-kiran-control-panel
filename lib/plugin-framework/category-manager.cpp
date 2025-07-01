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

#include "category-manager.h"
#include "config.h"
#include "logging-category.h"
#include "plugin-manager.h"

#include <glib.h>
#include <qobject.h>
#include <qt5-log-i.h>
#include <QDir>
#include <QMutex>

#define GROUP_KIRAN_CONTROL_PANEL_CATEGORY "Kiran Control Panel Category"
#define KEY_NAME "Name"
#define KEY_COMMENT "Comment"
#define KEY_ICON "Icon"
#define KEY_CATEGORY "Category"
#define KEY_WEIGHT "Weight"
#define KEY_KEYWORDS "Keywords"

CategoryManager* CategoryManager::_instance = nullptr;

bool parserCategoryDesktop(const QString& desktop, QString& id, QString& name, QString& icon, int& weight)
{
    bool bRes = false;
    GKeyFile* keyFile = nullptr;
    GError* error = nullptr;
    std::string desktopPath = desktop.toStdString();
    gchar *cname = nullptr, *cicon = nullptr, *ccategory = nullptr;

    keyFile = g_key_file_new();
    if (!g_key_file_load_from_file(keyFile,
                                   desktopPath.c_str(),
                                   G_KEY_FILE_KEEP_TRANSLATIONS,
                                   &error))
    {
        KLOG_ERROR(qLcPluginFramework) << "can't parse" << desktopPath.c_str() << (error ? error->message : "");
        goto out;
    }

    cname = g_key_file_get_locale_string(keyFile, GROUP_KIRAN_CONTROL_PANEL_CATEGORY, KEY_NAME, nullptr, &error);
    if (!cname)
    {
        KLOG_ERROR(qLcPluginFramework) << "missing" << GROUP_KIRAN_CONTROL_PANEL_CATEGORY << KEY_NAME << (error ? error->message : "");
        goto out;
    }
    name = cname;
    g_free(cname);

    cicon = g_key_file_get_string(keyFile, GROUP_KIRAN_CONTROL_PANEL_CATEGORY, KEY_ICON, &error);
    if (!cicon)
    {
        KLOG_ERROR(qLcPluginFramework) << "missing" << GROUP_KIRAN_CONTROL_PANEL_CATEGORY << KEY_ICON << (error ? error->message : "");
        goto out;
    }
    icon = cicon;
    g_free(cicon);
    if (!icon.startsWith('/'))
    {
        icon.insert(0, CATEGORY_ICON_DIR "/");
    }

    ccategory = g_key_file_get_string(keyFile, GROUP_KIRAN_CONTROL_PANEL_CATEGORY, KEY_CATEGORY, &error);
    if (!ccategory)
    {
        KLOG_ERROR(qLcPluginFramework) << "missing" << GROUP_KIRAN_CONTROL_PANEL_CATEGORY << KEY_CATEGORY << (error ? error->message : "");
        goto out;
    }
    id = ccategory;
    g_free(ccategory);

    weight = g_key_file_get_int64(keyFile, GROUP_KIRAN_CONTROL_PANEL_CATEGORY, KEY_WEIGHT, &error);
    if (error)
    {
        KLOG_ERROR(qLcPluginFramework) << "missing" << GROUP_KIRAN_CONTROL_PANEL_CATEGORY << KEY_WEIGHT << error->message;
        g_error_free(error);
        error = nullptr;
    }

    bRes = true;
out:
    if (error)
        g_error_free(error);
    if (keyFile)
        g_key_file_free(keyFile);
    return bRes;
}

CategoryManager::CategoryManager(QObject* parent)
    : QObject(parent)
{
}

CategoryManager::~CategoryManager()
{
    qDeleteAll(m_categorys);
}

CategoryManager* CategoryManager::instance()
{
    static QMutex mutex;
    QMutexLocker locker(&mutex);

    if (Q_UNLIKELY(!_instance))
    {
        if (!_instance)
        {
            _instance = new CategoryManager;
        }
    }

    return _instance;
}

/// @brief 初始化，加载所有的分类配置，以及所有的功能项
/// @return 是否初始化成功
bool CategoryManager::init()
{
    if (m_isInited)
    {
        return true;
    }

    loadAllCategory();
    loadAllSubItem();
    connectToPluginsSubItemChanged();
    return true;
}

/// @brief 加载分类目录下的所有分类desktop配置文件，提取相关信息并根据权重进行排序
/// @return 是否加载成功
bool CategoryManager::loadAllCategory()
{
    QList<Category*> categorys;
    QMap<QString, Category*> categorysMap;

    QDir categoryDesktopDir(CATEGORY_DESKTOP_DIR);
    QFileInfoList fileInfoList = categoryDesktopDir.entryInfoList({"*.desktop"}, QDir::Files);
    Q_FOREACH (auto categoryFileInfo, fileInfoList)
    {
        QString path = categoryFileInfo.absoluteFilePath();
        QString id, name, icon;
        int weight;

        if (!parserCategoryDesktop(path, id, name, icon, weight))
        {
            KLOG_ERROR(qLcPluginFramework) << "can't parse category:" << path;
            continue;
        }

        Category* category = new Category();
        category->setID(id);
        category->setName(name);
        category->setIcon(icon);
        category->setWeight(weight);
        categorys.append(category);

        categorysMap[id] = category;
    }

    auto sortFunc = [](Category* category_1,
                       Category* category_2) -> bool
    {
        return category_1->getWeight() > category_2->getWeight();
    };
    std::sort(categorys.begin(), categorys.end(), sortFunc);

    m_categorys.swap(categorys);
    m_categorysMap.swap(categorysMap);

    return true;
}

/// @brief 加载所有插件下的功能项,根据分类插入不同的分类节点下
/// @return 是否加载成功
bool CategoryManager::loadAllSubItem()
{
    auto pluginManager = PluginManager::instance();
    if (!pluginManager->init())
    {
        return false;
    }

    QList<Plugin*> plugins = pluginManager->getPlugins();
    
    QSignalBlocker blocker(this);
    QSet<QString> subitemIDs;
    for (auto plugin : plugins)
    {
        auto subitems = plugin->getSubItems();
        for (auto subitem : subitems)
        {
            addSubItemToCategory(plugin, subitem);
        }
    }

    return true;
}

void CategoryManager::dump()
{
    for (auto category : m_categorys)
    {
        fprintf(stdout, "category -- id: %-20s    name: %-20s\n",
                category->getID().toStdString().c_str(),
                category->getName().toStdString().c_str());
        auto subitemList = category->getSubItems();
        for (auto subitem : subitemList)
        {
            fprintf(stdout, "\tsubitem id: %-15s    name: %-15s ptr:%p \n",
                    subitem->getID().toStdString().c_str(),
                    subitem->getName().toStdString().c_str(),
                    subitem.data());
        }
    }
}

QList<Category*> CategoryManager::getCategorys()
{
    return m_categorys;
}

Category* CategoryManager::getCategory(const QString& categoryID)
{
    if (m_categorysMap.find(categoryID) == m_categorysMap.end())
        return nullptr;
    return m_categorysMap[categoryID];
}

void CategoryManager::connectToPluginsSubItemChanged()
{
    auto pluginManager = PluginManager::instance();
    QList<Plugin*> plugins = pluginManager->getPlugins();
    for (auto plugin : plugins)
    {
        connect(plugin, &Plugin::subItemInfoChanged, this, &CategoryManager::handlePluginSubItemInfoChanged);
        connect(plugin, &Plugin::subItemChanged, this, &CategoryManager::handlePluginSubItemChanged);
    }
}

/// @brief 添加功能项，进入功能项所属的分类之中
/// @param plugin   功能项所在的插件
/// @param subitem  功能项共享指针
void CategoryManager::addSubItemToCategory(Plugin* plugin, KiranControlPanel::SubItemPtr subitem)
{
    QString categoryID = subitem->getCategory();
    QString subitemID = subitem->getID();

    auto categoryIter = m_categorysMap.find(categoryID);
    if (categoryIter == m_categorysMap.end())
    {
        KLOG_WARNING(qLcPluginFramework) << "plugin:" << plugin->getID() << plugin->getName() << "\n"
                                         << "subitem:" << subitem->getID() << subitem->getName() << "\n"
                                         << "can't find category:" << categoryID;
        return;
    }

    SubItemInfoCacheItem cacheItem{plugin, categoryID, subitemID};
    m_subitemInfoCache << cacheItem;

    Category* category = categoryIter.value();
    category->appendSubItem(subitem);
    emit subItemAdded(categoryID, subitemID);
}

void CategoryManager::removeSubItem(const QString& categoryID, Plugin* plugin, const QString& subitemID)
{
    auto category = m_categorysMap[categoryID];
    category->removeSubItem(subitemID);

    for (auto iter = m_subitemInfoCache.begin(); iter != m_subitemInfoCache.end(); iter++)
    {
        if (iter->categoryID == categoryID && iter->pPlugin == plugin && iter->subItemID == subitemID)
        {
            m_subitemInfoCache.erase(iter);
            break;
        }
    }
    emit subItemDeleted(categoryID, subitemID);
}

/**
 * NOTE: 该方法为插件调用触发，主面板处理子功能项目信息发生变化
*/
void CategoryManager::handlePluginSubItemInfoChanged(const QString& subiemID)
{
    Plugin* plugin = qobject_cast<Plugin*>(sender());

    KLOG_DEBUG(qLcPluginFramework) << "plugin" << plugin->getID() << plugin->getName()
                                   << "subitem" << subiemID << "changed!";

    for (auto cacheItem : m_subitemInfoCache)
    {
        if ((plugin == cacheItem.pPlugin) && (cacheItem.subItemID == subiemID))
        {
            QString categoryID = cacheItem.categoryID;
            Category* category = m_categorysMap[categoryID];
            // 发出主分类子功能项信息变化信号，感兴趣类通过该方法
            // 重新加载该主分类下子功能项的信息，例如搜索项
            emit category->subItemInfoChanged(subiemID);
        }
    }
}

/**
 * NOTE: 该方法为插件调用触发，主面板处理子功能变更，更新二级分类
*/
void CategoryManager::handlePluginSubItemChanged()
{
    Plugin* plugin = qobject_cast<Plugin*>(sender());
    auto newSubItems = plugin->getSubItems();
    QList<SubItemInfoCacheItem> oldSubItems;

    KLOG_DEBUG(qLcPluginFramework) << "plugin" << plugin->getID() << plugin->getName() << "subitem changed!";

    // 找到该插件所有的功能项
    for (auto cacheItem : m_subitemInfoCache)
    {
        if (cacheItem.pPlugin == plugin)
        {
            oldSubItems << cacheItem;
        }
    }

    // 比对插件新增/删除的功能项，同步到分类(Category)
    for (auto subitem : newSubItems)
    {
        bool isFind = false;
        for (auto cacheitem : oldSubItems)
        {
            if ((cacheitem.categoryID == subitem->getCategory()) && (cacheitem.subItemID == subitem->getID()))
            {
                isFind = true;
            }
        }
        if (!isFind)
            addSubItemToCategory(plugin, subitem);
    }

    for (auto cacheitem : oldSubItems)
    {
        bool isFind = false;
        for (auto subitem : newSubItems)
        {
            if ((cacheitem.categoryID == subitem->getCategory()) && (cacheitem.subItemID == subitem->getID()))
            {
                isFind = true;
            }
        }
        if (!isFind)
            removeSubItem(cacheitem.categoryID, plugin, cacheitem.subItemID);
    }
}
