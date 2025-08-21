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
 * Author:     yuanxing <yuanxing@kylinsec.com.cn>
 */

#include "shortcut.h"
#include "custom-line-edit.h"
#include "keybinding_backEnd_proxy.h"
#include "keycode-translator.h"
#include "logging-category.h"
#include "shortcut-item.h"
#include "thread-object.h"
#include "ui_shortcut.h"

#include <kiran-log/qt5-log-i.h>
#include <kiran-message-box.h>
#include <kiran-push-button.h>
#include <kiran-session-daemon/keybinding-i.h>
#include <QClipboard>
#include <QFileDialog>
#include <QKeyEvent>
#include <QtConcurrentRun>

#define DEFAULT_APP_ICON "application-script-blank"
#define APP_ICON_WIDTH 20

Q_DECLARE_METATYPE(QList<ShortcutInfoPtr>)
using namespace Kiran;

Shortcut::Shortcut(QWidget *parent)
    : QWidget(parent),
      ui(new Ui::Shortcut)
{
    ui->setupUi(this);
    init();
}

Shortcut::~Shortcut()
{
    delete ui;

    foreach (ShortcutItem *item, m_shortcutItem)
    {
        delete item;
        item = nullptr;
    }

    clearFilterItems();
}

QSize Shortcut::sizeHint() const
{
    return QSize(700, 730);
}

void Shortcut::init()
{
    qRegisterMetaType<QList<ShortcutInfoPtr>>("QList<ShortcutInfoPtr>");

    m_keybindingInterface = new KeybindingBackEndProxy(KEYBINDING_DBUS_NAME,
                                                       KEYBINDING_OBJECT_PATH,
                                                       QDBusConnection::sessionBus(),
                                                       this);

    connect(m_keybindingInterface, &KeybindingBackEndProxy::Added, this, &Shortcut::handleShortcutAdded);
    connect(m_keybindingInterface, &KeybindingBackEndProxy::Deleted, this, &Shortcut::handledShortcutDeleted);
    connect(m_keybindingInterface, &KeybindingBackEndProxy::Changed, this, &Shortcut::handleShortcutChanged);

    // 初始化搜索定时器
    m_searchTimer = new QTimer(this);
    m_searchTimer->setInterval(200);
    m_searchTimer->setSingleShot(true);
    connect(m_searchTimer, &QTimer::timeout, this, &Shortcut::handleSearchTimerTimeout);

    initUI();
}

