/*
    
    bubba-networkmanager - http://www.excito.com/
    
    InterfaceController.cpp - this file is part of bubba-networkmanager.
    
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

#include <stdexcept>
#include <algorithm>

#include <signal.h>

#include "InterfaceController.h"
#include "PolicyController.h"

#include "../utils/SysConfig.h"
#include "../utils/Sockios.h"
#include "../utils/Hosts.h"
#include "../utils/JsonUtils.h"

#include "../datamodel/EthernetInterface.h"
#include "../datamodel/BridgeInterface.h"
#include "../datamodel/WlanInterface.h"

#include <libeutils/FileUtils.h>
#include <libeutils/Services.h>

using namespace EUtils;

static int do_call(const string& cmd){
	int ret=system(cmd.c_str());
	if(ret<0){
		return ret;
	}
	return WEXITSTATUS(ret);
}


InterfaceController::InterfaceController(){
}



InterfaceController & InterfaceController::Instance(){
	static InterfaceController ifc;
	return ifc;
}



auto_ptr<NetworkManager::Interface> InterfaceController::GetInterface(const string& ifname){
	PolicyController& pc=PolicyController::Instance();

	list<string> ifs=this->GetInterfaces();
	if(find(ifs.begin(),ifs.end(),ifname)==ifs.end()){
		throw std::runtime_error("Device not found");
	}

	string type=pc.GetInterfaceType(ifname);
	if(type=="ether"){
		return auto_ptr<Interface>(new EthernetInterface(ifname));
	}else if(type=="wlan"){
		return auto_ptr<Interface>(new WlanInterface(ifname));
	}else if(type=="bridge"){
		return auto_ptr<Interface>(new BridgeInterface(ifname));
	}else{
		throw std::runtime_error("Unsupported network class");
	}
	// Should not get here
	return auto_ptr<Interface>(new Interface());

}

list<string> InterfaceController::GetInterfaces(const string& itype){
	list<string> ifs=Sockios::GetInterfaceNames();
	list<string> ret;
	bool filter;
	filter=itype.length()!=0;
	bool add_virtual_bridge=true;
	if(filter && (itype!="bridge")){
		add_virtual_bridge=false;
	}
	for(list<string>::iterator iIt=ifs.begin();iIt!=ifs.end();iIt++){

		try{
			if(filter && !(PolicyController::Instance().GetInterfaceType(*iIt)==itype)){
				continue;
			}
		}catch(runtime_error& err){
			continue;
		}

		if((*iIt).substr(0,2)=="br"){
			add_virtual_bridge=false;
			ret.push_back(*iIt);
			continue;
		}
		if((*iIt).substr(0,3)=="eth"){
			ret.push_back(*iIt);
			continue;
		}
		if((*iIt).substr(0,4)=="wlan"){
			ret.push_back(*iIt);
			continue;
		}
	}

	if(add_virtual_bridge){
		// Add any virtual device
		ret.push_back("br0");
	}

	return ret;
}

static string getwlan(const Json::Value& v, list<string>& wifs){
	string ret;
	if(v.isMember("config")&& v["config"].isMember("bridge_ports")){
		list<string> bifs=JsonUtils::ArrayToList(v["config"]["bridge_ports"]);
		list<string>::iterator find=find_first_of(wifs.begin(),wifs.end(),bifs.begin(),bifs.end());
		if(find!=wifs.end()){
			ret=*find;
		}
	}
	if(ret=="" && wifs.size()>0){
		ret=wifs.front();
	}
	return ret;
}

string InterfaceController::GetDefaultLanInterface(){
	return SysConfig::Instance().ValueOrDefault("defaultlan","eth1");
}

string InterfaceController::GetDefaultWanInterface(){
	return SysConfig::Instance().ValueOrDefault("defaultwan","eth0");
}

string InterfaceController::GetCurrentLanInterface(){
	return SysConfig::Instance().ValueOrDefault("lanif",
				this->GetDefaultLanInterface());
}

string InterfaceController::GetCurrentWanInterface(){
	return SysConfig::Instance().ValueOrDefault("wanif",
				this->GetDefaultWanInterface());
}

string InterfaceController::GetCurrentWlanInterface(){

	// Try an educated guess on which wlan if is the "right" one.
	// Get wlan if from current bridge interface if existant
	// Otherwise return first wlanif in system
	string wlanif="";

	list<string> ifs=this->GetInterfaces("wlan");

	string lanname=SysConfig::Instance().ValueOrDefault("lanif","eth1");
	auto_ptr<NetworkManager::Interface> lanif=this->GetInterface(lanname);
	map<Profile,Configuration> cfgs=lanif->GetConfigurations();

	if(cfgs.find(BridgeStatic)!=cfgs.end()){
		wlanif=getwlan(cfgs[BridgeStatic].cfg,ifs);
	}else if(cfgs.find(BridgeDynamic)!=cfgs.end()){
		wlanif=getwlan(cfgs[BridgeDynamic].cfg,ifs);
	}else{
		if(ifs.size()>0){
			wlanif=ifs.front();
		}
	}


	return wlanif;
}

bool InterfaceController::SetMtu(const string & ifname, int mtu){
	bool found=false;

	auto_ptr<NetworkManager::Interface> ifc=this->GetInterface(ifname);

	map<Profile,Configuration> cfgs=ifc->GetConfigurations();

	for(map<Profile,Configuration>::iterator cIt=cfgs.begin();cIt!=cfgs.end();cIt++){
		if((*cIt).second.cfg.isMember("current")){
			(*cIt).second.cfg["current"]["mtu"]=mtu;
			found=true;
		}
	}

	if(found){
		ifc->SetConfigurations(cfgs);
	}

	return found;
}

int InterfaceController::GetMtu(const string & ifname){
	int mtu=-1;
	auto_ptr<NetworkManager::Interface> ifc=this->GetInterface(ifname);

	map<Profile,Configuration> cfgs=ifc->GetConfigurations();

	for(map<Profile,Configuration>::iterator cIt=cfgs.begin();cIt!=cfgs.end();cIt++){
		if((*cIt).second.cfg.isMember("current")){
			mtu=(*cIt).second.cfg["current"]["mtu"].asInt();
		}
	}

	if(mtu==-1){
		throw std::runtime_error("mtu not found on device");
	}

	return mtu;
}

bool InterfaceController::GetPromisc(const string& ifname){
	auto_ptr<NetworkManager::Interface> ifc=this->GetInterface(ifname);

	map<Profile,Configuration> cfgs=ifc->GetConfigurations();
	bool promisc=false;
	for(map<Profile,Configuration>::iterator cIt=cfgs.begin();cIt!=cfgs.end();cIt++){
		if((*cIt).second.cfg.isMember("current")){
			promisc=(*cIt).second.cfg["current"]["flags"]["promisc"].asBool();
		}
	}
	return promisc;
}


bool InterfaceController::SetPromisc(const string& ifname, bool promisc){
	bool found=false;

	auto_ptr<NetworkManager::Interface> ifc=this->GetInterface(ifname);

	map<Profile,Configuration> cfgs=ifc->GetConfigurations();

	for(map<Profile,Configuration>::iterator cIt=cfgs.begin();cIt!=cfgs.end();cIt++){
		if((*cIt).second.cfg.isMember("current")){
			(*cIt).second.cfg["current"]["flags"]["promisc"]=promisc;
			found=true;
		}
	}

	if(found){
		ifc->SetConfigurations(cfgs);
	}

	return found;

}



static void validateether( const Json::Value& cfg){
	// All configs should have netmask and address
	if(!cfg.isMember("address")){
		throw std::runtime_error("address parameter missing");
	}

	if(!cfg["address"].isArray()){
		throw std::runtime_error("address parameter not array");
	}

	if(!cfg.isMember("netmask")){
		throw std::runtime_error("netmask parameter missing");
	}

	if(!cfg["netmask"].isArray()){
		throw std::runtime_error("netmask parameter not array");
	}
}

static void validatebridge_andsetdefault( Json::Value& cfg){

	if(!cfg.isMember("bridge_maxwait")){
		cfg["bridge_maxwait"].append(SysConfig::Instance().ValueOrDefault("bridge_maxwait","0"));
	}

	if(!cfg["bridge_maxwait"].isArray()){
		throw std::runtime_error("bridge_maxwait parameter not array");
	}

	if(!cfg.isMember("bridge_fd")){
		cfg["bridge_fd"].append(SysConfig::Instance().ValueOrDefault("bridge_fd","0"));
	}

	if(!cfg["bridge_fd"].isArray()){
		throw std::runtime_error("bridge_fd parameter not array");
	}

	if(!cfg.isMember("bridge_ports")){
		cfg["bridge_ports"]=Json::Value(Json::arrayValue);
		cfg["bridge_ports"].append(InterfaceController::Instance().GetDefaultLanInterface());
		cfg["bridge_ports"].append(InterfaceController::Instance().GetCurrentWlanInterface());
	}

	if(!cfg["bridge_ports"].isArray()){
		throw std::runtime_error("bridge_ports parameter not array");
	}

}

void InterfaceController::SetStaticCfg(const string& ifname, const Json::Value& acfg){
	Json::Value cfg=acfg;
	auto_ptr<NetworkManager::Interface> ifc=this->GetInterface(ifname);
	map<Profile,Configuration> cfgs=ifc->GetConfigurations();

	validateether(cfg["config"]);

	if(cfgs.find(EthRaw)!=cfgs.end()){

		cfgs.erase(EthRaw);
		Configuration c(EthStatic);
		c.cfg["addressing"]="static";
		c.cfg["config"]=cfg["config"];
		c.cfg["name"]=ifname;
		c.cfg["type"]="ethernet";
		if(cfg.isMember("auto")){
			c.cfg["auto"]=true;
		}
		cfgs[EthStatic]=c;

	}else if(cfgs.find(EthDynamic)!=cfgs.end()){

		cfgs.erase(EthDynamic);
		Configuration c(EthStatic);
		c.cfg["addressing"]="static";
		c.cfg["config"]=cfg["config"];
		c.cfg["name"]=ifname;
		c.cfg["type"]="ethernet";
		if(cfg.isMember("auto")){
			c.cfg["auto"]=true;
		}
		cfgs[EthStatic]=c;

	}else if(cfgs.find(EthStatic)!=cfgs.end()){

		cfgs[EthStatic].cfg["config"]=cfg["config"];
		if(cfg.isMember("auto")){
			cfgs[EthStatic].cfg["auto"]=true;
		}

	}else if(cfgs.find(BridgeDynamic)!=cfgs.end()){

		validatebridge_andsetdefault(cfg["config"]);

		cfgs.erase(BridgeDynamic);
		Configuration c(BridgeStatic);
		c.cfg["addressing"]="static";
		c.cfg["config"]=cfg["config"];
		c.cfg["type"]="bridge";
		c.cfg["name"]=ifname;
		if(cfg.isMember("auto")){
			c.cfg["auto"]=true;
		}		
		cfgs[BridgeStatic]=c;

	}else if(cfgs.find(BridgeStatic)!=cfgs.end()){

		validatebridge_andsetdefault(cfg["config"]);

		if(cfg.isMember("auto")){
			cfgs[BridgeStatic].cfg["auto"]=true;
		}
		cfgs[BridgeStatic].cfg["config"]=cfg["config"];

	}else if(cfgs.find(BridgeRaw)!=cfgs.end()){

		validatebridge_andsetdefault(cfg["config"]);

		cfgs.erase(BridgeRaw);
		Configuration c(BridgeStatic);
		c.cfg["addressing"]="static";
		c.cfg["config"]=cfg["config"];
		c.cfg["name"]=ifname;
		c.cfg["type"]="bridge";
		if(cfg.isMember("auto")){
			c.cfg["auto"]=true;
		}				
		cfgs[BridgeStatic]=c;

	}else{
		throw std::runtime_error("Cant find config to update");
	}

	if(ifname==this->GetCurrentLanInterface()){
		// Update hosts file
		string ip;
		if(cfgs.find(EthStatic)!=cfgs.end()){
			ip=cfgs[EthStatic].cfg["config"]["address"][0u].asString();
		}else{
			ip=cfgs[BridgeStatic].cfg["config"]["address"][0u].asString();
		}

		Hosts h;
		string hostname=FileUtils::GetContentAsString("/proc/sys/kernel/hostname");
		Hosts::Entries e=h.Find(hostname);
		Hosts::UpdateIP(e,ip,hostname);
		h.Delete(hostname);
		h.Add(e);
		h.WriteBack();
	}

	pid_t pid;
	if((pid=Services::GetPid("dhclient."+ifname))!=0){
		kill(pid,SIGINT);
	}

	ifc->SetConfigurations(cfgs);

	if(PolicyController::Instance().GetInterfaceType(ifname)=="bridge"){
		this->SetPromisc(ifname,true);
	}

}

void InterfaceController::SetDynamicCfg(const string& ifname, const Json::Value& acfg){
	Json::Value cfg=acfg;
	auto_ptr<NetworkManager::Interface> ifc=this->GetInterface(ifname);
	map<Profile,Configuration> cfgs=ifc->GetConfigurations();

	if(cfgs.find(EthRaw)!=cfgs.end()){

		cfgs.erase(EthRaw);
		Configuration c(EthDynamic);
		c.cfg["addressing"]="dhcp";
		if(!cfg.isNull()){
			c.cfg["config"]=cfg["config"];
		}
		c.cfg["name"]=ifname;
		c.cfg["type"]="ethernet";
		if(cfg.isMember("auto")){
			c.cfg["auto"]=true;
		}				
		cfgs[EthDynamic]=c;

	}else if(cfgs.find(EthDynamic)!=cfgs.end()){

		if(!cfg.isNull()){
			cfgs[EthDynamic].cfg["config"]=cfg["config"];
		}
		if(cfg.isMember("auto")){
			cfgs[EthDynamic].cfg["auto"]=true;
		}		

	}else if(cfgs.find(EthStatic)!=cfgs.end()){
		cfgs.erase(EthStatic);

		Configuration c(EthDynamic);
		c.cfg["addressing"]="dhcp";
		if(!cfg.isNull()){
			c.cfg["config"]=cfg["config"];
		}
		c.cfg["name"]=ifname;
		c.cfg["type"]="ethernet";
		if(cfg.isMember("auto")){
			c.cfg["auto"]=true;
		}		
		cfgs[EthDynamic]=c;

	}else if(cfgs.find(BridgeDynamic)!=cfgs.end()){

		validatebridge_andsetdefault(cfg["config"]);
		cfgs[BridgeDynamic].cfg["config"]=cfg["config"];
		if(cfg.isMember("auto")){
			cfgs[BridgeDynamic].cfg["auto"]=true;
		}		


	}else if(cfgs.find(BridgeStatic)!=cfgs.end()){

		validatebridge_andsetdefault(cfg["config"]);

		cfgs.erase(BridgeStatic);
		Configuration c(BridgeDynamic);
		c.cfg["addressing"]="dhcp";
		c.cfg["config"]=cfg["config"];
		c.cfg["name"]=ifname;
		c.cfg["type"]="bridge";
		if(cfg.isMember("auto")){
			c.cfg["auto"]=true;
		}				
		cfgs[BridgeDynamic]=c;

	}else if(cfgs.find(BridgeRaw)!=cfgs.end()){

		validatebridge_andsetdefault(cfg["config"]);

		cfgs.erase(BridgeRaw);
		Configuration c(BridgeDynamic);
		c.cfg["addressing"]="dhcp";
		c.cfg["config"]=cfg["config"];
		c.cfg["name"]=ifname;
		c.cfg["type"]="bridge";
		if(cfg.isMember("auto")){
			c.cfg["auto"]=true;
		}		
		cfgs[BridgeDynamic]=c;

	}else{
		throw std::runtime_error("Cant find config to update");
	}

	// Remove any entries from hosts
	if(ifname==this->GetCurrentLanInterface()){
		Hosts h;
		string hostname=FileUtils::GetContentAsString("/proc/sys/kernel/hostname");
		h.Delete(hostname);
		h.WriteBack();
	}

	ifc->SetConfigurations(cfgs);

	if(PolicyController::Instance().GetInterfaceType(ifname)=="bridge"){
		this->SetPromisc(ifname,true);
	}

}

void InterfaceController::SetRawCfg(const string& ifname, const Json::Value& cfg){
	auto_ptr<NetworkManager::Interface> ifc=this->GetInterface(ifname);
	map<Profile,Configuration> cfgs=ifc->GetConfigurations();

	// Set common values.
	Configuration c;
	c.cfg["addressing"]="raw";
	c.cfg["name"]=ifname;


	if(cfgs.find(EthRaw)!=cfgs.end()){

		if(!cfg.isNull()){
			cfgs[EthRaw].cfg["config"]=cfg["config"];
		}

	}else if(cfgs.find(EthDynamic)!=cfgs.end()){

		cfgs.erase(EthDynamic);

		c.profile=EthRaw;
		if(!cfg.isNull()){
			c.cfg["config"]=cfg["config"];
		}
		c.cfg["type"]="ethernet";
		cfgs[EthRaw]=c;

	}else if(cfgs.find(EthStatic)!=cfgs.end()){
		cfgs.erase(EthStatic);

		c.profile=EthRaw;
		if(!cfg.isNull()){
			c.cfg["config"]=cfg["config"];
		}
		c.cfg["type"]="ethernet";
		cfgs[EthRaw]=c;

	}else if(cfgs.find(BridgeDynamic)!=cfgs.end()){

		cfgs.erase(BridgeDynamic);

		c.profile=BridgeRaw;
		if(!cfg.isNull()){
			c.cfg["config"]=cfg["config"];
		}
		c.cfg["type"]="bridge";
		cfgs[BridgeRaw]=c;

	}else if(cfgs.find(BridgeStatic)!=cfgs.end()){

		cfgs.erase(BridgeStatic);

		c.profile=BridgeRaw;
		if(!cfg.isNull()){
			c.cfg["config"]=cfg["config"];
		}
		c.cfg["type"]="bridge";
		cfgs[BridgeRaw]=c;

	}else if(cfgs.find(BridgeRaw)!=cfgs.end()){

		if(!cfg.isNull()){
			cfgs[BridgeRaw].cfg["config"]=cfg["config"];
		}

	}else{
		throw std::runtime_error("Cant find config to update");
	}

	// Remove any entries from hosts
	if(ifname==this->GetCurrentLanInterface()){
		Hosts h;
		string hostname=FileUtils::GetContentAsString("/proc/sys/kernel/hostname");
		h.Delete(hostname);
		h.WriteBack();
	}

	pid_t pid;
	if((pid=Services::GetPid("dhclient."+ifname))!=0){
		kill(pid,SIGINT);
	}

	ifc->SetConfigurations(cfgs);

	if(PolicyController::Instance().GetInterfaceType(ifname)=="bridge"){
		this->SetPromisc(ifname,true);
	}

}

bool InterfaceController::Up(const string& ifname){
	return do_call("/sbin/ifup "+ifname+" 2>/dev/null")==0;
}

bool InterfaceController::Down(const string& ifname){
	return do_call("/sbin/ifdown "+ifname+" 2>/dev/null")==0;
}


InterfaceController::~InterfaceController(){
}
