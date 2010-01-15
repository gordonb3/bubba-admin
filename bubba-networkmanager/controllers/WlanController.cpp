/*

    bubba-networkmanager - http://www.excito.com/

    WlanController.cpp - this file is part of bubba-networkmanager.

    Copyright (C) 2009 Tor Krill <tor@excito.com>

    bubba-networkmanager is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2
    as published by the Free Software Foundation.

    bubba-networkmanager is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    version 2 along with bubba-networkmanager; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.

    $Id$
*/

#include <stdexcept>

#include "WlanController.h"
#include "InterfaceController.h"
#include "PolicyController.h"
// Used to bootstrap config
#include "../utils/WlanCfg.h"

WlanController::WlanController() {

}

WlanController & WlanController::Instance(){
	static WlanController wc;
	return wc;
}

bool WlanController::HasWlan(){
	bool haswlan=false;
	PolicyController& pc=PolicyController::Instance();

	list<string> ifs=InterfaceController::Instance().GetInterfaces();
	for(list<string>::iterator iIt=ifs.begin();iIt!=ifs.end();iIt++){
		if(pc.Allowed(*iIt,"iswlan")){
			haswlan=true;
			break;
		}
	}
	return haswlan;
}

// Used to bootstrap AP cfg to use right interface
void WlanController::SetApInterface(const string& ifname){
	WlanCfg& wcfg=WlanCfg::Instance();
	Json::Value cfg=wcfg.GetCFG();
	cfg["interface"]=ifname;

	wcfg.UpdateCFG(cfg);
	wcfg.Commit();
}

void WlanController::SetApCfg(const string& ifname, const Json::Value& cfg){
	auto_ptr<NetworkManager::Interface> ifc=InterfaceController::Instance().GetInterface(ifname);

	map<Profile,Configuration> cfgs=ifc->GetConfigurations();
	if(cfgs.find(WlanAP)!=cfgs.end()){
		cfgs[WlanAP].cfg=cfg;
		ifc->SetConfigurations(cfgs);
	}else{
		throw std::runtime_error("Not a wlanAP interface");
	}
}
void WlanController::SetApHTCapab(const string& ifname, const Json::Value& capab){
	auto_ptr<NetworkManager::Interface> ifc=InterfaceController::Instance().GetInterface(ifname);

	map<Profile,Configuration> cfgs=ifc->GetConfigurations();
	if(cfgs.find(WlanAP)!=cfgs.end()){
		cfgs[WlanAP].cfg["config"]["ht_capab"]=capab;
		ifc->SetConfigurations(cfgs);
	}else{
		throw std::runtime_error("Not a wlanAP interface");
	}
}

void WlanController::SetApSSID(const string& ifname, const string& ssid){

	auto_ptr<NetworkManager::Interface> ifc=InterfaceController::Instance().GetInterface(ifname);
	map<Profile,Configuration> cfgs=ifc->GetConfigurations();

	if(cfgs.find(WlanAP)!=cfgs.end()){
		cfgs[WlanAP].cfg["config"]["ssid"]=ssid;
		ifc->SetConfigurations(cfgs);
	}else{
		throw std::runtime_error("Not a wlanAP interface");
	}

}
void WlanController::SetApMode(const string& ifname, const string& mode){

	auto_ptr<NetworkManager::Interface> ifc=InterfaceController::Instance().GetInterface(ifname);
	map<Profile,Configuration> cfgs=ifc->GetConfigurations();

	if(cfgs.find(WlanAP)!=cfgs.end()){
		cfgs[WlanAP].cfg["config"]["mode"]=mode;
		ifc->SetConfigurations(cfgs);
	}else{
		throw std::runtime_error("Not a wlanAP interface");
	}

}

void WlanController::Enable80211n(const string& ifname, const bool enable){

	auto_ptr<NetworkManager::Interface> ifc=InterfaceController::Instance().GetInterface(ifname);
	map<Profile,Configuration> cfgs=ifc->GetConfigurations();

	if(cfgs.find(WlanAP)!=cfgs.end()){
		cfgs[WlanAP].cfg["config"]["80211n"]=enable;
		ifc->SetConfigurations(cfgs);
	}else{
		throw std::runtime_error("Not a wlanAP interface");
	}

}
void WlanController::SetAPChannel(const string& ifname, int channel){

	auto_ptr<NetworkManager::Interface> ifc=InterfaceController::Instance().GetInterface(ifname);
	map<Profile,Configuration> cfgs=ifc->GetConfigurations();

	if(cfgs.find(WlanAP)!=cfgs.end()){
		cfgs[WlanAP].cfg["config"]["channel"]=channel;
		ifc->SetConfigurations(cfgs);
	}else{
		throw std::runtime_error("Not a wlanAP interface");
	}

}
void WlanController::SetAPAuthNone(const string& ifname){

	auto_ptr<NetworkManager::Interface> ifc=InterfaceController::Instance().GetInterface(ifname);
	map<Profile,Configuration> cfgs=ifc->GetConfigurations();

	if(cfgs.find(WlanAP)!=cfgs.end()){
		cfgs[WlanAP].cfg["config"].removeMember("auth");
		cfgs[WlanAP].cfg["config"]["auth"]["mode"]="none";
		ifc->SetConfigurations(cfgs);
	}else{
		throw std::runtime_error("Not a wlanAP interface");
	}

}

void WlanController::SetAPAuthWep(const string& ifname, const Json::Value& cfg){

	auto_ptr<NetworkManager::Interface> ifc=InterfaceController::Instance().GetInterface(ifname);
	map<Profile,Configuration> cfgs=ifc->GetConfigurations();

	if(cfgs.find(WlanAP)!=cfgs.end()){
		cfgs[WlanAP].cfg["config"].removeMember("auth");
		cfgs[WlanAP].cfg["config"]["auth"]["mode"]="wep";
		cfgs[WlanAP].cfg["config"]["auth"]["wep"]["defaultkey"]=cfg["defaultkey"];
		cfgs[WlanAP].cfg["config"]["auth"]["wep"]["keys"]=cfg["keys"];
		ifc->SetConfigurations(cfgs);
	}else{
		throw std::runtime_error("Not a wlanAP interface");
	}

}

void WlanController::SetAPAuthWpa(const string& ifname, const Json::Value& cfg){

	auto_ptr<NetworkManager::Interface> ifc=InterfaceController::Instance().GetInterface(ifname);
	map<Profile,Configuration> cfgs=ifc->GetConfigurations();

	if(cfgs.find(WlanAP)!=cfgs.end()){
		cfgs[WlanAP].cfg["config"].removeMember("auth");
		cfgs[WlanAP].cfg["config"]["auth"]["mode"]="wpa";
		cfgs[WlanAP].cfg["config"]["auth"]["wpa"]["mode"]=cfg["mode"];
		cfgs[WlanAP].cfg["config"]["auth"]["wpa"]["keys"]=cfg["keys"];
		ifc->SetConfigurations(cfgs);
	}else{
		throw std::runtime_error("Not a wlanAP interface");
	}

}

WlanController::~WlanController() {

}