void Shortcut::initUI()
{
    ui->lineEdit_search->setPlaceholderText(tr("Please enter a search keyword..."));

    KiranPushButton::setButtonType(ui->btn_shortcut_add, KiranPushButton::BUTTON_Default);
    KiranPushButton::setButtonType(ui->btn_page_add, KiranPushButton::BUTTON_Default);
    KiranPushButton::setButtonType(ui->btn_save, KiranPushButton::BUTTON_Default);

    ui->stackedWidget->setCurrentWidget(ui->page_shortcut);
    ui->stackedWidget_search->setCurrentWidget(ui->page_shortcut_list);

    QList<QLineEdit *> lineEdits = ui->page_add->findChildren<QLineEdit *>();
    foreach (QLineEdit *lineEdit, lineEdits)
    {
        lineEdit->setPlaceholderText(tr("Required"));
    }

    QHBoxLayout *hLayoutCustomApp = new QHBoxLayout(ui->lineEdit_custom_app);
    m_customAppIcon = new QLabel;
    m_customAppIcon->setFixedSize(24, 24);
    m_customAppIcon->setPixmap(QIcon::fromTheme(DEFAULT_APP_ICON).pixmap(20, 20));
    hLayoutCustomApp->addWidget(m_customAppIcon, 0, Qt::AlignVCenter);

    hLayoutCustomApp->addStretch();

    m_btnCustomApp = new QToolButton;
    m_btnCustomApp->setObjectName("btn_custom_app");
    m_btnCustomApp->setAccessibleName("ButtonAddCustomApp");
    m_btnCustomApp->setText(tr("Add"));
    m_btnCustomApp->setFixedSize(56, 30);
    m_btnCustomApp->setCursor(Qt::PointingHandCursor);
    hLayoutCustomApp->addWidget(m_btnCustomApp);

    ui->lineEdit_custom_app->setTextMargins(25, 0, m_btnCustomApp->width(), 0);
    connect(m_btnCustomApp, &QToolButton::clicked, this, &Shortcut::openFileSys);
    connect(ui->lineEdit_custom_app, &QLineEdit::textChanged, this, &Shortcut::handleCustomAppTextChanged);

    QHBoxLayout *hLayoutModifyApp = new QHBoxLayout(ui->lineEdit_modify_app);
    m_btnModifyApp = new QToolButton;
    m_btnModifyApp->setObjectName("btn_modify_app");
    m_btnModifyApp->setAccessibleName("ButtonModifyApp");
    m_btnModifyApp->setText(tr("Add"));
    m_btnModifyApp->setFixedSize(56, 30);
    m_btnModifyApp->setCursor(Qt::PointingHandCursor);
    hLayoutModifyApp->addStretch();
    hLayoutModifyApp->addWidget(m_btnModifyApp);
    ui->lineEdit_modify_app->setTextMargins(0, 0, m_btnModifyApp->width(), 0);
    connect(m_btnModifyApp, &QToolButton::clicked, this, &Shortcut::openFileSys);

    m_lECustomKey = new CustomLineEdit;
    m_lECustomKey->setPlaceholderText(tr("Please press the new shortcut key"));
    m_lECustomKey->setAccessibleName("EditCustomPressNewShotcut");
    m_lECustomKey->installEventFilter(this);
    ui->vlayout_custom_key->addWidget(m_lECustomKey);
    connect(m_lECustomKey, &CustomLineEdit::inputKeybinding, this, &Shortcut::handleInputKeycode);

    m_lEModifyKey = new CustomLineEdit;
    m_lEModifyKey->setPlaceholderText(tr("Please press the new shortcut key"));
    m_lEModifyKey->setAccessibleName("EditPressNewShotcut");
    m_lEModifyKey->installEventFilter(this);
    ui->vlayout_modify_key->addWidget(m_lEModifyKey);
    connect(m_lEModifyKey, &CustomLineEdit::inputKeybinding, this, &Shortcut::handleInputKeycode);

    // getAllShortcuts();
    m_loadShortcutsFuture = QtConcurrent::run(this, &Shortcut::loadShortcuts);

    connect(ui->btn_shortcut_add, &QPushButton::clicked,
            [this]
            {
                ui->stackedWidget->setCurrentWidget(ui->page_add);
                ui->lineEdit_custom_app->clear();
                // 将应用图标初始化为默认图标
                m_customAppIcon->setPixmap(QIcon::fromTheme(DEFAULT_APP_ICON).pixmap(20, 20));
                ui->lineEdit_custom_name->clear();
                ui->lineEdit_custom_name->setFocus();
                m_lECustomKey->clear();
            });

    connect(ui->btn_save, &QPushButton::clicked, this, &Shortcut::handleSaveClicked);
    connect(ui->btn_page_add, &QPushButton::clicked, this, &Shortcut::handleAppendClicked);

    connect(ui->btn_edit, &QToolButton::clicked,
            [this]
            {
                m_isEditMode = !m_isEditMode;
                if (m_isEditMode)
                    ui->btn_edit->setText(tr("Finished"));
                else
                    ui->btn_edit->setText(tr("Edit"));

                foreach (ShortcutItem *item, m_shortcutItem)
                    item->setEditMode(m_isEditMode);
            });

    connect(ui->btn_cancel, &QPushButton::clicked,
            [this]
            {
                ui->stackedWidget->setCurrentWidget(ui->page_shortcut);
            });

    connect(ui->btn_return, &QPushButton::clicked,
            [this]
            {
                ui->stackedWidget->setCurrentWidget(ui->page_shortcut);
            });

    connect(ui->lineEdit_search, &QLineEdit::textChanged,
            [this](QString text)
            {
                if (!text.isEmpty())
                    m_searchTimer->start();
                else
                {
                    m_searchTimer->stop();
                    ui->stackedWidget_search->setCurrentWidget(ui->page_shortcut_list);
                }
            });
    connect(ui->btn_reset, &QPushButton::clicked, this, &Shortcut::handleResetClicked);
}

