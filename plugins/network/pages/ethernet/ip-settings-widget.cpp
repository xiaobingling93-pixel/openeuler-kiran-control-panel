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
 * Author:     liuxinhao <liuxinhao@kylinsec.com.cn>
 */
#include "ip-settings-widget.h"
#include <qt5-log-i.h>
#include <QEvent>
#include <QMetaObject>
#include <QRegExp>
#include <QRegExpValidator>
#include "kiran-tips/kiran-tips.h"
#include "logging-category.h"
#include "ui_ip-settings-widget.h"

#define RETURN_WITH_ERROR_TIPS(text, widget) \
    {                                        \
        m_tips->setText(text);               \
        m_tips->showTipAroundWidget(widget); \
        return false;                        \
    }

Q_DECLARE_METATYPE(NetworkManager::Ipv4Setting::ConfigMethod)
Q_DECLARE_METATYPE(NetworkManager::Ipv6Setting::ConfigMethod)

namespace Kiran
{
namespace Network
{
using namespace NetworkManager;
enum IPSettingsPageIndex
{
    IP_SETTINGS_PAGE_INDEX_IPV4 = 0,
    IP_SETTINGS_PAGE_INDEX_IPV6 = 1
};

IpSettingsWidget::IpSettingsWidget(QWidget *parent)
    : QWidget(parent),
      ui(new ::Ui::IpSettingsWidget)
{
    ui->setupUi(this);
    init();
}

IpSettingsWidget::~IpSettingsWidget()
{
    delete ui;
}

void IpSettingsWidget::reset()
{
    ui->combo_method->setCurrentIndex(0);

    ui->edit_ipv4_ip->clear();
    ui->edit_ipv4_mask->clear();
    ui->edit_ipv4_gateway->clear();
    ui->edit_ipv4_dns->clear();

    ui->edit_ipv6_ip->clear();
    ui->edit_ipv6_prefix->clear();
    ui->edit_ipv6_gateway->clear();
    ui->edit_ipv6_dns->clear();

    m_ipv4Setting.clear();
    m_ipv6Setting.clear();
}

void IpSettingsWidget::loadSettings(NetworkManager::Ipv4Setting::Ptr setting)
{
    reset();

    if (m_type != TYPE_IPV4)
    {
        KLOG_WARNING(qLcNetwork, "set ipv4 setting failed, type invalid: %d\n", m_type);
        return;
    }

    m_ipv4Setting = setting;

    // config method
    loadConfigMethodComboBox();
    auto method = m_ipv4Setting->method();
    auto method_idx = ui->combo_method->findData(method);
    if (method_idx != -1)
    {
        ui->combo_method->setCurrentIndex(method_idx);
    }

    // ip/mask/gateway
    QStringList ips;
    QStringList masks;
    QString gateyway;
    auto addresses = m_ipv4Setting->addresses();
    for (auto address : addresses)
    {
        ips << address.ip().toString();
        masks << address.netmask().toString();

        QString temp = address.gateway().toString();
        if (temp != "0.0.0.0")
        {
            gateyway = temp;
        }
    }
    ui->edit_ipv4_ip->setText(ips.join(";"));
    ui->edit_ipv4_mask->setText(masks.join(";"));
    ui->edit_ipv4_gateway->setText(gateyway);

    // dns
    QString dnsString = "";
    if (!m_ipv4Setting->dns().isEmpty())
    {
        QStringList dnsList;
        auto hostAddressList = m_ipv4Setting->dns();
        for (auto address : hostAddressList)
        {
            dnsList << address.toString();
        }
        dnsString = dnsList.join(";");
    }
    ui->edit_ipv4_dns->setText(dnsString);
}

void IpSettingsWidget::loadSettings(NetworkManager::Ipv6Setting::Ptr setting)
{
    if (m_type != TYPE_IPV6)
    {
        KLOG_WARNING(qLcNetwork, "set ipv6 setting failed, type invalid: %d\n", m_type);
        return;
    }

    reset();

    m_ipv6Setting = setting;

    // config method
    loadConfigMethodComboBox();
    auto method = m_ipv6Setting->method();
    auto method_idx = ui->combo_method->findData(method);
    if (method_idx != -1)
    {
        ui->combo_method->setCurrentIndex(method_idx);
    }

    // ip/prefix/gateway
    QStringList ips;
    QStringList prefixs;
    QString gateway;
    auto addresses = m_ipv6Setting->addresses();
    for (auto address : addresses)
    {
        ips << address.ip().toString();
        prefixs << QString::number(address.prefixLength());

        if (gateway.isEmpty() &&
            !address.gateway().isNull() &&
            address.gateway() != QHostAddress::AnyIPv6)
        {
            gateway = address.gateway().toString();
        }
    }
    ui->edit_ipv6_ip->setText(ips.join(";"));
    ui->edit_ipv6_prefix->setText(prefixs.join(";"));
    ui->edit_ipv6_gateway->setText(gateway);

    // dns
    QString dnsString = "";
    if (!m_ipv6Setting->dns().isEmpty())
    {
        QStringList dnsList;
        auto hostAddressList = m_ipv6Setting->dns();
        for (auto address : hostAddressList)
        {
            dnsList << address.toString();
        }
        dnsString = dnsList.join(";");
    }
    ui->edit_ipv6_dns->setText(dnsString);
}

bool IpSettingsWidget::checkValid()
{
    if (m_type == TYPE_IPV4)
    {
        return checkIpv4Valid();
    }
    else if (m_type == TYPE_IPV6)
    {
        return checkIpv6Valid();
    }

    return false;
}

void IpSettingsWidget::save()
{
    switch (m_type)
    {
    case TYPE_IPV4:
        saveIpv4Settings();
        break;
    case TYPE_IPV6:
        saveIpv6Settings();
        break;
    default:
        break;
    }
}

void IpSettingsWidget::setTipsWidget(KiranTips *tips)
{
    m_tips = tips;
}

/// @brief 根据设置IP地址类型初始化界面
/// @param type IPv4或IPv6
void IpSettingsWidget::setIpSettingsType(Type type)
{
    if (type >= TYPE_LAST)
    {
        KLOG_WARNING(qLcNetwork, "set ip settings widget type failed, type invalid: %d", type);
        return;
    }

    m_type = type;
    ui->label_method->setText(type == TYPE_IPV4 ? tr("IPv4 Method") : tr("IPv6 Method"));
    ui->stackedWidget->setCurrentIndex(type);

    loadConfigMethodComboBox();
    processConfigMethodChanged();
}

void IpSettingsWidget::init()
{
    connect(ui->combo_method,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &IpSettingsWidget::processConfigMethodChanged);
    setIpSettingsType(TYPE_IPV4);
}

void IpSettingsWidget::loadConfigMethodComboBox()
{
    QMap<int, QString> configMethodV4Map = {
        {Ipv4Setting::Automatic, tr("Automatic")},
        {Ipv4Setting::LinkLocal, tr("LinkLocal")},
        {Ipv4Setting::Manual, tr("Manual")},
        {Ipv4Setting::Shared, tr("Shared")},
        {Ipv4Setting::Disabled, tr("Disabled")}};

    QMap<int, QString> configMethodV6Map = {
        {Ipv6Setting::Automatic, tr("Automatic")},
        {Ipv6Setting::Dhcp, tr("Dhcp")},
        {Ipv6Setting::LinkLocal, tr("LinkLocal")},
        {Ipv6Setting::Manual, tr("Manual")},
        {Ipv6Setting::Ignored, tr("Ignored")},
        {Ipv6Setting::ConfigDisabled, tr("ConfigDisabled")}};

    // 默认只展示自动/手动/禁用
    QSet<int> ipv4DefaultMethods = {
        Ipv4Setting::ConfigMethod::Automatic,
        Ipv4Setting::ConfigMethod::Manual,
        Ipv4Setting::ConfigMethod::Disabled};

    QSet<int> ipv6DefaultMethods = {
        Ipv6Setting::ConfigMethod::Automatic,
        Ipv6Setting::ConfigMethod::Manual,
        Ipv6Setting::ConfigMethod::Ignored};  // ConfigDisabled完全禁用IPv6, Ignore忽略当前IPv6配置

    const auto &defaultMethod = m_type == TYPE_IPV4 ? ipv4DefaultMethods : ipv6DefaultMethods;
    const auto &allMethodMap = m_type == TYPE_IPV4 ? configMethodV4Map : configMethodV6Map;

    ui->combo_method->clear();

    for (auto iter = defaultMethod.begin(); iter != defaultMethod.end(); ++iter)
    {
        auto methodEnum = *iter;
        auto text = allMethodMap[methodEnum];
        ui->combo_method->addItem(text, methodEnum);
    }

    // 当前配置不存在该方法，添加进入下拉框
    int settingsMethod = -1;
    if (m_type == TYPE_IPV4 && m_ipv4Setting)
    {
        settingsMethod = m_ipv4Setting->method();
    }
    else if (m_type == TYPE_IPV6 && m_ipv6Setting)
    {
        settingsMethod = m_ipv6Setting->method();
    }

    const auto methodInComboBox = (ui->combo_method->findData(settingsMethod) != -1);
    if (!methodInComboBox && allMethodMap.contains(settingsMethod))
    {
        ui->combo_method->addItem(allMethodMap[settingsMethod], settingsMethod);
    }
}

bool IpSettingsWidget::isStaticIPConfigUIVisible(Ipv4Setting::ConfigMethod method)
{
    static const QSet<Ipv4Setting::ConfigMethod> ipConfigMethod = {Ipv4Setting::Manual};
    return ipConfigMethod.contains(method);
}

bool IpSettingsWidget::isStaticIPConfigUIVisible(Ipv6Setting::ConfigMethod method)
{
    static const QSet<Ipv6Setting::ConfigMethod> ipConfigMethod = {Ipv6Setting::Manual};
    return ipConfigMethod.contains(method);
}

bool IpSettingsWidget::isDNSConfigUIVisible(NetworkManager::Ipv4Setting::ConfigMethod method)
{
    static const QSet<Ipv4Setting::ConfigMethod> dnsConfigMethod = {Ipv4Setting::Automatic,
                                                                    Ipv4Setting::Manual};
    return dnsConfigMethod.contains(method);
}

bool IpSettingsWidget::isDNSConfigUIVisible(NetworkManager::Ipv6Setting::ConfigMethod method)
{
    static const QSet<Ipv6Setting::ConfigMethod> dnsConfigMethod = {Ipv6Setting::Automatic,
                                                                    Ipv6Setting::Dhcp,
                                                                    Ipv6Setting::Manual};
    return dnsConfigMethod.contains(method);
}

bool IpSettingsWidget::ipv4AddressValid(const QString &address)
{
    QHostAddress ipAddr(address);
    if (ipAddr == QHostAddress(QHostAddress::Null) || ipAddr == QHostAddress(QHostAddress::AnyIPv4) || ipAddr.protocol() != QAbstractSocket::NetworkLayerProtocol::IPv4Protocol)
    {
        return false;
    }
    QRegExp regExpIP("((25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9][0-9]|[0-9])[\\.]){3}(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9][0-9]|[0-9])");
    return regExpIP.exactMatch(address);
}

bool IpSettingsWidget::ipv4MaskValid(const QString &address)
{
    bool done;
    quint32 mask = QHostAddress(address).toIPv4Address(&done);

    if (!done)
    {
        return false;
    }

    for (; mask != 0; mask <<= 1)
    {
        if ((mask & (static_cast<uint>(1) << 31)) == 0)
            return false;  // Highest bit is now zero, but mask is non-zero.
    }

    QRegExp regExpIP("^((128|192)|2(24|4[08]|5[245]))(\\.(0|(128|192)|2((24)|(4[08])|(5[245])))){3}$");
    return regExpIP.exactMatch(address);
}

bool IpSettingsWidget::checkIpv6Valid()
{
    const auto configMethod = ui->combo_method->currentData().toInt();

    // 检查手动静态IP配置
    if (configMethod == Ipv6Setting::ConfigMethod::Manual)
    {
        // 检查地址和前缀
        const auto addresses = ui->edit_ipv6_ip->text();
        if (addresses.isEmpty())
        {
            RETURN_WITH_ERROR_TIPS(tr("Please enter the IPv6 address."), ui->edit_ipv6_ip);
        }

        const auto prefixs = ui->edit_ipv6_prefix->text();
        if (prefixs.isEmpty())
        {
            RETURN_WITH_ERROR_TIPS(tr("Please enter the IPv6 prefix."), ui->edit_ipv6_prefix);
        }

        const auto addressList = addresses.split(";");
        const auto prefixList = prefixs.split(";");
        if (addressList.size() < prefixList.size())
        {
            RETURN_WITH_ERROR_TIPS(tr("The address does not match the prefix number"), ui->edit_ipv6_ip);
        }
        else if (addressList.size() > prefixList.size())
        {
            RETURN_WITH_ERROR_TIPS(tr("The prefix does not match the number of addresses"), ui->edit_ipv6_prefix);
        }

        for (auto address : addressList)
        {
            if (!ipv6AddressValid(address))
            {
                RETURN_WITH_ERROR_TIPS(tr("Invalid address, please re-enter"), ui->edit_ipv6_ip);
            }
        }

        for (auto prefix : prefixList)
        {
            if (!ipv6PrefixValid(prefix))
            {
                RETURN_WITH_ERROR_TIPS(tr("Invalid prefix, please re-enter"), ui->edit_ipv6_prefix);
            }
        }

        // 检查网关
        const auto gateway = ui->edit_ipv6_gateway->text();
        if (!gateway.isEmpty() && !ipv6AddressValid(gateway))
        {
            RETURN_WITH_ERROR_TIPS(tr("Invalid gateway, please re-enter"), ui->edit_ipv6_gateway);
        }
    }

    // 检查dns配置
    if (configMethod != Ipv6Setting::Ignored && !ui->edit_ipv6_dns->text().isEmpty())
    {
        const auto dnsList = ui->edit_ipv4_dns->text().split(";");
        for (auto dns : dnsList)
        {
            if (!ipv6AddressValid(dns))
            {
                RETURN_WITH_ERROR_TIPS(tr("Invalid dns, please re-enter"), ui->edit_ipv4_dns);
            }
        }
    }

    return true;
}

bool IpSettingsWidget::ipv6AddressValid(const QString &address)
{
    QHostAddress ipAddr(address);
    if (ipAddr == QHostAddress(QHostAddress::Null) || ipAddr == QHostAddress(QHostAddress::AnyIPv6) || ipAddr.protocol() != QAbstractSocket::NetworkLayerProtocol::IPv6Protocol)
    {
        return false;
    }
    if (ipAddr == QHostAddress(QHostAddress::LocalHostIPv6))
    {
        return false;
    }
    return true;
}

bool IpSettingsWidget::ipv6PrefixValid(const QString &prefix)
{
    bool ok = false;
    int tempPrefix = prefix.toUInt(&ok);
    if (tempPrefix > 0 && tempPrefix < 129)
    {
        return true;
    }
    return false;
}

void IpSettingsWidget::saveIpv4Settings()
{
    const auto configMethod = ui->combo_method->currentData().value<NetworkManager::Ipv4Setting::ConfigMethod>();

    m_ipv4Setting->setInitialized(true);
    m_ipv4Setting->setMethod(configMethod);

    // 写入静态地址
    if (configMethod == Ipv4Setting::Manual)
    {
        QList<IpAddress> addressList;
        const auto gateway = ui->edit_ipv4_gateway->text();
        const auto maskList = ui->edit_ipv4_mask->text().split(';');
        const auto ipList = ui->edit_ipv4_ip->text().split(';');

        for (int i = 0; i < ipList.count(); i++)
        {
            IpAddress address;
            address.setIp(QHostAddress(ipList.at(i)));
            address.setNetmask(QHostAddress(maskList.at(i)));  // 必须先设定地址，在设置掩码, qt代码会有检测
            address.setGateway(QHostAddress(gateway));
            addressList << address;
        }
        m_ipv4Setting->setAddresses(addressList);
    }
    else
    {
        // 非手动配置清理静态地址配置
        m_ipv4Setting->setAddresses({IpAddress()});
    }

    // 写入DNS配置
    m_ipv4Setting->setDns({});
    if (configMethod != Ipv4Setting::Disabled)
    {
        const auto dnsList = ui->edit_ipv4_dns->text().split(';', Qt::SkipEmptyParts);
        QList<QHostAddress> dnsSettings;
        for (auto dnsString : dnsList)
        {
            dnsSettings << QHostAddress(dnsString);
        }
        m_ipv4Setting->setDns(dnsSettings);
    }

    KLOG_INFO(qLcNetwork) << "save ipv4 settings" << m_ipv4Setting->toMap();
}

void IpSettingsWidget::saveIpv6Settings()
{
    const auto configMethod = ui->combo_method->currentData().value<NetworkManager::Ipv6Setting::ConfigMethod>();

    m_ipv6Setting->setMethod(configMethod);
    m_ipv6Setting->setInitialized(true);

    if (configMethod == Ipv6Setting::Manual)
    {
        QList<IpAddress> addresses;
        QHostAddress gateway = QHostAddress(ui->edit_ipv6_gateway->text());
        const auto ipList = ui->edit_ipv6_ip->text().split(';');
        const auto prefixList = ui->edit_ipv6_prefix->text().split(';');
        for (int i = 0; i < ipList.count(); i++)
        {
            auto prefix = prefixList.at(i).toUInt();
            IpAddress address;
            address.setIp(QHostAddress(ipList.at(i)));
            address.setPrefixLength(prefix);
            address.setGateway(gateway);
            addresses << address;
        }
        m_ipv6Setting->setAddresses(addresses);
    }
    else
    {
        m_ipv6Setting->setAddresses({IpAddress()});
    }

    m_ipv6Setting->setDns({});
    const auto dnsList = ui->edit_ipv6_dns->text().split(';', Qt::SkipEmptyParts);
    if (configMethod != Ipv6Setting::Ignored && !dnsList.isEmpty())
    {
        QList<QHostAddress> dnsSettings;
        for (auto dnsString : dnsList)
        {
            dnsSettings << QHostAddress(dnsString);
        }
        m_ipv6Setting->setDns(dnsSettings);
    }

    KLOG_INFO(qLcNetwork) << "save ipv6 settings" << m_ipv6Setting->toMap();
}

bool IpSettingsWidget::checkIpv4Valid()
{
    const auto configMethod = ui->combo_method->currentData().toInt();

    // 检查手动静态IP配置
    if (configMethod == Ipv4Setting::ConfigMethod::Manual)
    {
        // 检查ip地址和子网掩码
        const auto addresses = ui->edit_ipv4_ip->text();
        if (addresses.isEmpty())
        {
            RETURN_WITH_ERROR_TIPS(tr("Please enter the IPv4 address."), ui->edit_ipv4_ip);
        }

        const auto masks = ui->edit_ipv4_mask->text();
        if (masks.isEmpty())
        {
            RETURN_WITH_ERROR_TIPS(tr("Please enter the IPv4 subnet mask."), ui->edit_ipv4_mask);
        }

        const auto addressList = addresses.split(";");
        const auto maskList = masks.split(";");
        if (addressList.size() < maskList.size())
        {
            RETURN_WITH_ERROR_TIPS(tr("The address does not match the mask number"), ui->edit_ipv4_ip);
        }
        else if (maskList.size() < addressList.size())
        {
            RETURN_WITH_ERROR_TIPS(tr("The mask does not match the number of addresses"), ui->edit_ipv4_mask);
        }

        for (auto address : addressList)
        {
            if (!ipv4AddressValid(address))
            {
                RETURN_WITH_ERROR_TIPS(tr("Invalid address, please re-enter"), ui->edit_ipv4_ip);
            }
        }

        for (auto mask : maskList)
        {
            if (!ipv4MaskValid(mask))
            {
                RETURN_WITH_ERROR_TIPS(tr("Invalid mask, please re-enter"), ui->edit_ipv4_mask);
            }
        }

        // 检查网关
        const auto gateway = ui->edit_ipv4_gateway->text();
        if (!gateway.isEmpty() && !ipv4AddressValid(gateway))
        {
            RETURN_WITH_ERROR_TIPS(tr("Invalid gateway, please re-enter"), ui->edit_ipv4_gateway);
        }
    }

    // 检查dns配置
    if (configMethod != Ipv4Setting::Disabled && !ui->edit_ipv4_dns->text().isEmpty())
    {
        const auto dnsList = ui->edit_ipv4_dns->text().split(";");
        for (auto dns : dnsList)
        {
            if (!ipv4AddressValid(dns))
            {
                RETURN_WITH_ERROR_TIPS(tr("Invalid dns, please re-enter"), ui->edit_ipv4_dns);
                return false;
            }
        }
    }

    return true;
}

void IpSettingsWidget::processConfigMethodChanged()
{
    auto configMethod = ui->combo_method->currentData().toInt();
    if (m_type == TYPE_IPV4)
    {
        auto ipv4Method = (Ipv4Setting::ConfigMethod)(configMethod);
        auto staticAddrAbleVisible = isStaticIPConfigUIVisible(ipv4Method);
        auto dnsVisible = isDNSConfigUIVisible(ipv4Method);
        ui->widget_ipv4_staticAddr->setVisible(staticAddrAbleVisible);
        ui->widget_ipv4_dns->setVisible(dnsVisible);
        ui->stackedWidget->setVisible(staticAddrAbleVisible || dnsVisible);
    }
    else if (m_type == TYPE_IPV6)
    {
        auto ipv6Method = (Ipv6Setting::ConfigMethod)(configMethod);
        auto staticAddrAbleVisible = isStaticIPConfigUIVisible(ipv6Method);
        auto dnsVisible = isDNSConfigUIVisible(ipv6Method);
        ui->widget_ipv6_staticAddr->setVisible(staticAddrAbleVisible);
        ui->widget_ipv6_dns->setVisible(dnsVisible);
        ui->stackedWidget->setVisible(staticAddrAbleVisible || dnsVisible);
    }

    // 由于QStackedLayout::sizeHint计算子页最大大小，手动计算高度
    auto contentsMargin = layout()->contentsMargins();
    auto sizeHintHeight = contentsMargin.top() + contentsMargin.bottom();
    sizeHintHeight += ui->widget_config_method->sizeHint().height();
    if (ui->stackedWidget->currentWidget())
    {
        sizeHintHeight += ui->stackedWidget->currentWidget()->sizeHint().height() + layout()->spacing();
    }
    setFixedHeight(sizeHintHeight);
}

}  // namespace Network
}  // namespace Kiran