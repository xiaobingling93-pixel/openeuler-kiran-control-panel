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
#include "account-widget.h"
#include "account.h"
#include "accounts-global-info.h"
#include "config.h"
#include "hard-worker.h"
#include "logging-category.h"

#include "create-user-page/create-user-page.h"
#include "mask-widget/mask-widget.h"
#include "passwd-expiration-policy/password-expiration-policy-page.h"
#include "select-avatar-page/select-avatar-page.h"
#include "user-info-page/user-info-page.h"

#include <kiran-color-block.h>
#include <kiran-sidebar-item.h>
#include <kiran-sidebar-widget.h>
#include <qt5-log-i.h>
#include <QHBoxLayout>
#include <QIcon>
#include <QScrollArea>
#include <QStackedWidget>
#include <QtWidgets/QListWidgetItem>

#define ITEM_USER_OBJ_PATH_ROLE Qt::UserRole + 1

enum StackWidgetPageEnum
{
    PAGE_CREATE_USER,
    PAGE_USER_INFO,
    PAGE_SELECT_AVATAR,
    PAGE_PASSWD_EXPIRATION_POLICY
};

AccountWidget::AccountWidget(QWidget *parent)
    : QWidget(parent)
{
    m_workThread.start();
    m_hardworker = new HardWorker();
    m_hardworker->moveToThread(&m_workThread);
    initUI();
}

AccountWidget::~AccountWidget()
{
    if (m_workThread.isRunning())
    {
        m_workThread.quit();
        m_workThread.wait();
    }
    delete m_hardworker;
};

void AccountWidget::setCurrentUser(const QString &userPath)
{
    int findIdx = -1;

    for (int i = 0; i < m_tabList->count(); i++)
    {
        if (m_tabList->item(i)->data(ITEM_USER_OBJ_PATH_ROLE) != userPath)
            continue;
        findIdx = i;
        break;
    }

    Q_ASSERT(findIdx != -1);
    m_tabList->setCurrentRow(findIdx);
}

void AccountWidget::appendUser(const QString &userPath)
{
    auto accountUserBackend = DBusWrapper::Account::userInterface(userPath);
    QString iconFile = accountUserBackend->icon_file();
    QString userName = accountUserBackend->user_name();
    bool userLocked = accountUserBackend->locked();

    QPixmap temp;
    if (iconFile.isEmpty() || !temp.load(iconFile))
    {
        iconFile = ACCOUNT_DEFAULT_AVATAR;
    }

    auto item = new KiranSidebarItem(userName, m_tabList);
    item->setSizeHint(QSize(240, 50));
    item->setIcon(QPixmap(iconFile));
    item->setStatusDesc(userLocked ? tr("disable") : tr("enable"), userLocked ? QColor("#fa4949") : QColor("#43a3f2"));
    item->setData(ITEM_USER_OBJ_PATH_ROLE, userPath);

    m_tabList->addItem(item);
}

void AccountWidget::setDefaultSelectedUser()
{
    auto items = m_tabList->findItems(AccountsGlobalInfo::instance()->getCurrentUser(), Qt::MatchCaseSensitive);
    if (items.size() >= 1)
    {
        auto userItem = items.at(0);
        m_tabList->setCurrentRow(m_tabList->row(userItem));
    }
    else
    {
        m_tabList->setCurrentRow(0);
    }
}

