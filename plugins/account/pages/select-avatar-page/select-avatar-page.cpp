/**
 * Copyright (c) 2020 ~ 2021 KylinSec Co., Ltd.
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
#include "select-avatar-page.h"
#include "avatar-button/avatar-button.h"
#include "avatar-editor-wrapper.h"
#include "config.h"
#include "flow-layout/flowlayout.h"

#include <kiran-push-button.h>

#include <palette.h>
#include <qt5-log-i.h>
#include <QButtonGroup>
#include <QDir>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QPushButton>
#include <QScrollArea>

#define SYSTEM_AVATAR_OBJ_NAME "avatar_button_system"
#define USER_AVATAR_OBJ_NAME "avatar_button_user"
#define ADD_AVATAR_OBJ_NAME "avatar_button_add"
#define CUSTOM_AVATAR_OBJ_NAME "avatar_button_custom"

SelectAvatarPage::SelectAvatarPage(QWidget *parent) : QWidget(parent),
                                                      m_mode(CHANGE_AVATAR_FOR_USER)
{
    initUI();
}

SelectAvatarPage::~SelectAvatarPage() = default;

QString SelectAvatarPage::currentSelectAvatar() const
{
    auto button = qobject_cast<AvatarButton *>(m_buttonGroup->checkedButton());
    if (button == nullptr)
    {
        return QString();
    }
    else
    {
        return button->iconPath();
    }
}

void SelectAvatarPage::setMode(SelectAvatarPage::SelectAvatarMode mode)
{
    m_mode = mode;
}

SelectAvatarPage::SelectAvatarMode SelectAvatarPage::mode() const
{
    return m_mode;
}

void SelectAvatarPage::setCurrentAvatar(const QString &iconPath)
{
    QList<AvatarButton *> buttons = m_scrollArea->findChildren<AvatarButton *>(SYSTEM_AVATAR_OBJ_NAME);
    AvatarButton *currentAvatar = nullptr;

    // 删除之前用户添加头像
    removeUserAvatar();

    // 是否是系统头像
    for (auto button : buttons)
    {
        if (button->iconPath() == iconPath)
        {
            currentAvatar = button;
            break;
        }
    }

    // 当前头像路径未匹配上系统头像路径,添加新的头像
    if (currentAvatar != nullptr)
    {
        currentAvatar->setChecked(true);
    }
    else
    {
        QPixmap pixmap(iconPath);
        if (!pixmap.isNull())
        {
            addAvatar(iconPath, AVATAR_USER, true);
        }
        else
        {
            KLOG_WARNING() << "load" << iconPath << "failed";
        }
    }

    // 保证添加按钮在最后
    moveAddButtonToEnd();
}

void SelectAvatarPage::initUI()
{
    QPushButton *btn = nullptr;
    QLayoutItem *item = nullptr;

    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(16, 25, 16, 0);
    m_mainLayout->setSpacing(10);

    m_scrollArea = new QScrollArea;
    m_scrollArea->setFrameShape(QFrame::NoFrame);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_mainLayout->addWidget(m_scrollArea);

    m_buttonGroup = new QButtonGroup(this);
    m_buttonGroup->setExclusive(true);
    m_flowLayout = new FlowLayout(0, 16, 16);
    m_flowLayout->setMargin(0);
    /// NOTE:为什么直接使用QWidget不行?,为什么需要重写QWidget::sizeHint方法
    auto container = new QWidget(this);
    container->setLayout(m_flowLayout);
    m_scrollArea->setWidget(container);

    m_btnLayout = new QHBoxLayout;
    m_btnLayout->setSpacing(0);
    m_btnLayout->setMargin(0);
    m_btnLayout->setContentsMargins(0, 0, 0, 40);
    m_mainLayout->addItem(m_btnLayout);

    item = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    m_btnLayout->addItem(item);

    btn = new QPushButton(this);
    btn->setObjectName("btn_confirm");
    btn->setAccessibleName("ButtonConfirm");
    btn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    btn->setFixedSize(110, 40);
    btn->setText(tr("Confirm"));

    KiranPushButton::setButtonType(btn, KiranPushButton::BUTTON_Default);
    m_btnLayout->addWidget(btn);
    connect(btn, &QPushButton::clicked, [this]()
            { sigReturnToPrevPage(m_mode, true); });

    item = new QSpacerItem(40, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);
    m_btnLayout->addItem(item);

    btn = new QPushButton(this);
    btn->setObjectName("btn_return");
    btn->setAccessibleName("ButtonReturn");
    btn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    btn->setFixedSize(110, 40);
    btn->setText(tr("Return"));
    m_btnLayout->addWidget(btn);
    connect(btn, &QPushButton::clicked, [this]()
            { sigReturnToPrevPage(m_mode, false); });

    item = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    m_btnLayout->addItem(item);

    loadAvatar();

    m_addButton = addAvatar("krsvg-create-user-avatar", AVATAR_ADD, false);
    m_addButton->setAccessibleName("ButtonAddAvatar");
    m_addButton->setIcon(QIcon::fromTheme("krsvg-create-user-avatar"));

    connect(m_addButton, &AvatarButton::clicked, [this]()
            {
        //1.选择图片
        QString fileName = QFileDialog::getOpenFileName(this, tr("select picture"),
                                                        QDir::homePath(),
                                                        tr("image files(*.bmp *.jpg *.png *.tif *.gif"
                                                           " *.pcx *.tga *.exif *.fpx *.svg *.psd *.cdr *.pcd"
                                                           " *.dxf *.ufo *.eps *.ai *.raw *.WMF *.webp)"));
        if (fileName.isEmpty())
        {
            return;
        }

        //2.弹出头像编辑
        QString dstImagePath;
        if (AvatarEditorWrapper::exec(fileName, dstImagePath))
        {
            KLOG_INFO() << "exec avatar editor success." << dstImagePath;
            addAvatar(dstImagePath, AVATAR_CUSTOM, false);
            moveAddButtonToEnd();
        } });
}

void SelectAvatarPage::loadAvatar()
{
    QDir dir(ACCOUNT_BUILDIN_AVATAR_DIR);
    QFileInfoList fileInfoList = dir.entryInfoList(QDir::Files);
    for (auto &iter : fileInfoList)
    {
        addAvatar(iter.absoluteFilePath(), AVATAR_SYSTEM, false);
    }
}

AvatarButton *SelectAvatarPage::addAvatar(const QString &iconPath, AvatarTypeEnum type, bool isChecked)
{
    auto btn = new AvatarButton(m_scrollArea);
    switch (type)
    {
    case AVATAR_SYSTEM:
        btn->setObjectName(SYSTEM_AVATAR_OBJ_NAME);
        break;
    case AVATAR_USER:
        btn->setObjectName(USER_AVATAR_OBJ_NAME);
        break;
    case AVATAR_ADD:
        btn->setObjectName(ADD_AVATAR_OBJ_NAME);
        break;
    case AVATAR_CUSTOM:
        btn->setObjectName(CUSTOM_AVATAR_OBJ_NAME);
        break;
    }
    btn->setIcon(iconPath);
    btn->setFixedSize(80, 80);
    btn->setAccessibleName(QString("AvatarButton::%1").arg(iconPath));
    if (type != AVATAR_ADD)
    {
        btn->setCheckable(true);
        btn->setChecked(isChecked);
        m_buttonGroup->addButton(btn);
    }
    m_flowLayout->addWidget(btn);
    return btn;
}

void SelectAvatarPage::removeUserAvatar()
{
    // 删除非系统头像
    QList<AvatarButton *> userAvatars = m_scrollArea->findChildren<AvatarButton *>(USER_AVATAR_OBJ_NAME);
    // 删除用户添加的头像
    QList<AvatarButton *> customAvatars = m_scrollArea->findChildren<AvatarButton *>(CUSTOM_AVATAR_OBJ_NAME);

    QList<AvatarButton *> deleteAvatars;
    deleteAvatars.append(userAvatars);
    deleteAvatars.append(customAvatars);

    for (auto avatarButton : deleteAvatars)
    {
        m_flowLayout->removeWidget(avatarButton);
        avatarButton->deleteLater();
    }
}

void SelectAvatarPage::moveAddButtonToEnd()
{
    m_flowLayout->removeWidget(m_addButton);
    m_flowLayout->addWidget(m_addButton);
}
