/**
 * Copyright (c) 2020 ~ 2026 KylinSec Co., Ltd.
 * kiran-cc-daemon is licensed under Mulan PSL v2.
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
#include <kiran-titlebar-window.h>

#include <QList>

class KiranSidebarWidget;
class KiranSidebarItem;
class CustomTextBrowser;
class HistoryDialog : public KiranTitlebarWindow
{
    Q_OBJECT
public:
    explicit HistoryDialog(QWidget *parent = nullptr);
    ~HistoryDialog();

    void setUpgradeHistory(const QList<UpgradeHistory> &historyList);
    void prependHistory(const UpgradeHistory &history);

private:
    void initUI();
    void updatePackageInfo();
    KiranSidebarItem *createHistoryItem(const UpgradeHistory &history);
    void appendHistory(const UpgradeHistory &history);

private:
    KiranSidebarWidget *m_tabList;
    QWidget *m_successWidget;
    QWidget *m_failedWidget;
    CustomTextBrowser *m_successBrowser;
    CustomTextBrowser *m_failedBrowser;
};