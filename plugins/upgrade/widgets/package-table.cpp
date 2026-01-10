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

#include "package-table.h"
#include <kiran-log/qt5-log-i.h>
#include <palette.h>

#include <QApplication>
#include <QCoreApplication>
#include <QHeaderView>
#include <QMouseEvent>
#include <QPainterPath>
#include <QStyle>
#include <QStyleOptionButton>
#include <QToolTip>
#include "custom-header-view.h"
#include "logging-category.h"

using namespace Kiran::Theme;

PackageDelegate::PackageDelegate(QObject *parent) : QStyledItemDelegate(parent)
{
}

void PackageDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    auto widget = option.widget;
    auto style = widget ? widget->style() : QApplication::style();

    if (index.column() == PackageTableField::PACKAGE_TABLE_FIELD_CHECKBOX)
    {
        bool checked = index.data(Qt::EditRole).toBool();
        QStyleOptionButton checkboxOption;

        // 计算 checkbox 的矩形区域，使其居中显示
        int indicatorSize = style->pixelMetric(QStyle::PM_IndicatorWidth, &option, widget);
        QRect checkboxRect = QRect(0, 0, indicatorSize, indicatorSize);
        checkboxRect.moveCenter(option.rect.center());
        const_cast<PackageDelegate *>(this)->m_checkboxRects[index] = checkboxRect;
        checkboxOption.rect = checkboxRect;

        checkboxOption.state = QStyle::State_Enabled;

        // 根据选中状态设置样式
        if (checked)
        {
            checkboxOption.state |= QStyle::State_On;
        }
        else
        {
            checkboxOption.state |= QStyle::State_Off;
        }

        // 如果鼠标悬停，添加悬停状态
        if (option.state & QStyle::State_MouseOver)
        {
            checkboxOption.state |= QStyle::State_MouseOver;
        }

        // 如果项被选中，添加选中状态
        if (option.state & QStyle::State_Selected)
        {
            checkboxOption.state |= QStyle::State_Selected;
        }

        style->drawPrimitive(QStyle::PE_IndicatorCheckBox, &checkboxOption, painter, widget);
    }
    else
    {
        QStyleOptionViewItem viewOption(option);
        initStyleOption(&viewOption, index);
        // 设置文本不换行
        viewOption.features &= ~QStyleOptionViewItem::WrapText;
        QStyledItemDelegate::paint(painter, viewOption, index);
    }
}

bool PackageDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    auto decorationRect = option.rect;
    auto mouseEvent = static_cast<QMouseEvent *>(event);

    if (event->type() == QEvent::MouseButtonPress &&
        decorationRect.contains(mouseEvent->pos()) &&
        index.column() == PackageTableField::PACKAGE_TABLE_FIELD_CHECKBOX &&
        m_checkboxRects.contains(index))
    {
        auto value = model->data(index, Qt::EditRole).toBool();
        model->setData(index, !value, Qt::EditRole);
    }
    return QStyledItemDelegate::editorEvent(event, model, option, index);
}

PackageFilterModel::PackageFilterModel(QObject *parent)
    : QSortFilterProxyModel(parent),
      m_filterTypes(AdvisoryKindHelper::getDefaultFilterTypes())  // 默认全选
{
}

bool PackageFilterModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    // 如果没有任何类型被选中，显示所有
    if (m_filterTypes == 0)
    {
        return true;
    }

    auto index = sourceModel()->index(sourceRow, PackageTableField::PACKAGE_TABLE_FIELD_KINDS, sourceParent);
    auto kinds = sourceModel()->data(index).toString();

    auto flagToStringMap = AdvisoryKindHelper::getFlagToStringMap();
    // 检查类型是否匹配
    for (auto it = flagToStringMap.begin(); it != flagToStringMap.end(); ++it)
    {
        AdvisoryKindFlag flag = it.key();
        if ((m_filterTypes & flag) != 0)
        {
            if (kinds.contains(QCoreApplication::translate("AdvisoryKindHelper", it.value().toLatin1().data()), Qt::CaseInsensitive))
            {
                return true;
            }
        }
    }

    return false;
}

void PackageFilterModel::setFilterTypes(AdvisoryKindFlags filterTypes)
{
    if (m_filterTypes != filterTypes)
    {
        m_filterTypes = filterTypes;
        invalidateFilter();
    }
}

PackageModel::PackageModel(QObject *parent)
{
}

int PackageModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_upgradePkgsInfo.size();
}

int PackageModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return PackageTableField::PACKAGE_TABLE_FIELD_LAST;
}