void AccountWidget::initUI()
{
    /* 遮罩,用于繁忙时屏蔽用户操作 */
    m_maskWidget = new MaskWidget(this);
    m_maskWidget->setVisible(true);

    /* 初始化界面主布局 */
    auto contentLayout = new QHBoxLayout(this);
    contentLayout->setObjectName("AccountContentLayout");
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(4);

    /* 侧边栏 */
    auto siderbar = new KiranColorBlock(this);
    contentLayout->addWidget(siderbar);
    siderbar->setObjectName("siderWidget");
    siderbar->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    siderbar->setFixedWidth(272);

    auto vLayout = new QVBoxLayout(siderbar);
    vLayout->setSpacing(0);
    vLayout->setContentsMargins(0, 0, 0, 0);
    vLayout->setObjectName("SiderbarVLayout");

    m_tabList = new KiranSidebarWidget(siderbar);
    m_tabList->viewport()->setAutoFillBackground(false);
    m_tabList->setFrameShape(QFrame::NoFrame);
    m_tabList->setObjectName("tabList");
    m_tabList->setIconSize(QSize(40, 40));
    vLayout->addWidget(m_tabList);
    initUserList();

    /* 内容区域 */
#if 0
    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setWidgetResizable(true);

    QWidget *scrollAreaContentWidget = new QWidget(this);
    QHBoxLayout *scrollAreaContentLayout = new QHBoxLayout;
    scrollAreaContentLayout->setMargin(0);
    scrollAreaContentWidget->setLayout(scrollAreaContentLayout);
#endif
    KiranColorBlock *stackedColorBlock = new KiranColorBlock(this);
    contentLayout->addWidget(stackedColorBlock);
    QHBoxLayout *stackedColorBlockLayout = new QHBoxLayout(stackedColorBlock);

    m_stackWidget = new QStackedWidget(this);
    m_stackWidget->setObjectName("StackWidget");
    //    scrollAreaContentLayout->addWidget(m_stackWidget);
    stackedColorBlockLayout->addWidget(m_stackWidget);

    //    scrollArea->setWidget(scrollAreaContentWidget);
    //    contentLayout->addWidget(scrollArea);

    m_page_createUser = new CreateUserPage(m_stackWidget);
    m_stackWidget->insertWidget(PAGE_CREATE_USER, m_page_createUser);
    initPageCreateUser();

    m_page_userinfo = new UserInfoPage(m_stackWidget);
    m_stackWidget->insertWidget(PAGE_USER_INFO, m_page_userinfo);
    initPageUserInfo();

    m_page_selectAvatar = new SelectAvatarPage(m_stackWidget);
    m_stackWidget->insertWidget(PAGE_SELECT_AVATAR, m_page_selectAvatar);
    initPageSelectAvatar();

    m_page_passwdExpirationPolicy = new PasswordExpirationPolicyPage(m_stackWidget);
    m_stackWidget->insertWidget(PAGE_PASSWD_EXPIRATION_POLICY, m_page_passwdExpirationPolicy);
    initPagePasswdExpirationPolicy();

    connectToInfoChange();
    QTimer::singleShot(0, this, &AccountWidget::setDefaultSelectedUser);
}

void AccountWidget::initUserList()
{
    connect(m_tabList, &KiranSidebarWidget::itemSelectionChanged, [this]()
            {
                QList<QListWidgetItem *> selecteds = m_tabList->selectedItems();
                if (selecteds.size() != 1)
                {
                    return;
                }
                QListWidgetItem *item = selecteds.at(0);
                if (item == m_createUserItem)
                {
                    //重置创建用户页面
                    m_page_createUser->reset();
                    //切换到创建用户
                    m_stackWidget->setCurrentIndex(PAGE_CREATE_USER);
                }
                else
                {
                    QString usrObjPath = item->data(ITEM_USER_OBJ_PATH_ROLE).toString();
                    //更新用户信息页面
                    m_page_userinfo->setCurrentShowUserPath(usrObjPath);
                    //切换到用户信息
                    m_stackWidget->setCurrentIndex(PAGE_USER_INFO);
                }
            });

    /// 创建用户按钮
    m_createUserItem = new QListWidgetItem(tr("Create new user"), m_tabList);
    m_createUserItem->setIcon(QIcon::fromTheme("krsvg-create-user-avatar"));
    m_tabList->addItem(m_createUserItem);

    // 加载非系统用户
    QList<QString> userObjList;
    userObjList = AccountsGlobalInfo::instance()->getUserObjectPathList();
    for (auto &iter : userObjList)
    {
        appendUser(iter);
    }
}

void AccountWidget::initPageCreateUser()
{
    connect(m_page_createUser, &CreateUserPage::requestIconPageForNewUser,
            this, &AccountWidget::onRequestIconPageForNewUser);
    connect(m_page_createUser, &CreateUserPage::requestSetCurrentUser,
            this, &AccountWidget::onRequestSetCurrentUser);
    connect(m_page_createUser, &CreateUserPage::requestCreateUser,
            m_hardworker, &HardWorker::doCreateUser);
    connect(m_hardworker, &HardWorker::sigCreateUserDnoe,
            m_page_createUser, &CreateUserPage::onCreateUserDone);
    connect(m_page_createUser, &CreateUserPage::busyChanged,
            this, &AccountWidget::setMaskVisible);
}