void Shortcut::fetchShortcutInfoFromJson(const QJsonObject &obj, ShortcutInfoPtr &info)
{
    QMap<QString, QString *> shortcutInfoMap = {
        {KEYBINDING_SHORTCUT_JK_UID, &info->uid},
        {KEYBINDING_SHORTCUT_JK_KIND, &info->kind},
        {KEYBINDING_SHORTCUT_JK_NAME, &info->name},
        {KEYBINDING_SHORTCUT_JK_ACTION, &info->action},
        {KEYBINDING_SHORTCUT_JK_KEY_COMBINATION, &info->keyCombination}};

    for (auto iter = shortcutInfoMap.begin(); iter != shortcutInfoMap.end(); iter++)
    {
        if (obj.contains(iter.key()) && obj[iter.key()].isString())
        {
            *shortcutInfoMap[iter.key()] = obj[iter.key()].toString();
        }
    }
}

void Shortcut::loadShortcuts()
{
    QString error = tr("failed to load shortcut key data!");
    QString jsonStr;
    QJsonParseError jsonErr{};
    QJsonDocument jsonDoc;
    QJsonObject rootObject;
    QList<ShortcutInfoPtr> shortcutsInfo;

    auto funcParseShortcutArray =
        [](const QJsonObject &rootObj, ShortcutType type, QList<ShortcutInfoPtr> &shortcuts) -> bool
    {
        // <type <uid,key>>
        static QMultiMap<int, QPair<QString, QString>> blackKeybindings{
            {SHORTCUT_TYPE_SYSTEM, {"6468861322e474631c60f6b98b583eb6", "显示面板主菜单"}}};

        static QMap<ShortcutType, QString> typeJsonKeyMap{
            {SHORTCUT_TYPE_SYSTEM, KEYBINDING_SHORTCUT_JK_SYSTEM},
            {SHORTCUT_TYPE_CUSTOM, KEYBINDING_SHORTCUT_JK_CUSTOM}};

        if (typeJsonKeyMap.find(type) == typeJsonKeyMap.end())
        {
            return false;
        }

        if (!rootObj.contains(typeJsonKeyMap[type]) || !rootObj.value(typeJsonKeyMap[type]).isArray())
        {
            return false;
        }

        QJsonArray array = rootObj.value(typeJsonKeyMap[type]).toArray();
        for (auto &&shortcutItem : array)
        {
            QJsonObject obj = shortcutItem.toObject();
            ShortcutInfoPtr shortcutInfo(new ShortcutInfo);
            shortcutInfo->type = type;

            Shortcut::fetchShortcutInfoFromJson(obj, shortcutInfo);
            // 过滤黑名单中的快捷键
            auto value = QPair<QString, QString>(shortcutInfo->uid, shortcutInfo->name);
            if (blackKeybindings.contains(shortcutInfo->type, value))
            {
                KLOG_DEBUG(qLcKeybinding) << "Shortcut" << shortcutInfo->name << "is blacklisted";
                continue;
            }
            shortcuts.append(shortcutInfo);
        }

        return true;
    };

    auto reply = m_keybindingInterface->ListShortcuts();
    reply.waitForFinished();
    if (reply.isError() || !reply.isValid())
    {
        error = QString(tr("List shortcut failed,error:%1")).arg(reply.error().message());
        KLOG_ERROR(qLcKeybinding) << "list shortcut failed," << reply.error();
        goto failed;
    }
    jsonStr = reply.argumentAt(0).toString();

    jsonDoc = QJsonDocument::fromJson(jsonStr.toLocal8Bit().data(), &jsonErr);
    if (jsonErr.error != QJsonParseError::NoError || jsonDoc.isNull() || !jsonDoc.isObject())
    {
        KLOG_ERROR(qLcKeybinding) << "list shortcut failed,parse result failed!" << jsonErr.error;
        goto failed;
    }

    rootObject = jsonDoc.object();
    funcParseShortcutArray(rootObject, SHORTCUT_TYPE_CUSTOM, shortcutsInfo);
    funcParseShortcutArray(rootObject, SHORTCUT_TYPE_SYSTEM, shortcutsInfo);
    if (!QMetaObject::invokeMethod(this, "handleShortcutsLoadSuccesed", Qt::QueuedConnection, Q_ARG(QList<ShortcutInfoPtr>, shortcutsInfo)))
        KLOG_ERROR(qLcKeybinding) << "invoke method <handleShortcutsLoadSuccesed> failed!";
    return;

failed:
    QMetaObject::invokeMethod(this, "handleShortcutsLoadFailed", Qt::QueuedConnection, Q_ARG(QString, error));
}

