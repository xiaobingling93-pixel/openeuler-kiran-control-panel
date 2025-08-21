[TOC]

# kiran控制中心(kiran-control-panel)

## 基本使用
### 编译
1. 安装编译依赖

   ```bash
   $ sudo yum install gcc-c++ qt5-qtbase qt5-qtbase-devel qt5-qtbase-gui qt5-qtx11extras qt5-qtx11extras-devel qt5-qtsvg glibc glibc-devel libX11 libX11-devel kiranwidgets-qt5 kiran-widgets-qt5-devel qt5-qtbase-static qt5-qtbase-private-devel group-service libxkbcommon-devel
   ```

2. **源码根目录下创建**build**目录`mkdir build`

   ```bash
   $ mkdir build
   ```

3. 进入**build**目录,执行下列命令即可进行debug版本生成**Makefile**

   ```bash
   $ cmake -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=debug ..
   ```

4. 执行`make`进行编译

   ```bash
   $ make -j4
   ```
### 安装

1. 编译之后，在**build**目录下执行下述命令即可

   ```bash
   $ sudo make install
   ```

### 运行

- 启动主面板(加载所有插件)

```bash
$ kiran-control-panel
```

- 启动启动器(指定加载插件)

```bash
$ kiran-cpanel-launcher --cpanel-plugin kiran-cpanel-xxx
```

## 分类(Category)、插件(Plugin)、子功能项(SubItem)

### 分类(Category)

分类(Category)指的是用于**控制面板最左侧的功能项列表**的多个配置文件，提供给插件一个父节点(**所有的插件都应该指向一个分类**)，分类配置文件暂时放置于**/usr/share/kiran-control-panel/category/desktop/**(更推荐去kiran-control-panel-devel提供的pkgconfig文件中拿到相关)。

以下为分类配置文件格式:

```ini
[Desktop Entry]
#用作显示的主分类名，需提供翻译，控制中心根据当前语言环境选择合适的翻译形成左侧的列表
Name=About Systems
Name[zh_CN]=关于系统
#用作主分类节点的悬浮显示，控制中心根据当前语言环境选择合适的翻译形成左侧的列表节点悬浮提示框
Comment=About Systems
Comment[zh_CN]=关于系统
#用作主分类节点的图标，控制中心通过该图标形成左侧分类的图标，图标按规范放置的话可以为图标名，也可以为绝对路径
Icon=about-systems.png

#该组(Group)下均为提供给控制面板或其启动器的相关信息
[Kiran Control Panel Category]
#主分类的ID，插件需指定主分类的ID
Category=about-system
#主分类权重，控制面板将会根据该值的大小决定哪项在第一个
Weight=10
#主分类关键词,提供给控制面板用于检索主分类，与语言有关，需提供翻译
Keywords[zh_CN]=关于系统
Keywords=About Systems
```

以下为控制面板暂时提供的预安装的默认分类：

- about-system    
  关于系统
- account-management    
  帐户管理
- timedate    
  日期时间
- display    
  显示设置
- hardware    
  硬件相关
- individuation    
  个性化
- login-settings    
  登录设置
- network    
  网络相关
- power-manager    
  电源相关
- regional-language    
  区域语言

### 插件(Plugin)

插件(Plugin)指的是一个实现了控制面板接口的共享库，一个完整的插件至少需要提供**desktop文件**和**插件实现共享库**，一个插件可能提供一个分类(Category)下的多个子功能项，若一个分类下的所有插件的子功能项数量**小于等于1**的话，则**子功能项列表**则会**隐藏** 。

```ini
[Desktop Entry]
#插件名，启动器根据语言环境获取该值作为窗口标题，同时也做快捷方式的显示名
Name=Plugin Demo
Name[zh_CN]=插件demo
Comment=Plugin Demo
Comment[zh_CN]=插件demo
#插件图标,同时也是快捷方式图标
Icon=plugin-demo.svg
# 配置通过启动器单独执行，需传入插件的desktop文件名，启动器获取该dekstop文件位置加载相关信息 
# <kiran-cpanel-launcher -cpanel-plugin 插件的desktop文件名>
Exec=kiran-cpanel-launcher --cpanel-plugin=kiran-cpanel-demo
Categories=Settings;System;X-Common-Features;X-Common-Features;
Terminal=false
Type=Application
X-KIRAN-NoDisplay=true

#插件信息存储的组（Group)
[Kiran Control Panel Plugin]
#插件排序
Weight=3
#插件分类
Category=individuation
#插件共享库位置，若以'/'开头则为绝对路径，也可支持通过共享库名去插件安装目录去查找
Library=lib@PROJECT_NAME@.so
#功能子项，控制面板会根据该值对下列的功能子项Group进行解析
SubItems=Dialog001,Dialog002

#功能子项名
[Dialog001]
#功能子项名 该值提供给控制中心形成功能子项列表
Name[zh_CN]=弹窗001
#插件Icon
Icon=dialog001.svg
#插件关键字 该值提供给控制中心形成搜索列表
Keywords[zh_CN]=Dialog,Dialog001,001

[Dialog002]
Name[zh_CN]=弹窗002
Icon=dialog002.svg
Keywords[zh_CN]=Dialog,Dialog001,001
```

### 子功能项(SubItem)

子功能项(SubItem)主要是描述插件提供的功能项列表，一个分类(Category)下的子功能项可能由多个插件合并显示的，子功能项信息存储在插件desktop文件中，方便后期使用用作全局搜索

## 如何写一个插件

### 1. 编写插件desktop信息

```ini
[Desktop Entry]
#插件名，启动器根据语言环境获取该值作为窗口标题，同时也做快捷方式的显示名
Name=Plugin Demo
Name[zh_CN]=插件demo
Comment=Plugin Demo
Comment[zh_CN]=插件demo
#插件图标,同时也是快捷方式图标
Icon=plugin-demo.svg
# 配置通过启动器单独执行，需传入插件的desktop文件名，启动器获取该dekstop文件位置加载相关信息 
# <kiran-cpanel-launcher -cpanel-plugin 插件的desktop文件名>
Exec=kiran-cpanel-launcher --cpanel-plugin=kiran-cpanel-demo
Categories=Settings;System;X-Common-Features;X-Common-Features;
Terminal=false
Type=Application
X-KIRAN-NoDisplay=true

#插件信息存储的组（Group)
[Kiran Control Panel Plugin]
#插件排序
Weight=3
#插件分类
Category=individuation
#插件共享库位置，若以'/'开头则为绝对路径，也可支持通过共享库名去插件安装目录去查找
Library=lib@PROJECT_NAME@.so
#功能子项，控制面板会根据该值对下列的功能子项Group进行解析
SubItems=SubItem1

#功能子项名
[SubItem1]
#功能子项名 该值提供给控制中心形成功能子项列表
Name[zh_CN]=SubItem1
#插件Icon
Icon=kcp-demo-item1
#插件关键字 该值提供给控制中心形成搜索列表
Keywords[zh_CN]=SubItem1
```

### 2. 实现控制面板插件接口

:star: tips: **插件和控制面板**(包括启动器)的两个二进制程序**编译过程中需保证所使用的接口版本一致**。接口版本定义在头文件中kcp-plugin-interface.h中**KcpPluginInterface_iid**中所定义。控制面板和启动器不会加载实现接口版本不同的插件!

**接口定义:**

```c++
#ifndef KIRAN_CONTROL_PANEL_INCLUDE_KCP_PLUGIN_INTERFACE_H_
#define KIRAN_CONTROL_PANEL_INCLUDE_KCP_PLUGIN_INTERFACE_H_

#include <QString>
#include <QWidget>

//插件抽象接口
class KcpPluginInterface
{
public:
virtual ~KcpPluginInterface(){};
public:
/**
* 插件需提供的初始化方法，在其中加载翻译文件或做其他初始化操作
* \return 初始化返回值 返回0标志成功，其他值标志失败！
*/
virtual int init() = 0;