void AccountWidget::initPageUserInfo()
{
    // 用户信息页面处理头像点击
    connect(m_page_userinfo, &UserInfoPage::requestIconPage, [this](const QString &iconPath)
            {
                m_page_selectAvatar->setMode(SelectAvatarPage::CHANGE_AVATAR_FOR_USER);
                m_page_selectAvatar->setCurrentAvatar(iconPath);
                m_stackWidget->setCurrentIndex(PAGE_SELECT_AVATAR);
            });

    // 用户信息页面，密码过期策略点击时请求跳转至密码过期策略页面
    connect(m_page_userinfo, &UserInfoPage::requestPasswordExpirationPolicy, [this](const QString &userObj)
            {
                m_page_passwdExpirationPolicy->setCurrentUser(userObj);
                m_stackWidget->setCurrentIndex(PAGE_PASSWD_EXPIRATION_POLICY);
            });

    /// 修改属性
    connect(m_page_userinfo, &UserInfoPage::requestUpdateUserProperty,
            m_hardworker, &HardWorker::doUpdateUserProperty);

    connect(m_hardworker, &HardWorker::sigUpdateUserPropertyDone,
            m_page_userinfo, &UserInfoPage::handlerUpdateUserPropertyDone);

    /// 修改密码
    connect(m_page_userinfo, &UserInfoPage::requestUpdatePasswd,
            m_hardworker, &HardWorker::doUpdatePasswd);
    connect(m_hardworker, &HardWorker::sigUpdatePasswdDone,
            m_page_userinfo, &UserInfoPage::handlerUpdatePasswdDone);

    /// 删除用户
    connect(m_page_userinfo, &UserInfoPage::requestDeleteUser,
            m_hardworker, &HardWorker::doDeleteUser);
    connect(m_hardworker, &HardWorker::sigDeleteUserDone,
            m_page_userinfo, &UserInfoPage::handlerDeleteUserDone);

    /// 忙碌显示/隐藏遮罩
    connect(m_page_userinfo, &UserInfoPage::busyChanged,
            this, &AccountWidget::setMaskVisible);
}

void AccountWidget::initPageSelectAvatar()
{
    // 选择头像页面处理返回
    connect(m_page_selectAvatar, &SelectAvatarPage::sigReturnToPrevPage,
            [this](SelectAvatarPage::SelectAvatarMode mode, bool isConfirm)
            {
                switch (mode)
                {
                case SelectAvatarPage::SELECT_AVATAR_FOR_NEW_USER:
                    if (isConfirm)
                    {
                        m_page_createUser->setAvatarIconPath(m_page_selectAvatar->currentSelectAvatar());
                    }
                    m_stackWidget->setCurrentIndex(PAGE_CREATE_USER);
                    break;
                case SelectAvatarPage::CHANGE_AVATAR_FOR_USER:
                    if (isConfirm)
                    {
                        m_page_userinfo->setAvatarIconPath(m_page_selectAvatar->currentSelectAvatar());
                    }
                    m_stackWidget->setCurrentIndex(PAGE_USER_INFO);
                    break;
                }
            });
}

void AccountWidget::connectToInfoChange()
{
    connect(AccountsGlobalInfo::instance(), &AccountsGlobalInfo::UserAdded, this, &AccountWidget::onUserAdded);
    connect(AccountsGlobalInfo::instance(), &AccountsGlobalInfo::UserDeleted, this, &AccountWidget::onUserDeleted);
    connect(AccountsGlobalInfo::instance(), &AccountsGlobalInfo::UserPropertyChanged, this, &AccountWidget::onUserPropertyChanged);
}

void AccountWidget::setMaskVisible(bool visible)
{
    if (visible)
    {
        m_maskWidget->raise();
        m_maskWidget->show();
    }
    else
    {
        m_maskWidget->hide();
    }
}

void AccountWidget::onUserAdded(const QString &objectPath)
{
    KLOG_DEBUG(qLcAccount) << "on user added,add user" << objectPath << "to sidebar";
    appendUser(objectPath);
}