bool Shortcut::getShortcutInfo(const QString &uid, ShortcutInfoPtr &info)
{
    QDBusPendingReply<QString> reply;

    /// NOTE:目前后端快捷键新增修改信息中 kind为翻译过后，只能根据uid前缀区分
    if (uid.startsWith("Custom", Qt::CaseInsensitive))
    {
        reply = m_keybindingInterface->GetCustomShortcut(uid);
    }
    else
    {
        reply = m_keybindingInterface->GetSystemShortcut(uid);
    }

    reply.waitForFinished();

    if (reply.isError() || !reply.isValid())
    {
        KiranMessageBox::message(nullptr,
                                 tr("Error"),
                                 QString("%1 %2").arg(tr("Get shortcut failed,error:")).arg(reply.error().message()),
                                 KiranMessageBox::Ok);
        return false;
    }

    QJsonDocument jsonDoc = QJsonDocument::fromJson(reply.argumentAt(0).toString().toLocal8Bit().data());
    QJsonObject obj = jsonDoc.object();
    fetchShortcutInfoFromJson(obj, info);
    return true;
}

ShortcutItem *Shortcut::createShortcutItem(QVBoxLayout *parent, ShortcutInfoPtr shortcutInfo, int type)
{
    ShortcutItem *item = new ShortcutItem(type, shortcutInfo);
    parent->addWidget(item);

    connect(item, &ShortcutItem::sigClicked, this, &Shortcut::handleItemClicked);
    connect(item, &ShortcutItem::sigDelete, this, &Shortcut::handleItemDeleteClicked);

    return item;
}

bool Shortcut::isConflict(QString &originName, QString newKeyCombination)
{
    foreach (auto shortcut, m_shortcutInfoList)
    {
        if (!QString::compare(shortcut->keyCombination, newKeyCombination, Qt::CaseInsensitive))
        {
            KLOG_WARNING(qLcKeybinding) << newKeyCombination << "is Conflict";
            originName = shortcut->name;
            return true;
        }
    }
    return false;
}

// 判断是否都是修饰键
bool Shortcut::isPureModifierKeys(const QString &keybinding)
{
    static QSet<QString> modifierSets = {
        MODIFIER_KEY_SHIFT, MODIFIER_KEY_CTRL, MODIFIER_KEY_ALT, MODIFIER_KEY_META};

    bool pureModifier = true;
    auto keys = keybinding.split("+");

    foreach (auto key, keys)
    {
        if (!modifierSets.contains(key))
        {
            pureModifier = false;
            break;
        }
    }

    return !pureModifier;
}

bool Shortcut::extractDesktopInfo(const QString &fileName, QString &exec, QString &icon)
{
    QSettings settings(fileName, QSettings::IniFormat);
    QString str = settings.value("Desktop Entry/Exec").toString();
    if (str.isNull())
        return false;

    // 移除掉无用的%f,%u,%F,%U
    // https://specifications.freedesktop.org/desktop-entry-spec/1.1/ar01s06.html

    str = str.replace("%f", "", Qt::CaseInsensitive);
    str = str.replace("%u", "", Qt::CaseInsensitive);

    exec = str;

    str = settings.value("Desktop Entry/Icon").toString();
    icon = str.isEmpty() ? DEFAULT_APP_ICON : str;

    return true;
}

void Shortcut::openFileSys()
{
    QToolButton *senderbtn = qobject_cast<QToolButton *>(sender());
    QLineEdit *lineEdit = qobject_cast<QLineEdit *>(senderbtn->parent());

    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Open File"),
                                                    "/usr/share/applications",
                                                    tr("Desktop entries(*.desktop)"));
    if (fileName.isNull())
        return;

    QString cmd;
    QString icon = DEFAULT_APP_ICON;
    if (!extractDesktopInfo(fileName, cmd, icon))
    {
        KLOG_ERROR(qLcKeybinding) << "cant't get Exec key from " << fileName;
        KiranMessageBox::message(this, tr("Error"),
                                 "Extracting the program to be executed from the application's desktop file failed",
                                 KiranMessageBox::Ok);
        return;
    }

    // 缓存该次添加的应用以及图标信息
    // 后续启动命令修改的时候，应复位图标
    m_lastIcon = icon;
    m_lastIconExec = cmd;

    QIcon appIcon = QIcon::fromTheme(icon);
    if (appIcon.isNull())
    {
        m_lastIcon = DEFAULT_APP_ICON;
        appIcon = QIcon::fromTheme(DEFAULT_APP_ICON);
    }

    m_customAppIcon->setPixmap(appIcon.pixmap(QSize(APP_ICON_WIDTH, APP_ICON_WIDTH)));
    lineEdit->setText(cmd);
}

