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

#include "config.h"
#include "launcher.h"
#include "plugin-v1-subitem-wrapper.h"
#include "plugin-v1.h"
#include "plugin-v2.h"

#include <kiran-single-application.h>
#include <locale.h>
#include <qt5-log-i.h>
#include <QCommandLineParser>
#include <QDesktopWidget>
#include <QEvent>
#include <QIcon>
#include <QLayout>
#include <QProcess>
#include <QLoggingCategory>
#include <QScreen>
#include <QStyleFactory>
#include <QTranslator>
#include <iostream>

// NOTE:
// 2.4版本之后，kiran-cpanel-launcher已不提供单独启动控制中心插件的功能
// 保留launcher只是为了兼容，转发拉起控制中心

int main(int argc, char* argv[])
{
    /// 先将插件选项从参数中提取出来,作为校验进程单例的一部分
    QStringList arguments;
    for (int i = 0; i < argc; i++)
    {
        arguments << argv[i];
    }
    QString pluginName;
    QCommandLineOption pluginOption("cpanel-plugin", "plugin desktop filename", "plugin", "");
    QCommandLineParser parser;
    parser.setApplicationDescription("kiran control panel module runalone");
    parser.addOption(pluginOption);
    QCommandLineOption helpOption = parser.addHelpOption();
    parser.parse(arguments);
    if (parser.isSet(pluginOption))
    {
        pluginName = parser.value(pluginOption);
        KiranSingleApplication::addApplicationIDUserData(pluginName);
    }

    KiranSingleApplication app(argc, argv, false,
                               KiranSingleApplication::Mode::User |
                                   KiranSingleApplication::Mode::SecondaryNotification);

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

    // 为了保持插件使用启动器进行启动后，底部面板不堆叠，插件图标显示正常，
    // 设置ApplicationName,更新窗口WM_CLASS属性为插件desktop名称
    if (!pluginName.isEmpty())
    {
        QApplication::setApplicationName(pluginName);
    }

    /// 再次解析命令行参数是为了处理--help选项得到正确的输出
    parser.addHelpOption();
    parser.process(app);

    QTranslator translator;
    QLocale locale;
    if (translator.load(locale,
                        qAppName(),
                        ".",
                        TRANSLATE_PREFIX,
                        ".qm"))
    {
        qApp->installTranslator(&translator);
    }
    else
    {
        QString qmFile = QString("%1/%2%3%4%5").arg(TRANSLATE_PREFIX).arg(qAppName()).arg(".").arg(locale.name()).arg(".qm");
        KLOG_ERROR() << "can't load translator!" << qmFile;
    }

    QVector<KiranControlPanel::SubItemPtr> pluginSubItems;


    //兼容两个版本
    // plugin v1接口通过desktop文件拿到信息再找so
    QString pluginDesktopPath = QString("%1/%2").arg(PLUGIN_DESKTOP_DIR).arg(pluginName);
    if (!pluginDesktopPath.endsWith(".desktop"))
    {
        pluginDesktopPath.append(".desktop");
    }

    // plugin v2接口直接加载so读取信息
    QString pluginV2LibraryPath = QString("%1/lib%2.so").arg(PLUGIN_LIBRARY_DIR).arg(pluginName);

    PluginV1 plugin;
    PluginV2 pluginV2;
    if (plugin.load(pluginDesktopPath))
    {
        pluginSubItems = plugin.getSubItems();
    }
    else if (pluginV2.load(pluginV2LibraryPath))
    {
        pluginSubItems = pluginV2.getSubItems();
    }


    if( pluginSubItems.isEmpty() )
    {
        KLOG_CERR("plugin name(%s),can't find plugin subitem",pluginName.toStdString().c_str());
        exit(EXIT_FAILURE);
    }

    QStringList positionArgs;
    positionArgs << QString("-c %1").arg(pluginSubItems.at(0)->getCategory());
    positionArgs << QString("-s %2").arg(pluginSubItems.at(0)->getName());
    return QProcess::startDetached(PANEL_FULL_PATH,positionArgs);
}
