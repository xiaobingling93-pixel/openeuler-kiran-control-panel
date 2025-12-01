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
#include "exclusion-group.h"
#include <QString>

class KiranFrame;
class QLabel;
class QHBoxLayout;

class ThemePreview : public ExclusionWidget
{
    Q_OBJECT
public:
    ThemePreview(QWidget* parent = nullptr);
    ~ThemePreview();

    void setPreviewFixedHeight(int height);
    void setPreviewFixedSize(QSize size);

    // 设置图片展示区域的间距以及margin
    void setSpacingAndMargin(int spacing,QMargins margins);

    // 设置选中的时候是否显示选择指示器(勾)
    void setSelectedIndicatorEnable(bool visible);

    // 设置选中高亮边框宽度,默认为1
    void setSelectedBorderWidth(int width);

    // 设置当前展示的图片信息
    void setThemeInfo(const QString& name,const QString& id);

    // 设置展示图片
    void setPreviewPixmaps(const QList<QPixmap>& pixmaps,const QSize& size);
    // 设置展示控件
    void setPreviewWidget(QWidget* widget);
    void clearPreview();

    // 获取该展示控件的主题ID
    QString getID() const override;

    virtual void setSelected(bool selected) override;

signals:
    void pressed();

private:
    void initUI();
    virtual bool eventFilter(QObject* watched, QEvent* event) override;

private:
    bool m_selectedIndicatorEnable = false;
    QString m_themeName;
    QString m_themeID;
    KiranFrame* m_frame;
    QHBoxLayout* m_frameLayout;
    QLabel* m_labelThemeName = nullptr;
    QLabel* m_selectedIndicator = nullptr;
};
