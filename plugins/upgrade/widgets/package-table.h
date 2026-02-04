/**
 * Copyright (c) 2020 ~ 2025 KylinSec Co., Ltd.
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

#include <kiran-system-daemon/upgrade-i.h>
#include <QAbstractItemModel>
#include <QAbstractTableModel>
#include <QMap>
#include <QModelIndex>
#include <QPaintEvent>
#include <QPainter>
#include <QRect>
#include <QResizeEvent>
#include <QShowEvent>
#include <QSortFilterProxyModel>
#include <QStyleOptionViewItem>
#include <QStyledItemDelegate>
#include <QTableView>

#include "def.h"

class CustomHeaderView;

//代理
class PackageDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    PackageDelegate(QObject *parent = nullptr);
    virtual ~PackageDelegate(){};

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override;

private:
    QMap<QModelIndex, QRect> m_checkboxRects;
};

//过滤模型
class PackageFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit PackageFilterModel(QObject *parent = nullptr);
    virtual ~PackageFilterModel(){};

    void setFilterTypes(AdvisoryKindFlags filterTypes);
    AdvisoryKindFlags filterTypes() const { return m_filterTypes; }

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;

private:
    AdvisoryKindFlags m_filterTypes;  // 使用位标志存储选中的类型
};

//数据模型
class PackageModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit PackageModel(QObject *parent = nullptr);
    virtual ~PackageModel(){};

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    QList<UpgradePkgInfo> getUpgradePkgsInfo();
    void updateRecord(QList<UpgradePkgInfo> upgradePkgsInfo);

private:
    QList<UpgradePkgInfo> m_upgradePkgsInfo;
};

//表格
class PackageTable : public QTableView
{
    Q_OBJECT
public:
    explicit PackageTable(QWidget *parent = nullptr);
    virtual ~PackageTable(){};

    PackageFilterModel *getFilterProxy() { return m_filterProxy; };
    void searchTextChanged(const QString &text);

    void setUpgradePkgsInfo(QList<UpgradePkgInfo> upgradePkgsInfo);
    QList<UpgradePkgInfo> getUpgradePkgsInfo();

    QList<UpgradePkgInfo> getSelectedUpgradePkgsInfo();

    void clearTable();

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void showEvent(QShowEvent *event) override;

private:
    void updateColumnWidths();

private slots:
    void mouseEnter(const QModelIndex &index);
    void checkedAllItem(Qt::CheckState checkState);
    void updateHeaderState();

signals:
    void packageSelectedChanged(int selectedCount, int totalCount);

private:
    PackageFilterModel *m_filterProxy;
    PackageModel *m_model;
    PackageDelegate *m_delegate;
    CustomHeaderView *m_headerView;
};