QVariant PackageModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    auto row = index.row();
    auto column = index.column();

    if (row >= m_upgradePkgsInfo.size() || column >= PackageTableField::PACKAGE_TABLE_FIELD_LAST)
    {
        KLOG_DEBUG(qLcUpgrade) << "The index exceeds range limit.";
        return QVariant();
    }

    auto pkgInfo = m_upgradePkgsInfo[row];

    switch (role)
    {
    case Qt::DisplayRole:
    {
        switch (column)
        {
        case PackageTableField::PACKAGE_TABLE_FIELD_NAME:
            return pkgInfo.name;
        case PackageTableField::PACKAGE_TABLE_FIELD_CURRENT_VERSION:
            return pkgInfo.currentVersion;
        case PackageTableField::PACKAGE_TABLE_FIELD_LATEST_VERSION:
            return pkgInfo.latestVersion;
        case PackageTableField::PACKAGE_TABLE_FIELD_KINDS:
            return pkgInfo.kinds;
        case PackageTableField::PACKAGE_TABLE_FIELD_SIZE:
            return pkgInfo.size;
        default:
            break;
        }
    }
    case Qt::EditRole:
    {
        switch (column)
        {
        case PackageTableField::PACKAGE_TABLE_FIELD_CHECKBOX:
            return pkgInfo.selected;
        default:
            break;
        }
    }
    default:
        break;
    }

    return QVariant();
}

QVariant PackageModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal)
        return QVariant();

    switch (role)
    {
    case Qt::DisplayRole:
    {
        switch (section)
        {
        case PackageTableField::PACKAGE_TABLE_FIELD_NAME:
            return tr("Package Name");
        case PackageTableField::PACKAGE_TABLE_FIELD_CURRENT_VERSION:
            return tr("Current Version");
        case PackageTableField::PACKAGE_TABLE_FIELD_LATEST_VERSION:
            return tr("Latest Version");
        case PackageTableField::PACKAGE_TABLE_FIELD_KINDS:
            return tr("Kinds");
        case PackageTableField::PACKAGE_TABLE_FIELD_SIZE:
            return tr("Size");
        default:
            break;
        }
    }
    case Qt::EditRole:
    {
        switch (section)
        {
        case PackageTableField::PACKAGE_TABLE_FIELD_CHECKBOX:
            return QVariant();
        default:
            break;
        }
    }
    default:
        break;
    }

    return QVariant();
}

bool PackageModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.column() != PackageTableField::PACKAGE_TABLE_FIELD_CHECKBOX)
    {
        return false;
    }

    m_upgradePkgsInfo[index.row()].selected = value.toBool();
    emit dataChanged(index, index);
    return true;
}

Qt::ItemFlags PackageModel::flags(const QModelIndex &index) const
{
    if (index.column() == PackageTableField::PACKAGE_TABLE_FIELD_CHECKBOX)
    {
        return Qt::ItemFlag::ItemIsEnabled;
    }
    return Qt::ItemFlag::NoItemFlags;
}

QList<UpgradePkgInfo> PackageModel::getUpgradePkgsInfo()
{
    return m_upgradePkgsInfo;
}

void PackageModel::updateRecord(QList<UpgradePkgInfo> upgradePkgsInfo)
{
    beginResetModel();
    m_upgradePkgsInfo = upgradePkgsInfo;
    endResetModel();
}

PackageTable::PackageTable(QWidget *parent)
{
    viewport()->setAutoFillBackground(false);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setMouseTracking(true);

    // 设置Model
    m_model = new PackageModel(this);

    // 设置自定义表头
    m_headerView = new CustomHeaderView(this);
    setHorizontalHeader(m_headerView);

    m_filterProxy = new PackageFilterModel(this);
    m_filterProxy->setSourceModel(qobject_cast<QAbstractItemModel *>(m_model));
    setModel(m_filterProxy);

    // 设置Delegate
    m_delegate = new PackageDelegate(this);
    setItemDelegate(m_delegate);

    // 连接表头信号
    connect(m_headerView, &CustomHeaderView::toggled, this, &PackageTable::checkedAllItem);
    connect(m_headerView, &CustomHeaderView::filterTypesChanged, [this](AdvisoryKindFlags filterTypes)
            { m_filterProxy->setFilterTypes(filterTypes); });

    // 在代理模型更新时，更新表头选中状态,发送选择变化信号
    connect(m_model, &PackageModel::dataChanged, this, [this](const QModelIndex &topLeft, const QModelIndex &bottomRight)
            {
                if (topLeft.column() > PackageTableField::PACKAGE_TABLE_FIELD_CHECKBOX ||
                    bottomRight.column() < PackageTableField::PACKAGE_TABLE_FIELD_CHECKBOX)
                    return;

                updateHeaderState();
                emit packageSelectedChanged(getSelectedUpgradePkgsInfo().size(), m_model->rowCount());
            });

    connect(m_filterProxy, &PackageFilterModel::modelReset, this, [this]()
            { m_headerView->setCheckState(Qt::Unchecked); });

    // 设置水平行表头
    m_headerView->resizeSection(PackageTableField::PACKAGE_TABLE_FIELD_CHECKBOX, 50);
    m_headerView->resizeSection(PackageTableField::PACKAGE_TABLE_FIELD_NAME, 80);
    m_headerView->resizeSection(PackageTableField::PACKAGE_TABLE_FIELD_CURRENT_VERSION, 80);
    m_headerView->resizeSection(PackageTableField::PACKAGE_TABLE_FIELD_LATEST_VERSION, 80);
    m_headerView->resizeSection(PackageTableField::PACKAGE_TABLE_FIELD_KINDS, 80);
    m_headerView->resizeSection(PackageTableField::PACKAGE_TABLE_FIELD_SIZE, 80);
    m_headerView->setSectionResizeMode(PackageTableField::PACKAGE_TABLE_FIELD_CHECKBOX, QHeaderView::Fixed);
    m_headerView->setStretchLastSection(true);
    m_headerView->setSectionsMovable(false);
    m_headerView->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_headerView->setFixedHeight(40);

    // 设置表的其他属性
    setSelectionMode(QAbstractItemView::NoSelection);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setFocusPolicy(Qt::NoFocus);
    setShowGrid(false);

    // 隐藏列表头
    this->verticalHeader()->setVisible(false);
    this->verticalHeader()->setDefaultSectionSize(60);

    connect(this, &PackageTable::entered, this, &PackageTable::mouseEnter);
}

