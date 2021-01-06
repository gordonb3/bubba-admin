#pragma once

#include <string>
#include <vector>
#include <arpa/inet.h>

namespace netifrc {

	namespace config {
		namespace type {
			enum value
			{
				OTHER,
				CONFIG,
				ROUTES,
				BRIDGE_PORTS,
				CONTROL,
				MODULES,
				DHCP_PARAMETERS,
				FALLBACK,
				BRIDGE_DEPRECATED_CTL
			};

		}; // namespace type

		namespace argumentstatus {
			enum value
			{
				FINISHED,
				WANT,
				READING
			};

		}; // namespace argumentstatus
	}; // namespace config


	static const std::string config_header 	= "# This file is managed by bubba-networkmanager\n";
	static const std::string modules_main	= "modules=\"iproute2\"\n";
	static const std::string rfkill_unblock	= "preup() {\n  if [ \"${IFACE}\" = \"wlan0\" ] ; then\n     rfkill unblock all\n  fi\n  return 0\n}";

	static const std::string if_header	= "# setup for {if} (lan Ethernet port)\n";
	static const std::string if_auto	= "config_{if}=\"dhcp\"\ndhcpcd_{if}=\"-t 15\"\n";
	static const std::string if_manual	= "config_{if}=\"{addr} netmask {mask} brd {brd}\"\n";
	static const std::string if_nonet	= "config_{if}=\"null\"\nrc_net_{if}_provide=\"!net\"\n";

	static const std::string eth1_fallback	= "fallback_eth1=\"192.168.10.1 netmask 255.255.255.0 brd 192.168.10.255\"\n";
	static const std::string modules_wlan	= "modules_{if}=\"!iw !iwconfig !wpa_supplicant\"\n";
	static const std::string bridge_entry	= "bridge_{if}=\"{members}\"\nbridge_forward_delay_{if}=0\nbridge_hello_time_{if}=1000\nbridge_stp_state_{if}=1\n";
	static const std::string wlan_extra_header	= "# (this will be owned by hostapd, if present on your system)\n";

	static const std::string preup_function	= "\npreup() {\n  # prevent hostapd failure due to soft-blocked radio\n  if [ \"${IFACE}\" = \"wlan0\" ] ; then\n     rfkill unblock all\n  fi\n\n  # test WAN link\n  if [ \"${IFACE}\" = \"eth0\" ]; then\n    if (mii-tool \"${IFACE}\" | grep -q 'no link'); then\n      ewarn \"No link on ${IFACE}, aborting configuration\"\n      return 1\n    fi\n  fi\n\n  return 0\n}\n";

	static std::string mk_bridge_entry(const std::string bridge, const std::vector<std::string> bridge_ports)
	{
		std::string result;
		std::string portlist;

		std::string header = netifrc::if_header;
		header.replace(22, 13, "bridge");
		header.replace(12, 4, bridge);
		result.append(header);

		for (int j = 0; j < static_cast<int>(bridge_ports.size()); j++)
		{
			if (!bridge_ports[j].empty())
			{
				if (j > 0)
					portlist.append(" ");
				portlist.append(bridge_ports[j]);

				std::string member = netifrc::if_nonet;
				member.replace(26, 4, bridge_ports[j]);
				member.replace(7, 4, bridge_ports[j]);
				if (bridge_ports[j].substr(0,2) == "wl")
				{
					member.insert(0, modules_wlan);
					member.replace(8, 4, bridge_ports[j]);
					member.append("rc_net_");
					member.append(bridge);
					member.append("_need=\"hostapd\"\n");
				}
				result.append(member);
			}
		}

		std::string newbridge = netifrc::bridge_entry;
		newbridge.replace(97, 4, bridge);
		newbridge.replace(70, 4, bridge);
		newbridge.replace(45, 4, bridge);
		newbridge.replace(13, 9, portlist);
		newbridge.replace(7, 4, bridge);

		result.append(newbridge);
		return result;
	}

	static std::string mk_config_line(const std::string ifname, const std::string address, const std::string netmask = "")
	{
		if (address.empty())
			return "";

		std::string result;
		if (ifname != "br0")
		{
			int offset = 12;
			result = netifrc::if_header;
			if (ifname == "eth0")
				result.replace(18, 1, "w");
			else if (ifname == "wlan0")
			{
				result.replace(18, 17, "WiFi adaptor");
				result.insert(2, "null ");
				offset += 5;
				result.append(netifrc::wlan_extra_header);
				std::string modules = netifrc::modules_wlan;
				modules.replace(8, 4, ifname);
				result.append(modules);
			}
			result.replace(offset, 4, ifname);
		}

		if (address == "dhcp")
		{
			std::string autoconf = netifrc::if_auto;
			autoconf.replace(26, 4, ifname);
			autoconf.replace(7, 4, ifname);
			result.append(autoconf);
			if (ifname == "eth1")
				result.append(netifrc::eth1_fallback);
			return result;
		}
		if (address == "0.0.0.0")
		{
			std::string nullconf = netifrc::if_nonet;
			nullconf.replace(26, 4, ifname);
			nullconf.replace(7, 4, ifname);
			result.append(nullconf);
			return result;
		}

		struct in_addr s_ipaddress;
		struct in_addr s_netmask;
		inet_aton(address.c_str(), &s_ipaddress);
		inet_aton(netmask.c_str(), &s_netmask);
		s_ipaddress.s_addr |= ~(s_netmask.s_addr);
		std::string broadcast = inet_ntoa(s_ipaddress);

		std::string manualconf = netifrc::if_manual;
		manualconf.replace(39, 5, broadcast);
		manualconf.replace(28, 6, netmask);
		manualconf.replace(13, 6, address);
		manualconf.replace(7, 4, ifname);
		result.append(manualconf);
		return result;
	}

	static std::string mk_routes_line(const std::string ifname, const std::vector<std::string> routes)
	{
		int i = 0;
		std::string result;
		for (int j = 0; j < static_cast<int>(routes.size()); j++)
		{
			if (!routes[j].empty())
			{
				if (i == 0)
					result.append("routes_" + ifname + "=\"");
				else
					result.append("\n             ");

				result.append(routes[j]);
				i++;
			}
		}
		if (i == 0)
			return "";
		if (i > 1)
			result.append("\n\"");
		else
			result.append("\"");
		return result;
	}

}; // namespace netifrc
