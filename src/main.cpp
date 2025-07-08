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

#include <kiran-single-application.h>
#include <qt5-log-i.h>
#include <iostream>
#include <QCommandLineParser>
#include <QDesktopWidget>
#include <QLoggingCategory>
#include <QTranslator>
#include <QJsonObject>
#include <QJsonDocument>
#include <QScreen>

#include "category-manager.h"
#include "config.h"
#include "panel-window.h"
#include "plugin-manager.h"

using namespace std;

bool installTranslator()
{
    QTranslator* tsor = new QTranslator(qApp);
    if (!tsor->load(QLocale(),
                   qAppName(),
                   ".",
                   TRANSLATE_PREFIX,
                   ".qm"))
    {
        KLOG_ERROR() << "kiran-control-panel load translator failed!";
        return false;
    }

    QCoreApplication::installTranslator(tsor);
    return true;
}

QString defaultCategory = "about-system";
QString defaultSubItem  = "SystemInformation";
bool listAllPluginInfo = false;

void processCommandLine()
{
    KiranSingleApplication* singleApp = dynamic_cast<KiranSingleApplication*>(qApp);
    if( !singleApp )
    {
        std::cerr << "cast to KiranSingleApplication failed!" << std::endl;
        return;
    }

    QCommandLineParser cmdParser;
    QCommandLineOption categoryOption("c","主面板进入哪个分类","category");
    QCommandLineOption subItemOption("s","分类下的哪个子项","subItem");
    QCommandLineOption listInfoOption("l","列举出所有的子功能项");

    cmdParser.addHelpOption();
    cmdParser.addOptions({categoryOption,subItemOption,listInfoOption});
    cmdParser.process(*singleApp);

    QString category = cmdParser.value(categoryOption);
    category = category.trimmed();

    QString subItem = cmdParser.value(subItemOption);
    subItem = subItem.trimmed();

    if( !subItem.isEmpty() && category.isEmpty() )
    {
        std::cerr << "failed to set sub item without category" << std::endl;
        exit(EXIT_FAILURE);
    }

    if( !category.isEmpty() )
    {
        if( !singleApp->isPrimary() )
        {
            QJsonObject jumpInfoObject({{"category", category},{"subitem",subItem}});
            QJsonDocument jumpInfoDoc(jumpInfoObject);
            QByteArray byteArray = jumpInfoDoc.toJson();
            singleApp->sendMessage(byteArray);
        }
        else
        {
            defaultCategory = category;
            defaultSubItem = subItem;
        }
    }

    if( cmdParser.isSet(listInfoOption) )
    {
        listAllPluginInfo = true;
    }
}

void dumpPluginManagerInfo()
{
#if 0
    auto pluginManager = PluginManager::getInstance();
    for(auto category:pluginManager->getCategorys())
    {
        auto categoryInfo = category->getCategoryDesktopInfo();
        fprintf(stdout,"category id: <%s> name: <%s>\n",
                categoryInfo.categoryName.toStdString().c_str(),
                categoryInfo.name.toStdString().c_str());

        for(auto plugin:category->getPlugins())
        {
            auto pluginInfo = plugin->getPluginDesktopInfo();
            fprintf(stdout,"\tplugin name: <%s>\n",pluginInfo.name.toStdString().c_str());

            auto subItemsInfo = pluginInfo.subItems;
            for(auto subItem:subItemsInfo)
            {
                fprintf(stdout,"\t\tsubitem id: <%s> name: <%s> keywords: <%s>\n",
                        subItem.id.toStdString().c_str(),
                        subItem.name.toStdString().c_str(),
                        subItem.keywords.join(",").toStdString().c_str());
            }
        }
    }
#endif
}

#if (QT_VERSION < QT_VERSION_CHECK(5, 10, 0))
static QScreen *screenAt(const QPoint &point)
{
    QVarLengthArray<const QScreen *, 8> visitedScreens;
    for (const QScreen *screen : QGuiApplication::screens()) {
        if (visitedScreens.contains(screen))
            continue;

        // The virtual siblings include the screen itself, so iterate directly
        for (QScreen *sibling : screen->virtualSiblings()) {
            if (sibling->geometry().contains(point))
                return sibling;

            visitedScreens.append(sibling);
        }
    }

    return nullptr;
}
#endif

int main(int argc, char *argv[])
{
    // 用户内单例可能存在同一用户登录多个图形会话（本地+远程）
    // 加入DISPLAY信息一起做进程单例的判断，确保一个会话内只存在一个
    // XDG_SESSION_ID不准确，tigervnc vncserver/vncsession 手动拉不会创logind会话
    KiranSingleApplication::addApplicationIDUserData(qgetenv("DISPLAY"));
    KiranSingleApplication app(argc,
                             argv,
                             true,
                             KiranSingleApplication::Mode::User | KiranSingleApplication::Mode::SecondaryNotification);

    // 初始化日志库
    int iret = klog_qt5_init("", "kylinsec-session", "kiran-control-panel", "kiran-control-panel");
    if (iret != 0)
    {
        fprintf(stderr, "klog_qt5_init faield,res:%d\n", iret);
    }

    // 处理命令行参数
    processCommandLine();
    if( !app.isPrimary() )
    {
        exit(EXIT_SUCCESS);
    }

    /// NOTE: 由于strftime获取系统locale进行格式化，Qt使用UTF8,若编码设置不为UTF8中文环境下会导致乱码
    /// 所以LANG后面的编码若不为UTF-8,修改成UTF-8,使获取时间都为UTF-8格式
    QString lang = qgetenv("LANG");
#if (QT_VERSION < QT_VERSION_CHECK(5, 14, 0))
    QStringList splitRes = lang.split(".", QString::SkipEmptyParts);
#else
    QStringList splitRes = lang.split(".", Qt::SkipEmptyParts);
#endif
    if( (splitRes.size() == 1) || (splitRes.size() == 2 && splitRes.at(1)!="UTF-8") )
    {
        QString newLangEnv = QString("%1.UTF-8").arg(splitRes.at(0));
        setlocale(LC_TIME, newLangEnv.toStdString().c_str());
    }

    // 安装翻译
    installTranslator();

    PluginManager* pManager = PluginManager::instance();
    pManager->init();

    CategoryManager* cManager = CategoryManager::instance();
    cManager->init();

    // 输出所有插件，所有功能子项的信息
    if( listAllPluginInfo )
    {
        cManager->dump();
        exit(EXIT_SUCCESS);
    }

    PanelWindow w;
    w.jump(defaultCategory,defaultSubItem);

#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
    auto screen = QApplication::screenAt(QCursor::pos());
#else
    auto screen = screenAt(QCursor::pos());
#endif
    QRect screenGeometry = screen->geometry();
    w.resize(1031, 742);
    w.move(screenGeometry.x() + (screenGeometry.width() - w.width()) / 2,
           screenGeometry.y() + (screenGeometry.height() - w.height()) / 2);
    w.show();
    w.setContentWrapperMarginBottom(0);

    int execRet = app.exec();
    return execRet;
}
