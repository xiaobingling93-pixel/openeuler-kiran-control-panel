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

#include "history-dialog.h"
#include "widgets/custom-plain-text-edit.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>

#include <kiran-color-block.h>
#include <kiran-sidebar-item.h>
#include <kiran-sidebar-widget.h>

#define ITEM_UPGRADE_HISTORY_ROLE Qt::UserRole + 1
HistoryDialog::HistoryDialog(QWidget *parent)
    : KiranTitlebarWindow(parent, Qt::Dialog)
{
    initUI();
}

HistoryDialog::~HistoryDialog()
{
}

void HistoryDialog::initUI()
{
    setTitle(tr("Upgrade History"));
    setButtonHints(TitlebarCloseButtonHint);
    setFixedSize(750, 550);
    setTitlebarColorBlockEnable(true);

    // 主布局
    auto contentLayout = new QHBoxLayout(getWindowContentWidget());
    contentLayout->setContentsMargins(4, 4, 4, 4);
    contentLayout->setSpacing(4);

    // 侧边栏
    auto siderbar = new KiranColorBlock(this);
    contentLayout->addWidget(siderbar);
    siderbar->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    siderbar->setFixedWidth(272);

    auto vLayout = new QVBoxLayout(siderbar);
    vLayout->setSpacing(0);
    vLayout->setContentsMargins(0, 0, 0, 0);

    m_tabList = new KiranSidebarWidget(siderbar);
    m_tabList->viewport()->setAutoFillBackground(false);
    m_tabList->setFrameShape(QFrame::NoFrame);
    m_tabList->setObjectName("tabList");
    m_tabList->setIconSize(QSize(40, 40));
    vLayout->addWidget(m_tabList);

    // 内容区域
    auto historyWidget = new KiranColorBlock(this);
    contentLayout->addWidget(historyWidget);

    auto historyLayout = new QVBoxLayout(historyWidget);
    historyLayout->setSpacing(10);
    historyLayout->setContentsMargins(0, 0, 0, 0);

    // 安装成功的软件包
    m_successWidget = new QWidget(historyWidget);
    auto successLayout = new QVBoxLayout(m_successWidget);
    successLayout->setSpacing(6);
    auto successLabel = new QLabel(tr("Upgraded Packages"), m_successWidget);
    m_successBrowser = new CustomPlainTextEdit(m_successWidget);
    successLayout->addWidget(successLabel);
    successLayout->addWidget(m_successBrowser);
    historyLayout->addWidget(m_successWidget);

    // 安装失败的软件包
    m_failedWidget = new QWidget(historyWidget);
    auto failedLayout = new QVBoxLayout(m_failedWidget);
    failedLayout->setSpacing(6);
    auto failedLabel = new QLabel(tr("Un-upgraded Packages"), m_failedWidget);
    m_failedBrowser = new CustomPlainTextEdit(m_failedWidget);
    failedLayout->addWidget(failedLabel);
    failedLayout->addWidget(m_failedBrowser);
    historyLayout->addWidget(m_failedWidget);

    connect(m_tabList, &KiranSidebarWidget::itemSelectionChanged, this, &HistoryDialog::updatePackageInfo);
}

void HistoryDialog::updatePackageInfo()
{
    QList<QListWidgetItem *> selecteds = m_tabList->selectedItems();
    if (selecteds.size() != 1)
    {
        return;
    }
    QListWidgetItem *item = selecteds.at(0);
    UpgradeHistory history = item->data(ITEM_UPGRADE_HISTORY_ROLE).value<UpgradeHistory>();
    // 获取侧边栏绑定的历史记录数据
    auto successPackages = history.successPackages;
    auto failedPackages = history.failedPackages;
    // 更新至内容区域
    m_successBrowser->clear();
    m_failedBrowser->clear();
    for (const auto &package : successPackages)
    {
        m_successBrowser->appendPlainText(package);
    }
    for (const auto &package : failedPackages)
    {
        m_failedBrowser->appendPlainText(package);
    }
    m_successWidget->setVisible(successPackages.size() > 0);
    m_failedWidget->setVisible(failedPackages.size() > 0);
}

void HistoryDialog::setUpgradeHistory(const QList<UpgradeHistory> &historyList)
{
    // 清空侧边栏更新历史
    m_tabList->clear();
    // 侧边栏添加更新历史
    for (const auto &history : historyList)
    {
        appendHistory(history);
    }
    m_tabList->setCurrentRow(0);
}

KiranSidebarItem *HistoryDialog::createHistoryItem(const UpgradeHistory &history)
{
    QString statusDesc;
    switch (history.result)
    {
    case UPGRADE_RESULT_SUCCESS:
        statusDesc = tr("Success");
        break;
    case UPGRADE_RESULT_FAILED:
        statusDesc = tr("Failed");
        break;
    default:
        statusDesc = tr("Unknown");
        break;
    }
    // 创建 item 时不能指定 parent，否则 insertItem 时会排列到最后
    auto item = new KiranSidebarItem(history.upgradeTime);
    item->setSizeHint(QSize(240, 50));
    item->setStatusDesc(statusDesc,
                        history.result == UPGRADE_RESULT_SUCCESS ? QColor("#5ab940")
                                                                 : QColor("#fa4949"));
    item->setData(ITEM_UPGRADE_HISTORY_ROLE, QVariant::fromValue(history));
    return item;
}
void HistoryDialog::appendHistory(const UpgradeHistory &history)
{
    auto item = createHistoryItem(history);
    m_tabList->addItem(item);
}

void HistoryDialog::prependHistory(const UpgradeHistory &history)
{
    auto item = createHistoryItem(history);
    m_tabList->insertItem(0, item);
}