void AccountWidget::onUserDeleted(const QString &objectPath)
{
    KLOG_DEBUG(qLcAccount) << "on user deleted,delete user" << objectPath << "from sidbar";
    int findIdx = -1;

    for (int i = 0; i < m_tabList->count(); i++)
    {
        if (m_tabList->item(i)->data(ITEM_USER_OBJ_PATH_ROLE) == objectPath)
        {
            findIdx = i;
            break;
        }
    }

    if (findIdx == -1)
    {
        KLOG_WARNING(qLcAccount) << "can't find deleted user:" << objectPath;
        return;
    }

    bool needResetSidebarItem = m_tabList->item(findIdx)->isSelected();

    QListWidgetItem *widgetItem = m_tabList->takeItem(findIdx);
    delete widgetItem;

    if (needResetSidebarItem)
    {
        setDefaultSelectedUser();
    }
}

void AccountWidget::onUserPropertyChanged(const QString &objectPath, const QString &propertyName, QVariant value)
{
    KLOG_DEBUG(qLcAccount) << "user property changed:"
                           << " user:" << objectPath
                           << " property name:" << propertyName
                           << " value:" << value;

    // 侧边栏
    if ((propertyName == "locked") || (propertyName == "icon_file"))
    {
        for (int i = 0; i < m_tabList->count(); i++)
        {
            QListWidgetItem *item = m_tabList->item(i);
            QString itemUserPath = item->data(ITEM_USER_OBJ_PATH_ROLE).toString();
            if (itemUserPath != objectPath)
            {
                continue;
            }
            auto accountAPI = DBusWrapper::Account::userInterface(itemUserPath);
            QString userName = accountAPI->user_name();
            QString iconFile = accountAPI->icon_file();
            QPixmap tempPixmap;
            if (iconFile.isEmpty() || !tempPixmap.load(iconFile))
            {
                iconFile = ACCOUNT_DEFAULT_AVATAR;
            }

            bool isLocked = accountAPI->locked();
            item->setText(userName);
            item->setIcon(QIcon(iconFile));
            ((KiranSidebarItem *)item)->setStatusDesc(isLocked ? tr("disable") : tr("enable"), isLocked ? QColor("#fa4949") : QColor("#43a3f2"));
            break;
        }
    }

    // 用户详情页面
    QString currentUserPath = m_page_userinfo->getCurrentShowUserPath();
    if (objectPath == currentUserPath)
    {
        m_page_userinfo->updateInfo();
    }
}

void AccountWidget::onRequestIconPageForNewUser(const QString &iconPath)
{
    // 设置选择头像模式，为了缓存选择头像之前的页面，方便之后的返回
    m_page_selectAvatar->setMode(SelectAvatarPage::SELECT_AVATAR_FOR_NEW_USER);

    // 设置头像，切换到选择头像页面
    m_page_selectAvatar->setCurrentAvatar(iconPath);
    m_stackWidget->setCurrentIndex(PAGE_SELECT_AVATAR);
}

void AccountWidget::onRequestSetCurrentUser(const QString &userPath)
{
    // 保证在设置当前行时,新用户已在侧边栏创建节点
    QTimer::singleShot(0, this, [=]()
                       {
                           int findIdx = -1;
                           for (int i = 0; i < m_tabList->count(); i++)
                           {
                               if (m_tabList->item(i)->data(ITEM_USER_OBJ_PATH_ROLE) != userPath)
                               {
                                   continue;
                               }
                               findIdx = i;
                               break;
                           }
                           Q_ASSERT(findIdx != -1);
                           m_tabList->setCurrentRow(findIdx);
                       });
}

QSize AccountWidget::sizeHint() const
{
    return {780, 657};
}

void AccountWidget::initPagePasswdExpirationPolicy()
{
    connect(m_page_passwdExpirationPolicy, &PasswordExpirationPolicyPage::sigReturn, [this]()
            { m_stackWidget->setCurrentIndex(PAGE_USER_INFO); });
}

void AccountWidget::jumpToUser(const QString &user)
{
    auto items = m_tabList->findItems(user, Qt::MatchCaseSensitive);
    if (items.size() == 1)
    {
        auto item = items.at(0);
        m_tabList->setCurrentRow(m_tabList->row(item));
    }
}

void AccountWidget::jumpToAddUser()
{
    m_tabList->setCurrentRow(0);
}