void Shortcut::handleSearchTimerTimeout()
{
    clearFilterItems();
    QString text = ui->lineEdit_search->text();
    foreach (ShortcutItem *item, m_shortcutItem)
    {
        if (item->getName().contains(text, Qt::CaseInsensitive))
        {
            ShortcutItem *filterItem = createShortcutItem(ui->vlayout_filter, item->getShortcut(), item->getType());
            m_filterItem.append(filterItem);
        }
    }
    ui->stackedWidget_search->setCurrentWidget(ui->page_filter_list);
}

// custom
void Shortcut::updateShorcut(ShortcutInfoPtr newShortcut)
{
    // 更新快捷键列表
    foreach (auto shortcut, m_shortcutInfoList)
    {
        if (shortcut->uid == newShortcut->uid)
        {
            // update
            shortcut->name = newShortcut->name;
            shortcut->action = newShortcut->action;
            shortcut->keyCombination = newShortcut->keyCombination;
            break;
        }
    }
    // 更新快捷键项
    foreach (ShortcutItem *item, m_shortcutItem)
    {
        if (item->getUid() == newShortcut->uid)
        {
            item->setName(newShortcut->name);
            item->setKeyBinding(newShortcut->keyCombination);
            item->setAction(newShortcut->action);
            break;
        }
    }
    // 更新过滤后的快捷键项
    foreach (ShortcutItem *item, m_filterItem)
    {
        if (item->getUid() == newShortcut->uid)
        {
            item->setName(newShortcut->name);
            item->setKeyBinding(newShortcut->keyCombination);
            item->setAction(newShortcut->action);
            break;
        }
    }
}

void Shortcut::clearFilterItems()
{
    foreach (ShortcutItem *item, m_filterItem)
    {
        if (item)
        {
            m_filterItem.removeOne(item);
            delete item;
            item = nullptr;
            update();
        }
    }
}

void Shortcut::insertShortcut(ShortcutInfoPtr shortcutInfo)
{
    ShortcutItem *item;
    if (shortcutInfo->type == SHORTCUT_TYPE_SYSTEM)
    {
        item = createShortcutItem(m_keybindingKinds.value(shortcutInfo->kind), shortcutInfo, shortcutInfo->type);
    }
    else
    {
        item = createShortcutItem(ui->vlayout_custom,
                                  shortcutInfo,
                                  shortcutInfo->type);
        m_customShortcutCount++;
    }
    m_shortcutItem.append(item);
    m_shortcutInfoList.append(shortcutInfo);
    if (m_customShortcutCount == 0)
        ui->widget_custom->hide();
    else
        ui->widget_custom->show();
}

void Shortcut::handleShortcutsLoadFailed(QString error)
{
}

void Shortcut::handleShortcutAdded(QString result)
{
    QJsonParseError jsonErr{};
    QJsonDocument jsonDoc = QJsonDocument::fromJson(result.toLocal8Bit().data(), &jsonErr);

    if (jsonErr.error != QJsonParseError::NoError || jsonDoc.isNull())
    {
        KLOG_ERROR(qLcKeybinding) << "parse shortcut json failed!";
        return;
    }

    ShortcutInfoPtr shortcutInfo(new ShortcutInfo);
    QJsonObject jsonObject = jsonDoc.object();

    fetchShortcutInfoFromJson(jsonObject, shortcutInfo);
    QString uid = shortcutInfo->uid;
    if (uid.startsWith("Custom", Qt::CaseInsensitive))
    {
        shortcutInfo->type = SHORTCUT_TYPE_CUSTOM;
    }

    KLOG_INFO(qLcKeybinding) << "shortcut added:" << shortcutInfo->uid << shortcutInfo->kind << shortcutInfo->name;

    getShortcutInfo(uid, shortcutInfo);
    insertShortcut(shortcutInfo);
}

