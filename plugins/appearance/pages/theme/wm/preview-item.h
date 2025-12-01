/**
 * Copyright (c) 2020 ~ 2025 KylinSec Co., Ltd.
 * kwindecoration-preview is licensed under Mulan PSL v2.
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
#include <QRectF>
#include <QSharedPointer>
#include <QWidget>

namespace KDecoration2
{
class Decoration;
class DecorationSettings;
}  // namespace KDecoration2

class QPainter;

namespace Kiran
{
namespace Decoration
{
class PreviewBridge;
class PreviewSettings;
class PreviewClient;
class PreviewItem : public QWidget
{
    Q_OBJECT
public:
    explicit PreviewItem(PreviewBridge *bridge, QWidget *parent);
    ~PreviewItem() override;

    /**
     * @brief
     * 设置锚定信息
     * 归一化的矩形，x, y, width, height 都是 0.0-1.0 的比例
     * @param
     * x, y 表示左上角相对于父控件的比例
     * width, height 表示宽度和高度相对于父控件的比例
     * 例如：QRectF(0.1, 0.1, 0.8, 0.8)
     * 表示距离左边和上边各10%，宽度和高度各占80%
     */
    void setAnchorInfo(const QRectF &anchorRect);
    PreviewClient *client() const;

signals:
    void shadowChanged();

private:
    void init();
    void createDecoration();
    void syncSizeToClient();
    void paintShadow(QPainter *painter, int &paddingLeft, int &paddingRight, int &paddingTop,
                     int &paddingBottom);
    void translateMouseEvent(QMouseEvent *event);
    void translateHoverEvent(QHoverEvent *event);
    void updateGeometryFromAnchor();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
    bool event(QEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private:
    PreviewBridge *m_bridge = nullptr;
    PreviewClient *m_client = nullptr;
    QScopedPointer<KDecoration2::Decoration> m_decoration;
    QSharedPointer<KDecoration2::DecorationSettings> m_settings;
    QRectF m_anchorInfo;
};
}  // namespace Decoration
}  // namespace Kiran