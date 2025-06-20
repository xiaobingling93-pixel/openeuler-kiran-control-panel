/**
 * Copyright (c) 2020 ~ 2024 KylinSec Co., Ltd.
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
#ifndef __ACCOUNT_SUBITEM_H__
#define __ACCOUNT_SUBITEM_H__

#include <QDBusObjectPath>
#include <QObject>

#include "panel-interface.h"
#include "plugin-subitem-interface.h"

class AccountWidget;
class AccountSubItem : public QObject,
                       public KiranControlPanel::PluginSubitemInterface
{
    Q_OBJECT
public:
    AccountSubItem(KiranControlPanel::PanelInterface* interface, QObject* parent = nullptr);
    ~AccountSubItem();

    bool eventFilter(QObject* watched, QEvent* event) override;

    QString getID() override;
    QString getName() override;
    QString getCategory() override;
    QString getDesc() override;
    QString getIcon() override;
    int getWeight() override;

    // 获取自定义搜索关键词
    //  QVector< 显示文本(已翻译)，搜索跳转标识ID >
    QVector<QPair<QString, QString>> getSearchKeys() override;

    QWidget* createWidget() override;
    bool jumpToSearchEntry(const QString& key) override;
    bool haveUnSavedOptions() override;

private:
    void handleUserAdded(const QString& objectPath);
    void handleUserDeleted(const QString& objectPath);

private:
    KiranControlPanel::PanelInterface* m_interface = nullptr;
    AccountWidget* m_accountWidget = nullptr;
};

#endif