void Shortcut::handledShortcutDeleted(QString result)
{
    QJsonParseError jsonErr{};
    QJsonDocument jsonDoc = QJsonDocument::fromJson(result.toLocal8Bit().data(), &jsonErr);

    if (jsonErr.error != QJsonParseError::NoError || jsonDoc.isNull())
    {
        return;
    }
    ShortcutInfoPtr shortcutInfo(new ShortcutInfo);
    QJsonObject jsonObject = jsonDoc.object();

    fetchShortcutInfoFromJson(jsonObject, shortcutInfo);

    KLOG_INFO(qLcKeybinding) << "shortcut deleted:" << shortcutInfo->uid
                             << shortcutInfo->kind << shortcutInfo->name;

    QString uid = shortcutInfo->uid;
    foreach (ShortcutItem *item, m_shortcutItem)
    {
        if (item->getUid() == uid)
        {
            m_shortcutItem.removeOne(item);
            ShortcutInfoPtr shortcut = item->getShortcut();
            if (shortcut->type == SHORTCUT_TYPE_CUSTOM)
            {
                m_customShortcutCount--;
                if (m_customShortcutCount == 0)
                {
                    ui->widget_custom->hide();
                    ui->btn_edit->setText(tr("Edit"));
                }
            }
            delete item;
            item = nullptr;
            break;
        }
    }
}

void Shortcut::handleShortcutChanged(QString result)
{
    QJsonParseError jsonErr{};
    QJsonDocument jsonDoc = QJsonDocument::fromJson(result.toLocal8Bit().data(), &jsonErr);

    if (jsonErr.error != QJsonParseError::NoError || jsonDoc.isNull())
    {
        return;
    }
    ShortcutInfoPtr shortcutInfo(new ShortcutInfo);
    QJsonObject jsonObject = jsonDoc.object();

    fetchShortcutInfoFromJson(jsonObject, shortcutInfo);

    QString uid = shortcutInfo->uid;

    KLOG_INFO(qLcKeybinding) << "shortcut changed:" << shortcutInfo->uid
                             << shortcutInfo->kind << shortcutInfo->name;

    getShortcutInfo(uid, shortcutInfo);
    updateShorcut(shortcutInfo);
}

void Shortcut::handleShortcutsLoadSuccesed(QList<ShortcutInfoPtr> shortcutInfoList)
{
    foreach (auto shortcutInfo, shortcutInfoList)
    {
        QLayout *layout = ui->scrollAreaCont_all->layout();
        auto iter = m_keybindingKinds.find(shortcutInfo->kind);

        // 非自定义快捷键种类，创建布局
        if ((!shortcutInfo->kind.isEmpty()) && iter == m_keybindingKinds.end())
        {
            QWidget *widget = new QWidget();
            widget->setObjectName(QString("widget_%1").arg(shortcutInfo->kind));

            QVBoxLayout *vLayout = new QVBoxLayout(widget);
            vLayout->setMargin(0);
            vLayout->setSpacing(10);

            QLabel *labKind = new QLabel(widget);
            if (shortcutInfo->kind == SHORTCUT_KIND_SYSTEM)
                labKind->setText(tr("System"));
            else if (shortcutInfo->kind == SHORTCUT_KIND_SOUND)
                labKind->setText(tr("Sound"));
            else if (shortcutInfo->kind == SHORTCUT_KIND_ACCESSIBILITY)
                labKind->setText(tr("Accessibility"));
            else
                labKind->setText(shortcutInfo->kind);
            vLayout->addWidget(labKind);

            m_keybindingKinds.insert(shortcutInfo->kind, vLayout);
            layout->addWidget(widget);
        }

        // 添加快捷键条目进入布局
        insertShortcut(shortcutInfo);
    }
}

void Shortcut::handleItemClicked(int type, QString uid, QString name, QString keyCombination, QString action)
{
    Q_UNUSED(keyCombination)
    ShortcutItem *senderItem = qobject_cast<ShortcutItem *>(sender());
    ui->stackedWidget->setCurrentWidget(ui->page_modify);
    m_lEModifyKey->clear();
    m_lEModifyKey->setFocus();
    m_editUid = uid;

    ui->lineEdit_modify_name->setText(name);
    ui->lineEdit_modify_app->setText(action);

    if (type == SHORTCUT_TYPE_SYSTEM)
    {
        ui->widget_modify_app->hide();
        ui->lineEdit_modify_name->setDisabled(true);
    }
    else
    {
        ui->widget_modify_app->show();
        ui->lineEdit_modify_name->setDisabled(false);
    }
    m_lEModifyKey->setText(senderItem->getShowKeybinding());
    m_editKeybination = senderItem->getShowKeybinding();
}

