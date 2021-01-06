/*
    
    bubba-networkmanager - http://www.excito.com/
    
    InterfacesCfg.cpp - this file is part of bubba-networkmanager.
    
    Copyright (C) 2009 Tor Krill <tor@excito.com>
    
    bubba-networkmanager is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2
    as published by the Free Software Foundation.
    
    bubba-networkmanager is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    You should have received a copy of the GNU General Public License
    version 2 along with libeutils; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
    
    $Id$
*/

#include "include/netifrc.h"

#include <string>
#include <iostream>
using namespace std;

#include "InterfacesCfg.h"

#include <libeutils/FileUtils.h>
#include <libeutils/StringTools.h>
using namespace EUtils;

namespace NetworkManager{

static const std::string handledInterfaces[4] = {"eth0", "eth1", "br0", "wlan0"};

InterfacesCfg::InterfacesCfg():cfg(Json::objectValue)
{
	this->parse_cfg();
}

InterfacesCfg& InterfacesCfg::Instance()
{
	static InterfacesCfg cfg;

	return cfg;
}

bool InterfacesCfg::is_handled_interface(std::string ifname)
{
	int i = 0;
	int s = static_cast<int>(sizeof(handledInterfaces)) - 1;
	while ((s > 0) && (handledInterfaces[i] != ifname))
	{
		s -= static_cast<int>(sizeof(handledInterfaces[i]));
		i++;
	}
	if (s < 0)
		return false;
	return true;
}

void InterfacesCfg::parse_cfg()
{
	list<std::string> fil = FileUtils::GetContent(IFSFILE);
	netifrc::config::type::value eItemType = netifrc::config::type::OTHER;
	netifrc::config::argumentstatus::value eArgumentStatus = netifrc::config::argumentstatus::FINISHED;

	std::string curif;
	for(list<string>::iterator fIt=fil.begin(); fIt!=fil.end(); fIt++)
	{
		std::string line = StringTools::Trimmed(*fIt, " \t");

		if ((line == "") or (line[0] == '#'))
			continue;

		std::string line7 = line.substr(0,7);
		if (line7 == "config_")
			eItemType = netifrc::config::type::CONFIG;
		else if (line7 == "routes_")
			eItemType = netifrc::config::type::ROUTES;
		else if (line7 == "bridge_")
			eItemType = netifrc::config::type::BRIDGE_PORTS;
		else if (eArgumentStatus == netifrc::config::argumentstatus::FINISHED)
			continue;

		std::string arguments;
		if (eItemType != netifrc::config::type::OTHER)
		{
			eArgumentStatus = netifrc::config::argumentstatus::WANT;
			size_t argsep = line.find_first_of("=");
			curif = line.substr(7, (argsep - 7));
			if (curif.size() > 7)	// sysfs entry
				continue;
			arguments = line.substr(argsep + 1);
			if (arguments == "")
				continue;
		}
		else
			arguments = line;

		if (!is_handled_interface(curif))
		{
			eArgumentStatus = netifrc::config::argumentstatus::FINISHED;
			// eItemType = NET_OTHER;
			continue;
		}

		if (arguments.substr(0,1) == "\"")
		{
			if (eArgumentStatus == netifrc::config::argumentstatus::WANT)
			{
				eArgumentStatus = netifrc::config::argumentstatus::READING;
				arguments = arguments.substr(1);
			}
			else
			{
				eArgumentStatus = netifrc::config::argumentstatus::FINISHED;
				continue;
			}
		}

		if (eArgumentStatus == netifrc::config::argumentstatus::READING)
		{
			if (arguments.substr(arguments.size()-1, 1) == "\"")
			{
				eArgumentStatus = netifrc::config::argumentstatus::FINISHED;
				arguments.resize(arguments.size() - 1);
			}
			list<std::string> words = StringTools::Split(arguments, "[ \t]");

			if (eItemType == netifrc::config::type::ROUTES)
			{
				this->cfg[curif]["options"]["routes"].append(arguments);
			}
			else if (eItemType == netifrc::config::type::BRIDGE_PORTS)
			{
				while (!words.empty())
				{
					this->cfg[curif]["options"]["bridge_ports"].append(words.front());
					words.pop_front();
				}
 			}
			else if (words.front() == "dhcp")
			{
				this->cfg[curif]["addressing"] = "dhcp";
			}
			else if (words.front() == "null")
			{
				this->cfg[curif]["addressing"] = "static";
				this->cfg[curif]["options"]["address"].append("0.0.0.0");
				this->cfg[curif]["options"]["netmask"].append("0.0.0.0");
			}
			else if (words.front() == "default")
			{
				this->cfg[curif]["options"]["gateway"].append(words.back());
			}
			else
			{
				this->cfg[curif]["addressing"]="static";
				this->cfg[curif]["options"]["address"].append(words.front());
				words.pop_front();
				while (!words.empty()){
					string key=words.front();
					words.pop_front();
 					this->cfg[curif]["options"][key].append(words.front());
					words.pop_front();
				}
			}
		}
	}
}

Json::Value InterfacesCfg::GetCFG(){
	return this->cfg;
}

bool InterfacesCfg::UpdateCFG(const Json::Value& val){
	this->cfg=val;
	return true;
}

bool InterfacesCfg::Commit()
{
	Json::Value devs = this->cfg;
	list<std::string> fil = FileUtils::GetContent(IFSFILE);
	netifrc::config::type::value eItemType = netifrc::config::type::OTHER;
	netifrc::config::argumentstatus::value eArgumentStatus = netifrc::config::argumentstatus::FINISHED;
	string curif = "";
	bool have_empty_line = false;
	list<std::string> res;

	if (devs.isMember("br0"))
	{
		devs.removeMember("eth1");
		devs.removeMember("wlan0");
	}

	bool have_preup = false;
	for (list<std::string>::iterator fIt = fil.begin(); (!have_preup && (fIt != fil.end())); fIt++)
	{
		std::string line = StringTools::Trimmed(*fIt, " \t");
		if (line.substr(0,7) == "preup()")
		{
			have_preup = true;
		}
	}

	res.push_back(netifrc::config_header);
	res.push_back(netifrc::modules_main);
	if (!have_preup)
		res.push_back(netifrc::preup_function);

	bool inHeader = true;
	for (list<std::string>::iterator fIt = fil.begin(); fIt != fil.end(); fIt++)
	{
		std::string line = StringTools::Trimmed(*fIt, " \t");
		if ((line[0] == '#') && inHeader)
			continue;
		else
			inHeader = false;

		if (line.empty())
		{
			if (!have_empty_line)
			{
				res.push_back("\n");
				have_empty_line = true;
			}
			continue;
		}

		if (line[0] == '#')
		{
			if ((line.size() > 7) && (line.substr(2,5) == "setup"))
				continue;
			if ((line.size() > 12) && (line.substr(2,10) == "null setup"))
				continue;
			if ((line.size() > 32) && (line.substr(25,7) == "hostapd"))
				continue;
			res.push_back(*fIt + "\n");
			continue;
		}

		if ((line.substr(0,7) == "modules") && (line.substr(7,1) != "_"))
			continue;

		std::string line7 = line.substr(0,7);
		if (line7 == "config_")
			eItemType = netifrc::config::type::CONFIG;
		else if (line7 == "routes_")
			eItemType = netifrc::config::type::ROUTES;
		else if (line7 == "bridge_")
			eItemType = netifrc::config::type::BRIDGE_PORTS;
		else if (line7 == "rc_net_")
			eItemType = netifrc::config::type::CONTROL;
		else if (line7 == "dhcpcd_")
			eItemType = netifrc::config::type::DHCP_PARAMETERS;
		else if (line.substr(0,9) == "fallback_")
			eItemType = netifrc::config::type::FALLBACK;
		else if (line.substr(0,8) == "modules_")
			eItemType = netifrc::config::type::MODULES;
		else if (line.substr(0,6) == "brctl_")
			eItemType = netifrc::config::type::BRIDGE_DEPRECATED_CTL;
		else if (eArgumentStatus == netifrc::config::argumentstatus::FINISHED)
		{
			res.push_back(*fIt + "\n");
			have_empty_line = false;
			continue;
		}

		std::string arguments;
		if (eItemType != netifrc::config::type::OTHER)
		{
			eArgumentStatus = netifrc::config::argumentstatus::WANT;
			int argsep = static_cast<int>(line.find_first_of("="));
			int offset = 7;
			if (eItemType == netifrc::config::type::BRIDGE_DEPRECATED_CTL)
				offset--;
			else if (eItemType == netifrc::config::type::MODULES)
				offset++;
			else if (eItemType == netifrc::config::type::FALLBACK)
				offset += 2;
			curif = line.substr(offset, (argsep - offset));
			arguments = line.substr(argsep + 1);
		}
		else
			arguments = line;


		if (eItemType == netifrc::config::type::CONTROL)
		{
			std::string rcnet = curif;
			size_t argsep = rcnet.find_first_of("_");
			curif = rcnet.substr(0, argsep);
		}

		if ((eItemType == netifrc::config::type::BRIDGE_PORTS) && (curif.size() > 7))	// sysfs entry
		{
			std::string sysfs = curif;
			size_t argsep = sysfs.find_last_of("_");
			curif = sysfs.substr(argsep + 1);
		}

		if (arguments.substr(0,1) == "\"")
		{
			if (eArgumentStatus == netifrc::config::argumentstatus::WANT)
				eArgumentStatus = netifrc::config::argumentstatus::READING;
			else
				eArgumentStatus = netifrc::config::argumentstatus::FINISHED;
		}

		if ((eArgumentStatus == netifrc::config::argumentstatus::READING) && (arguments.substr(arguments.size() - 1, 1) == "\""))
			eArgumentStatus = netifrc::config::argumentstatus::FINISHED;

		if (!is_handled_interface(curif))
		{
			if ((eItemType == netifrc::config::type::CONFIG) && (eArgumentStatus == netifrc::config::argumentstatus::FINISHED))
				cout << "Keep device " << curif << " : not handled by this application\n";
			curif = "";
			res.push_back(*fIt + "\n");
			have_empty_line = false;
			eArgumentStatus = netifrc::config::argumentstatus::FINISHED;
			continue;
		}

		if (!devs.isMember(curif))
		{
			if ((eItemType == netifrc::config::type::CONFIG) && (eArgumentStatus == netifrc::config::argumentstatus::FINISHED))
				cout << "Remove device " << curif << " : not part of new configuration\n";
			continue;
		}

		if ((eItemType == netifrc::config::type::CONFIG) && (eArgumentStatus == netifrc::config::argumentstatus::FINISHED))
		{
			if (!have_empty_line)
				res.push_back("\n");
			have_empty_line = false;

			cout << "Change config for device " << curif << "\n";
			Json::Value NIC = devs[curif]["options"];
			int i=0;
			if (NIC.isMember("bridge_ports"))
			{

				std::vector<std::string> bridge_ports;
				for (int j = 0; j < static_cast<int>(NIC["bridge_ports"].size()); j++)
				{
					bridge_ports.push_back(NIC["bridge_ports"][j].asString());
				}
                                res.push_back(netifrc::mk_bridge_entry(curif, bridge_ports));
			}

			if (devs[curif]["addressing"] == "dhcp")
                                res.push_back(netifrc::mk_config_line(curif, "dhcp"));
			else if (NIC["address"][i].asString()=="0.0.0.0")
			{
                                res.push_back(netifrc::mk_config_line(curif, NIC["address"][i].asString()));
				NIC.removeMember("gateway");
				NIC.removeMember("routes");
			}
			else if (NIC["address"][i].asString()!="")
                                res.push_back(netifrc::mk_config_line(curif, NIC["address"][i].asString(), NIC["netmask"][i].asString()));

			std::vector<std::string> routes;
			if (NIC.isMember("routes"))
			{
				for (int j = 0; j < static_cast<int>(NIC["routes"].size()); j++)
					routes.push_back(NIC["routes"][j].asString());
			}
			if (NIC.isMember("gateway") && (NIC["gateway"][i].asString() != "0.0.0.0"))
				routes.push_back("default via " + NIC["gateway"][i].asString());
			if (routes.size() > 0)
                                res.push_back(netifrc::mk_routes_line(curif, routes));

			devs.removeMember(curif);
		}
		curif="";
	}

	Json::Value::Members opts=devs.getMemberNames();
	for(Json::Value::Members::iterator oIt=opts.begin();oIt!=opts.end();oIt++){
		if (!have_empty_line)
			res.push_back("\n");
		have_empty_line = false;

		cout << "Add config for device " << *oIt << "\n";
		Json::Value NIC = devs[*oIt]["options"];
		int i = 0;
		if (NIC.isMember("bridge_ports"))
		{
			std::vector<std::string> bridge_ports;
			for (int j = 0; j < static_cast<int>(NIC["bridge_ports"].size()); j++)
			{
				bridge_ports.push_back(NIC["bridge_ports"][j].asString());
			}
                               res.push_back(netifrc::mk_bridge_entry(*oIt, bridge_ports));
		}
		if (devs[*oIt]["addressing"] == "dhcp")
			res.push_back(netifrc::mk_config_line(curif, "dhcp"));
		else if (NIC["address"][i].asString() == "0.0.0.0")
		{
			res.push_back(netifrc::mk_config_line(*oIt, NIC["address"][i].asString()));
			NIC.removeMember("gateway");
			NIC.removeMember("routes");
		}
		else if (NIC["address"][i].asString() != "")
			res.push_back(netifrc::mk_config_line(*oIt, NIC["address"][i].asString(), NIC["netmask"][i].asString()));

		if (NIC.isMember("gateway") && (NIC["gateway"][i].asString() != "0.0.0.0"))
		{
			std::vector<std::string> routes;
			routes.push_back("default via " + NIC["gateway"][i].asString());
			res.push_back(netifrc::mk_routes_line(*oIt, routes));
		}
	}

	cout << "Write config\n";
	FileUtils::Write(IFSFILE, res, 0644);
	return true;
}

InterfacesCfg::~InterfacesCfg(){
}


}