/**
* 插件需提供取消初始化的操作，在其中对翻译文件进行卸载或取消其他初始化操作
*/
virtual void uninit() = 0;

/**
* \brief 通过插件功能项名称(PluginSubItem->name)获取显示控件
* \param id 功能项ID
* \return 该功能项的显示控件
*/
virtual QWidget* getSubItemWidget(QString id) = 0;

/**
* 插件实现该方法用于判断是否存在未保存的设置项,用于提供切换页面时做检查
* \return 是否存在未保存项
*/
virtual bool haveUnsavedOptions() = 0;

/**
* 获取应该显示的子功能项
* \param id 功能项ID
* \return 显示的子功能项列表
*/
virtual QStringList visibleSubItems() = 0;
};

#define KcpPluginInterface_iid "com.kylinsec.Kiran.ControlPanelInterface/1.0"
Q_DECLARE_INTERFACE(KcpPluginInterface,KcpPluginInterface_iid)

#endif  //KIRAN_CONTROL_PANEL_INCLUDE_KCP_PLUGIN_INTERFACE_H_
```

接口实现实例:

```c++
#ifndef INTERFACE_H
#define INTERFACE_H

#include <kiran-control-panel/kcp-plugin-interface.h>

class PluginDemoInterface : public QObject,public KcpPluginInterface
{
    //定义Q_OBJECT进行moc元对象编译
    Q_OBJECT
    //定义插件元数据，声明该类实现的接口IID
    Q_PLUGIN_METADATA(IID KcpPluginInterface_iid)
    //申明该Qt类实现了哪些接口
    Q_INTERFACES(KcpPluginInterface)
public:
    ~PluginDemoInterface(){};
    int init() override;
    void uninit() override;

    QWidget* getSubItemWidget(QString subItemName) override;
    bool haveUnsavedOptions() override;
    QStringList visibleSubItems() override;
};
#endif  // INTERFACE_H
```

### 3. 通过devel包所提供的pkgconfig配置文件找到插件安装相应的位置

```cmake
#通过kiran control panel的pkgconfig配置文件取出插件Desktop安装位置、插件共享库安装位置
find_package(PkgConfig REQUIRED)
pkg_search_module(KIRAN_CONTROL_PANEL_PKG REQUIRED kiran-control-panel)
pkg_get_variable(CPANEL_PLUGIN_DIR kiran-control-panel plugin_location)
pkg_get_variable(CPANEL_DESKTOP_DIR kiran-control-panel plugin_desktop_location)
```

### 4. 编译、安装之后，可通过启动控制面板或启动器单独加载启动

启动控制面板

```bash
$ kiran-control-panel
```

通过启动器启动

```bash
$ kiran-cpanel-launcher --cpanel-plugin 插件安装的desktop文件名
```