void Shortcut::handleItemDeleteClicked(QString uid)
{
    QDBusPendingReply<> reply = m_keybindingInterface->DeleteCustomShortcut(uid);
    reply.waitForFinished();
    if (reply.isError() || !reply.isValid())
    {
        KLOG_ERROR(qLcKeybinding) << "Call DeleteCustomShortcut method failed "
                                  << " Error: " << reply.error().message();

        KiranMessageBox::message(nullptr,
                                 tr("Failed"),
                                 QString("%1 %2").arg(tr("Delete shortcut failed,error:")).arg(reply.error().message()),
                                 KiranMessageBox::Ok);
        return;
    }
}

void Shortcut::handleSaveClicked()
{
    int type = ui->lineEdit_modify_app->isVisible() ? SHORTCUT_TYPE_CUSTOM : SHORTCUT_TYPE_SYSTEM;
    if (ui->lineEdit_modify_name->text().isEmpty() ||
        (ui->lineEdit_modify_app->text().isEmpty() && type == SHORTCUT_TYPE_CUSTOM))
    {
        KiranMessageBox::message(nullptr,
                                 tr("Warning"),
                                 tr("Please complete the shortcut information!"),
                                 KiranMessageBox::Ok);
        return;
    }

    QString newKeyCombination;
    if (m_lEModifyKey->text().isEmpty())
    {
        auto reply = KiranMessageBox::message(nullptr,
                                              tr("Set shortcut"),
                                              tr("Are you sure you want to disable this shortcut?"),
                                              KiranMessageBox::Yes | KiranMessageBox::No);
        if (reply == KiranMessageBox::Yes)
            newKeyCombination = "disabled";
        else
            return;
    }
    else if (m_lEModifyKey->text() == m_editKeybination)
    {
        ui->stackedWidget->setCurrentWidget(ui->page_shortcut);
        return;
    }
    else
        newKeyCombination = KeycodeTranslator::readable2BackendKeyString(m_lEModifyKey->text());

    KLOG_DEBUG(qLcKeybinding) << "Modify new keybind to backend: " << newKeyCombination;

    if (type == SHORTCUT_TYPE_SYSTEM)
    {
        QDBusPendingReply<> reply = m_keybindingInterface->ModifySystemShortcut(m_editUid, newKeyCombination);
        reply.waitForFinished();
        if (reply.isError() || !reply.isValid())
        {
            KLOG_ERROR(qLcKeybinding) << "Call ModifySystemShortcut method failed "
                                      << " Error: " << reply.error().message();

            KiranMessageBox::message(nullptr,
                                     tr("Failed"),
                                     QString("%1 %2").arg(tr("Modify system shortcut failed,error:")).arg(reply.error().message()),
                                     KiranMessageBox::Ok);

            return;
        }
        else
            ui->stackedWidget->setCurrentWidget(ui->page_shortcut);
    }
    else
    {
        QString newName = ui->lineEdit_modify_name->text();
        QString newAction = ui->lineEdit_modify_app->text();
        QDBusPendingReply<> reply = m_keybindingInterface->ModifyCustomShortcut(m_editUid, newName, newAction, newKeyCombination);
        reply.waitForFinished();
        if (reply.isError() || !reply.isValid())
        {
            KLOG_ERROR(qLcKeybinding) << "Call ModifyCustomShortcut method failed "
                                      << " Error: " << reply.error().message();

            KiranMessageBox::message(nullptr,
                                     tr("Failed"),
                                     QString("%1 %2").arg(tr("Modify custom shortcut failed,error:")).arg(reply.error().message()),
                                     KiranMessageBox::Ok);
            return;
        }
        else
            ui->stackedWidget->setCurrentWidget(ui->page_shortcut);
    }
}

void Shortcut::handleAppendClicked()
{
    QString newName = ui->lineEdit_custom_name->text();
    QString newAction = ui->lineEdit_custom_app->text();
    QString newKey = m_lECustomKey->text();
    if (newName.isEmpty() || newAction.isEmpty())
    {
        KiranMessageBox::message(nullptr,
                                 tr("Warning"),
                                 tr("Please complete the shortcut information!"),
                                 KiranMessageBox::Ok);
        return;
    }

    // dbus ->AddCustomShortcut
    QString keyCombination = newKey.isEmpty() ? "disabled" : KeycodeTranslator::readable2BackendKeyString(newKey);

    KLOG_DEBUG(qLcKeybinding) << "Add custom shortcut to backend:" << newName << keyCombination << newAction;

    QDBusPendingReply<QString> reply = m_keybindingInterface->AddCustomShortcut(newName, newAction, keyCombination);
    reply.waitForFinished();
    if (reply.isError() || !reply.isValid())
    {
        KLOG_ERROR(qLcKeybinding) << "Call AddCustomShortcut method failed "
                                  << " Error: " << reply.error().message();

        KiranMessageBox::message(nullptr,
                                 tr("Failed"),
                                 QString("%1 %2").arg(tr("Add custom shortcut failed,error:")).arg(reply.error().message()),
                                 KiranMessageBox::Ok);
        return;
    }
    else
        ui->stackedWidget->setCurrentWidget(ui->page_shortcut);
}

