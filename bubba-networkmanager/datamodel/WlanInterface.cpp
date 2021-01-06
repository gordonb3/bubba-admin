/*

    bubba-networkmanager - http://www.excito.com/

    WlanInterface.cpp - this file is part of bubba-networkmanager.

    Copyright (C) 2009 Tor Krill <tor@excito.com>

    bubba-networkmanager is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2
    as published by the Free Software Foundation.

    bubba-networkmanager is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    version 2 along with bubba-networkmanager if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.

    $Id$
*/
#include "WlanInterface.h"
#include <libeutils/Services.h>
#include <libeutils/FileUtils.h>

#include "../utils/WlanCfg.h"
#include "../utils/InterfacesCfg.h"
#include "../utils/Sockios.h"

using namespace EUtils;

namespace NetworkManager {

WlanInterface::WlanInterface(const string& device):EthernetInterface(device) {
	this->read_wlan_cfg();
}

void WlanInterface::read_cfg(){
	EthernetInterface::read_cfg();
	this->read_wlan_cfg();
}

void WlanInterface::read_wlan_cfg(){
	// Todo replace with define or something a bit smarter
	if(Stat::FileExists("/etc/hostapd/hostapd.conf")){
		Json::Value wcfg=WlanCfg::Instance().GetCFG();
		if(wcfg["interface"]==this->name){
			Configuration c(WlanAP);
			c.cfg["addressing"]="ap";
			c.cfg["config"]=wcfg;
			c.cfg["type"]="wlan";
			this->configs[c.profile]=c;
		}
	}else{
		// Assume raw for now
		Configuration c(WlanRaw);
		c.cfg["addressing"]="raw";
		c.cfg["type"]="wlan";
		this->configs[c.profile]=c;
	}
}


void WlanInterface::SetConfiguration(const Configuration& cfg){
	switch(cfg.profile){
	case EthDynamic:
	case EthStatic:
	case EthRaw:
		EthernetInterface::SetConfiguration(cfg);
		break;
	case WlanRaw:
		{
			if(Services::IsEnabled("hostapd")){
				Services::Disable("hostapd");
			}
		}
		break;
	case WlanAP:
		{
			if(!Services::IsEnabled("hostapd")){
				Services::Enable("hostapd");
			}
			WlanCfg& wcfg=WlanCfg::Instance();
			wcfg.UpdateCFG(cfg.cfg["config"]);
			wcfg.Commit();
		}
		break;
	default:
		// Unsupported profile
		break;
	}
}


WlanInterface::~WlanInterface(){

}

}
