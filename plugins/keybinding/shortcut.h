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

#ifndef SHORTCUT_H
#define SHORTCUT_H

#include "keybinding_def.h"

#include <QFile>
#include <QFuture>
#include <QLineEdit>
#include <QTimer>
#include <QToolButton>
#include <QVBoxLayout>
#include <QWidget>

namespace Ui
{
class Shortcut;
}

class ThreadObject;
class ShortcutItem;
class KeyMap;
class CustomLineEdit;
class KeybindingBackEndProxy;
class QLabel;

class Shortcut : public QWidget
{
    Q_OBJECT

public:
    explicit Shortcut(QWidget *parent = 0);
    ~Shortcut();

    QSize sizeHint() const override;

private:
    void init();
    void initUI();

    // 从Json对象之中提取信息至ShortcutInfo
    static void fetchShortcutInfoFromJson(const QJsonObject &obj, ShortcutInfoPtr &info);
    // 加载全部快捷键
    void loadShortcuts();
    bool getShortcutInfo(const QString &uid, ShortcutInfoPtr &info);

private:
    ShortcutItem *createShortcutItem(QVBoxLayout *parent, ShortcutInfoPtr shortcutInfo, int type);
    bool isConflict(QString &originName, QString newKeyCombination);
    bool isPureModifierKeys(const QString &keys);
    bool extractDesktopInfo(const QString &fileName, QString &exec, QString &icon);
    void updateShorcut(ShortcutInfoPtr newShortcut);
    void insertShortcut(ShortcutInfoPtr shortcutInfo);
    void clearFilterItems();

public slots:
    void handleShortcutsLoadSuccesed(QList<ShortcutInfoPtr> shortcuts);
    void handleShortcutsLoadFailed(QString error);

    void handleShortcutAdded(QString result);
    void handledShortcutDeleted(QString result);
    void handleShortcutChanged(QString result);

    void handleCustomAppTextChanged(const QString &text);
    void handleInputKeycode(const QStringList &keys);

    void handleItemDeleteClicked(QString uid);
    void handleItemClicked(int type, QString uid, QString name, QString keyCombination, QString action = nullptr);

    void handleSaveClicked();
    void handleAppendClicked();
    void handleResetClicked();
    void openFileSys();

    void handleSearchTimerTimeout();

private:
    virtual bool eventFilter(QObject *target, QEvent *event);

private:
    Ui::Shortcut *ui;

    QFuture<void> m_loadShortcutsFuture;

    QList<ShortcutInfoPtr> m_shortcutInfoList;
    QList<ShortcutItem *> m_shortcutItem;
    QList<ShortcutItem *> m_filterItem;

    // 记录上次通过应用打开的desktop文件记录的Exec和Icon
    QString m_lastIconExec;
    QString m_lastIcon;

    QToolButton *m_btnModifyApp;
    QToolButton *m_btnCustomApp;
    QLabel *m_customAppIcon;
    CustomLineEdit *m_lECustomKey;
    CustomLineEdit *m_lEModifyKey;

    KeybindingBackEndProxy *m_keybindingInterface;
    QTimer *m_searchTimer;

    bool m_isEditMode = false;
    int m_customShortcutCount = 0;
    QString m_editUid;
    QString m_editKeybination;
    QMap<QString, QVBoxLayout *> m_keybindingKinds;
};

#endif  // SHORTCUT_H