void PackageTable::searchTextChanged(const QString &text)
{
    KLOG_DEBUG(qLcUpgrade) << "The search text is change to " << text;

    m_filterProxy->setFilterFixedString(text);
}

void PackageTable::setUpgradePkgsInfo(QList<UpgradePkgInfo> upgradePkgsInfo)
{
    m_model->updateRecord(upgradePkgsInfo);
}

QList<UpgradePkgInfo> PackageTable::getUpgradePkgsInfo()
{
    return m_model->getUpgradePkgsInfo();
}

QList<UpgradePkgInfo> PackageTable::getSelectedUpgradePkgsInfo()
{
    QList<UpgradePkgInfo> selectedUpgradePkgsInfo;
    for (int i = 0; i < m_filterProxy->rowCount(); i++)
    {
        auto proxyIndex = m_filterProxy->index(i, PackageTableField::PACKAGE_TABLE_FIELD_CHECKBOX);
        auto isSelected = m_filterProxy->data(proxyIndex, Qt::EditRole).toBool();
        if (isSelected)
        {
            auto sourceIndex = m_filterProxy->mapToSource(proxyIndex);
            selectedUpgradePkgsInfo.append(m_model->getUpgradePkgsInfo().at(sourceIndex.row()));
        }
    }
    return selectedUpgradePkgsInfo;
}

void PackageTable::clearTable()
{
    m_model->removeRows(0, m_model->rowCount());
}

void PackageTable::mouseEnter(const QModelIndex &index)
{
    if (index.column() != PackageTableField::PACKAGE_TABLE_FIELD_NAME &&
        index.column() != PackageTableField::PACKAGE_TABLE_FIELD_CURRENT_VERSION &&
        index.column() != PackageTableField::PACKAGE_TABLE_FIELD_LATEST_VERSION)
    {
        return;
    }
    auto mod = selectionModel()->model()->data(index);
    QToolTip::showText(QCursor::pos(), mod.toString(), this, rect(), 2000);
}
void PackageTable::checkedAllItem(Qt::CheckState checkState)
{
    for (int i = 0; i < m_filterProxy->rowCount(); i++)
    {
        auto proxyIndex = m_filterProxy->index(i, PackageTableField::PACKAGE_TABLE_FIELD_CHECKBOX);
        auto sourceIndex = m_filterProxy->mapToSource(proxyIndex);
        m_model->setData(sourceIndex, checkState == Qt::Checked, Qt::EditRole);
    }
}

void PackageTable::updateHeaderState()
{
    int checkedCount = 0;
    int totalCount = m_filterProxy->rowCount();

    for (int i = 0; i < totalCount; i++)
    {
        auto proxyIndex = m_filterProxy->index(i, PackageTableField::PACKAGE_TABLE_FIELD_CHECKBOX);
        if (m_filterProxy->data(proxyIndex, Qt::EditRole).toBool())
        {
            checkedCount++;
        }
    }

    Qt::CheckState state;
    if (checkedCount == 0)
        state = Qt::Unchecked;
    else if (checkedCount == totalCount)
        state = Qt::Checked;
    else
        state = Qt::PartiallyChecked;

    m_headerView->setCheckState(state);
}

void PackageTable::paintEvent(QPaintEvent *event)
{
    auto kiranPalette = DEFAULT_PALETTE();
    QColor borderColor = kiranPalette->getColor(Palette::ColorGroup::ACTIVE, Palette::ColorRole::BORDER);

    QPainter painter(viewport());
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(QPen(borderColor, 1));

    // 绘制外边框
    QRect viewportRect = viewport()->rect();
    painter.drawRect(viewportRect.adjusted(0, 0, -1, -1));
    QTableView::paintEvent(event);
}