/*

    bubba-networkmanager - http://www.excito.com/

    BridgeInterface.cpp - this file is part of bubba-networkmanager.

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

#include <algorithm>
#include <stdexcept>

#include "BridgeInterface.h"

#include "../utils/Sockios.h"
#include "../utils/InterfacesCfg.h"
#include "../utils/JsonUtils.h"

namespace NetworkManager {

BridgeInterface::BridgeInterface(const string& device):Interface(device) {
	this->read_cfg();
}

void BridgeInterface::read_cfg(){
	this->configs.clear();

	InterfacesCfg& icfg=InterfacesCfg::Instance();
	Json::Value ieth=icfg.GetCFG()[name];

	// Check current config
	Configuration c;
	c.cfg["current"]=Sockios::GetConfig(name);
	c.cfg["name"]=name;
	c.cfg["type"]="bridge";


	if(ieth.isMember("addressing")){
		c.cfg["addressing"]=ieth["addressing"];
		if(ieth.isMember("auto")){
			c.cfg["auto"]=ieth["auto"];
		}
		if(ieth["addressing"]=="static"){
			c.profile=NetworkManager::BridgeStatic;
			c.cfg["config"]=ieth["options"];
		}else if(ieth["addressing"]=="dhcp"){
			c.profile=NetworkManager::BridgeDynamic;
			c.cfg["config"]=ieth["options"];
		}
	}else{
		c.profile=NetworkManager::BridgeRaw;
		c.cfg["addressing"]="raw";
	}

	this->configs[c.profile]=c;
}

map<Profile,Configuration> BridgeInterface::GetConfigurations(){
	return this->configs;
}


void BridgeInterface::SetConfiguration(const Configuration & cfg){
	switch(cfg.profile){
	case BridgeDynamic:
	case BridgeStatic:
		{
			// Update interfaces
			InterfacesCfg& icfg=InterfacesCfg::Instance();
			Json::Value ic=icfg.GetCFG();

			ic.removeMember(this->name);

			ic[cfg.cfg["name"].asString()]["options"]=cfg.cfg["config"];

			if(cfg.cfg["config"].isMember("bridge_ports")){
				// Sort out any dups
				vector<string> ports=JsonUtils::ArrayToVector(cfg.cfg["config"]["bridge_ports"]);

				std::sort(ports.begin(),ports.end());

				vector<string>::iterator nEnd=std::unique(ports.begin(),ports.end());
				ports.resize(nEnd-ports.begin());

				ic[cfg.cfg["name"].asString()]["options"]["bridge_ports"]=JsonUtils::toArray(ports);;
			}
			ic[cfg.cfg["name"].asString()]["addressing"]=cfg.cfg["addressing"];

			if(cfg.cfg.isMember("auto")){
				ic[this->name]["auto"]=true;
			}

			icfg.UpdateCFG(ic);
			icfg.Commit();

			// Update interface
			Sockios::SetConfig(this->name,cfg.cfg["current"]);

		}
		break;
	case BridgeRaw:
		//Just remove declaration from interfaces file
		{
			InterfacesCfg& icfg=InterfacesCfg::Instance();
			Json::Value ic=icfg.GetCFG();
			ic.removeMember(this->name);
			icfg.UpdateCFG(ic);
			icfg.Commit();
		}
		break;

	default:
		throw runtime_error("Unsupported profile for Bridge interface");
		break;
	}

}
void BridgeInterface::SetConfigurations(const map<Profile,Configuration>& cfgs){
	for(map<Profile,Configuration>::const_iterator cIt=cfgs.begin();cIt!=cfgs.end();cIt++){
		this->SetConfiguration((*cIt).second);
	}
	this->read_cfg();
}

BridgeInterface::~BridgeInterface() {
}

}