void Shortcut::handleResetClicked()
{
    KLOG_INFO(qLcKeybinding) << "reset shortcuts";

    QDBusPendingReply<> reply = m_keybindingInterface->ResetShortcuts();
    reply.waitForFinished();
    if (reply.isError() || !reply.isValid())
    {
        KLOG_ERROR(qLcKeybinding) << "Call ResetShortcuts method failed "
                                  << " Error: " << reply.error().message();

        KiranMessageBox::message(nullptr,
                                 tr("Failed"),
                                 QString("%1 %2").arg(tr("Reset shortcut failed,error:")).arg(reply.error().message()),
                                 KiranMessageBox::Ok);

        return;
    }
}

void Shortcut::handleCustomAppTextChanged(const QString &text)
{
    // 直接编辑自定义应用输入框，修改命令
    // 导致和desktop读取出来的不一致时清空图标
    if (!text.isEmpty() && text != m_lastIconExec)
    {
        m_customAppIcon->setPixmap(QIcon::fromTheme(DEFAULT_APP_ICON).pixmap(20, 20));
    }
}

void Shortcut::handleInputKeycode(const QStringList &keys)
{
    CustomLineEdit *senderLineEdit = qobject_cast<CustomLineEdit *>(sender());

    // 转化成字符串列表,用于显示
    QString keyStr = KeycodeTranslator::keyStrings2ReadableString(keys);
    KLOG_DEBUG(qLcKeybinding) << "The input keys: " << keys << "Readable string: " << keyStr;

    // 判断快捷键输入是否合法（排除都是修饰键的情况）
    if (!isPureModifierKeys(keyStr))
    {
        KiranMessageBox::message(nullptr,
                                 tr("Failed"),
                                 QString(tr("Cannot use shortcut \"%1\","
                                            "Shortcuts cannot be set to only modifier keys. "
                                            "Please add a regular key, like A-Z, and so on."))
                                     .arg(keyStr),
                                 KiranMessageBox::Ok);
        return;
    }

    // 不支持单个按键作为快捷键
    if (keys.size() == 1)
    {
        KiranMessageBox::message(nullptr,
                                 tr("Failed"),
                                 QString(tr("Cannot use shortcut \"%1\","
                                            "Please keep pressing the modifier keys such as Ctrl,"
                                            "Alt, and Shift before pressing the last key of the shortcut key"))
                                     .arg(keyStr),
                                 KiranMessageBox::Ok);
        return;
    }

    // 不支持shift+主键
    if (keys.size() == 2 && keyStr.contains(MODIFIER_KEY_SHIFT))
    {
        KiranMessageBox::message(nullptr,
                                 tr("Failed"),
                                 QString(tr("Cannot use shortcut \"%1\","
                                            "Please continue to input modifier keys like Ctrl, Alt, and Meta."))
                                     .arg(keyStr),
                                 KiranMessageBox::Ok);
        return;
    }

    // 判断是否重复
    QString originName;
    if (isConflict(originName, keyStr))
    {
        KiranMessageBox::message(nullptr,
                                 QString(tr("Failed")),
                                 QString(tr("Shortcut keys %1 are already used in %2,Please try again!")).arg(keyStr).arg(originName),
                                 KiranMessageBox::Ok);
        m_lECustomKey->clear();
        return;
    }

    // 显示在输入框中
    senderLineEdit->setText(keyStr);
    senderLineEdit->clearFocus();
}

// 解决输入Ctrl+v会显示剪切板中的内容
bool Shortcut::eventFilter(QObject *target, QEvent *event)
{
    if (target == m_lECustomKey || target == m_lEModifyKey)
    {
        if (event->type() == QEvent::KeyPress)
        {
            QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
            if (keyEvent->matches(QKeySequence::Paste))
            {
                return true;
            }
        }
    }
    return QWidget::eventFilter(target, event);
}
