/*

    bubba-networkmanager - http://www.excito.com/

    Dispatcher.h - this file is part of bubba-networkmanager.

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

#include "Dispatcher.h"
#include "utils/SysConfig.h"
#include "utils/Route.h"
#include "utils/JsonUtils.h"
#include "utils/Resolv.h"
#include "controllers/InterfaceController.h"
#include "controllers/WlanController.h"
#include "controllers/PolicyController.h"
#include <signal.h>

#include <libeutils/json/json.h>

using namespace EUtils;

Dispatcher::Dispatcher(const string& sockpath, int timeout):NetDaemon(sockpath,timeout) {
// Add dispatch functions
	this->cmds["test"]=&Dispatcher::test;

	this->cmds["getwanif"]=&Dispatcher::getwanif;
	this->cmds["getlanif"]=&Dispatcher::getlanif;
	this->cmds["setwanif"]=&Dispatcher::setwanif;
	this->cmds["setlanif"]=&Dispatcher::setlanif;
	this->cmds["getifcfg"]=&Dispatcher::getifcfg;
	this->cmds["getdefaultroute"]=&Dispatcher::getdefaultroute;
	this->cmds["getroutes"]=&Dispatcher::getroutes;
	this->cmds["getinterfaces"]=&Dispatcher::getinterfaces;
	this->cmds["getwlanif"]=&Dispatcher::getwlanif;

	this->cmds["setnameservers"]=&Dispatcher::setnameservers;
	this->cmds["getnameservers"]=&Dispatcher::getnameservers;

	this->cmds["getmtu"]=&Dispatcher::getmtu;
	this->cmds["setmtu"]=&Dispatcher::setmtu;

	this->cmds["ifup"]=&Dispatcher::ifup;
	this->cmds["ifdown"]=&Dispatcher::ifdown;
	this->cmds["ifrestart"]=&Dispatcher::ifrestart;

	this->cmds["setstaticcfg"]=&Dispatcher::setstatic;
	this->cmds["setdynamiccfg"]=&Dispatcher::setdynamic;
	this->cmds["setrawcfg"]=&Dispatcher::setraw;

	this->cmds["setapif"]=&Dispatcher::setapif;
	this->cmds["haswlan"]=&Dispatcher::haswlan;
	this->cmds["setapcfg"]=&Dispatcher::setapcfg;
	this->cmds["setapssid"]=&Dispatcher::setapssid;
	this->cmds["setapmode"]=&Dispatcher::setapmode;
	this->cmds["setapchannel"]=&Dispatcher::setapchannel;
	this->cmds["setapauthnone"]=&Dispatcher::setapauthnone;
	this->cmds["setapauthwep"]=&Dispatcher::setapauthwep;
	this->cmds["setapauthwpa"]=&Dispatcher::setapauthwpa;

	signal(SIGINT, Dispatcher::sighandler);
	signal(SIGTERM,Dispatcher::sighandler);
}

void Dispatcher::Dispatch(UnixClientSocket* con){
	char recbuf[16384];
	ssize_t r=con->Receive(recbuf, 16384);
	Result res=Dispatcher::Done;
	if(r>0){
		recbuf[r]=0; // Make sure its terminated
		Json::Value res;
		bool success=reader.parse(recbuf,res);
		if(success){
			res=this->handle_request(con,res);
		}else{
			syslog(LOG_ERR,"Invalid request received");
		}
	}
	if(res!=Dispatcher::Spawned){
		this->decreq();
	}
}

Dispatcher::Result Dispatcher::handle_request(EUtils::UnixClientSocket *con, Json::Value & v){
	if(v.isMember("cmd") && v["cmd"].isString()){

		string cmd=v["cmd"].asString();

		if(this->cmds.find(cmd)!=this->cmds.end()){
			((*this).*cmds[cmd])(con,v);
		}else{
			Json::Value ret(Json::objectValue);
			ret["status"]=false;
			ret["error"]="Unknown command";
			this->send_jsonvalue(con,ret);
			return Dispatcher::Failed;
		}

	}else{
		return Dispatcher::Failed;
	}
	return Dispatcher::Done;
}

void Dispatcher::send_jsonvalue(EUtils::UnixClientSocket *con, Json::Value & v){
	string r=writer.write(v);
	con->Send(r.c_str(),r.length());
}

Dispatcher::Result Dispatcher::test(EUtils::UnixClientSocket* con, const Json::Value & v){
	Json::Value res(Json::objectValue);
	res["status"]=true;
	res["msg"]="Im testing";
	this->send_jsonvalue(con,res);
	return Dispatcher::Done;
}

void Dispatcher::sighandler(int sig){
	cout << "Got signal "<<sig<<endl;
}

Dispatcher::Result Dispatcher::setwanif(EUtils::UnixClientSocket *con, const Json::Value & v){

	// TODO: do update of HW

	Dispatcher::Result retval=Dispatcher::Done;

	Json::Value res(Json::objectValue);
	res["status"]=true;

	if(v.isMember("wanif") && v["wanif"].isString()){
		SysConfig& cfg=SysConfig::Instance();
		cfg.Update("wanif",v["wanif"].asString());
		if(!cfg.Writeback()){
			res["status"]=false;
			res["error"]="Failed to write config";
			retval=Dispatcher::Failed;
		}
	}else{
		res["status"]=false;
		res["error"]="Missing parameter";
		retval=Dispatcher::Failed;
	}

	this->send_jsonvalue(con,res);

	return retval;
}

Dispatcher::Result Dispatcher::getlanif(EUtils::UnixClientSocket *con, const Json::Value & v){

	Json::Value res(Json::objectValue);
	res["status"]=true;
	res["lanif"]=InterfaceController::Instance().GetCurrentLanInterface();
	this->send_jsonvalue(con,res);

	return Dispatcher::Done;
}

Dispatcher::Result Dispatcher::getwlanif(EUtils::UnixClientSocket *con, const Json::Value & v){

	Json::Value res(Json::objectValue);
	res["status"]=true;
	res["wlanif"]=InterfaceController::Instance().GetCurrentWlanInterface();
	this->send_jsonvalue(con,res);

	return Dispatcher::Done;
}


Dispatcher::Result Dispatcher::getinterfaces(EUtils::UnixClientSocket* con,const Json::Value& v){
	Json::Value res(Json::objectValue);
	res["status"]=true;

	string ifilter="";
	if(v.isMember("type") && v["type"].isString()){
		ifilter=v["type"].asString();
	}

	res["interfaces"]=JsonUtils::toArray(InterfaceController::Instance().GetInterfaces(ifilter));

	this->send_jsonvalue(con,res);

	return Dispatcher::Done;
}


Dispatcher::Result Dispatcher::setlanif(EUtils::UnixClientSocket *con, const Json::Value & v){

	// TODO: This does not belong at this level.

	Dispatcher::Result retval=Dispatcher::Done;

	Json::Value res(Json::objectValue);
	res["status"]=true;

	if(v.isMember("lanif") && v["lanif"].isString()){
		string newif=v["lanif"].asString();
		string newiftype=PolicyController::Instance().GetInterfaceType(newif);
		SysConfig& cfg=SysConfig::Instance();

		// Get old lanif
		string olf=cfg.ValueOrDefault("lanif",InterfaceController::Instance().GetDefaultLanInterface());
		if(olf==newif){
			// Same interface
			this->send_jsonvalue(con,res);
			return retval;
		}

		InterfaceController& ifc = InterfaceController::Instance();

		try{
			auto_ptr<Interface> in = ifc.GetInterface(olf);
			map<Profile,Configuration> cfgs=in->GetConfigurations();

			// save old interface
			map<Profile,Configuration> ocfgsave=cfgs;
			// Shut down old if
			InterfaceController::Down(olf);

			for(map<Profile,Configuration>::iterator cIt=cfgs.begin();cIt!=cfgs.end();cIt++){
				switch((*cIt).first){
				case NetworkManager::BridgeRaw:
				case NetworkManager::EthRaw:
				case NetworkManager::WlanRaw:
					InterfaceController::Instance().SetRawCfg(newif,Json::stringValue);
					break;
				case NetworkManager::BridgeDynamic:
				case NetworkManager::BridgeStatic:
					{
						Json::Value v((*cIt).second.cfg);
						if(v.isMember("config") && !v["config"].isNull()){
							if(v["config"].isMember("bridge_maxwait")){
								v["config"].removeMember("bridge_maxwait");
							}
							if(v["config"].isMember("bridge_ports")){
								v["config"].removeMember("bridge_ports");
							}
							if(v["addressing"]=="static"){
								InterfaceController::Instance().SetStaticCfg(newif,v["config"]);
							}else{
								InterfaceController::Instance().SetDynamicCfg(newif,v["config"]);
							}
						}else{
							if(v["addressing"]=="static"){
								InterfaceController::Instance().SetStaticCfg(newif,Json::Value(Json::objectValue));
							}else{
								InterfaceController::Instance().SetDynamicCfg(newif,Json::Value(Json::objectValue));
							}
						}
					}
					break;
				case NetworkManager::EthDynamic:
				case NetworkManager::EthStatic:
					{
						Json::Value v((*cIt).second.cfg);
						if(v.isMember("config") && !v["config"].isNull()){
							if(newiftype=="bridge"){
								v["config"]["bridge_maxwait"]=Json::Value(Json::arrayValue);
								v["config"]["bridge_maxwait"].append("0");
								v["config"]["bridge_ports"]=Json::Value(Json::arrayValue);
								v["config"]["bridge_ports"].append(InterfaceController::Instance().GetDefaultLanInterface());
								v["config"]["bridge_ports"].append(InterfaceController::Instance().GetCurrentWlanInterface());
							}
							if(v["addressing"]=="static"){
								InterfaceController::Instance().SetStaticCfg(newif,v["config"]);
							}else{
								InterfaceController::Instance().SetDynamicCfg(newif,v["config"]);
							}
						}else{
							if(v["addressing"]=="static"){
								InterfaceController::Instance().SetStaticCfg(newif,Json::Value(Json::objectValue));
							}else{
								InterfaceController::Instance().SetDynamicCfg(newif,Json::Value(Json::objectValue));
							}
						}
					}
					break;
				case NetworkManager::WlanAP:
					cout << "Wlan interface"<<endl;
					break;
				default:
					cout << "Unknown interface"<<endl;
				}
			}
			if(!res["status"]){
				// Try restoring original settings
				in->SetConfigurations(ocfgsave);
				InterfaceController::Up(olf);
			}else{
				// "Deactivate" old interface
				InterfaceController::Instance().SetRawCfg(olf,Json::Value(Json::objectValue));
				InterfaceController::Down(olf);
				// Pick up new if
				InterfaceController::Up(newif);
			}
		}catch(runtime_error& err){
			res["status"]=false;
			res["error"]=string("Operation failed: ")+err.what();
			retval=Dispatcher::Failed;
			this->send_jsonvalue(con,res);

			return retval;
		}

		cfg.Update("lanif",newif);
		if(!cfg.Writeback()){
			res["status"]=false;
			res["error"]="Failed to write config";
			retval=Dispatcher::Failed;
		}
	}else{
		res["status"]=false;
		res["error"]="Missing parameter";
		retval=Dispatcher::Failed;
	}

	this->send_jsonvalue(con,res);

	return retval;
}

Dispatcher::Result Dispatcher::getwanif(EUtils::UnixClientSocket *con, const Json::Value & v){

	Json::Value res(Json::objectValue);
	res["status"]=true;
	res["wanif"]=InterfaceController::Instance().GetCurrentWanInterface();
	this->send_jsonvalue(con,res);

	return Dispatcher::Done;
}

Dispatcher::Result Dispatcher::getifcfg(EUtils::UnixClientSocket *con, const Json::Value & v){
	Dispatcher::Result retval=Dispatcher::Done;

	Json::Value res(Json::objectValue);
	res["status"]=true;

	if(v.isMember("ifname") && v["ifname"].isString()){
		try{
			InterfaceController& ifc = InterfaceController::Instance();
			auto_ptr<Interface> in = ifc.GetInterface(v["ifname"].asString());

			map<Profile, Configuration> bcfg = in->GetConfigurations();
			for (map<Profile, Configuration>::iterator cIt = bcfg.begin(); cIt
					!= bcfg.end(); cIt++) {
				string type((*cIt).second.cfg["type"].asString());
				// TODO: fix higher up
				// Cludge, bridge is an ethernet device
				if(type=="bridge"){
					type="ethernet";
				}
				res["config"][type] = (*cIt).second.cfg;
			}

		}catch(std::runtime_error& err){
			res["status"]=false;
			res["error"]=string("Operation failed: ")+err.what();
			retval=Dispatcher::Failed;
		}

	}else{
		res["status"]=false;
		res["error"]="Missing parameter";
		retval=Dispatcher::Failed;
	}

	this->send_jsonvalue(con,res);

	return retval;
}

Dispatcher::Result Dispatcher::ifup(EUtils::UnixClientSocket* con,const Json::Value& v){
	Json::Value res(Json::objectValue);
	Dispatcher::Result ret=Dispatcher::Done;

	if(v.isMember("ifname") && v["ifname"].isString()){

		if(InterfaceController::Up(v["ifname"].asString())){
			res["status"]=true;
		}else{
			res["status"]=false;
			ret=Dispatcher::Failed;
		}

	}else{
		res["status"]=false;
		res["error"]="Missing ifname parameter";
		ret=Dispatcher::Failed;
	}

	this->send_jsonvalue(con,res);

	return ret;
}

Dispatcher::Result Dispatcher::ifdown(EUtils::UnixClientSocket* con,const Json::Value& v){
	Json::Value res(Json::objectValue);
	Dispatcher::Result ret=Dispatcher::Done;

	if(v.isMember("ifname") && v["ifname"].isString()){

		if(InterfaceController::Down(v["ifname"].asString())){
			res["status"]=true;
		}else{
			res["status"]=false;
			ret=Dispatcher::Failed;
		}

	}else{
		res["status"]=false;
		res["error"]="Missing ifname parameter";
		ret=Dispatcher::Failed;
	}

	this->send_jsonvalue(con,res);

	return ret;
}

Dispatcher::Result Dispatcher::ifrestart(EUtils::UnixClientSocket* con,const Json::Value& v){
	Json::Value res(Json::objectValue);
	Dispatcher::Result ret=Dispatcher::Done;

	if(v.isMember("ifname") && v["ifname"].isString()){

		if(InterfaceController::Down(v["ifname"].asString())
				&& InterfaceController::Up(v["ifname"].asString())){

			res["status"]=true;
		}else{
			res["status"]=false;
			ret=Dispatcher::Failed;
		}

	}else{
		res["status"]=false;
		res["error"]="Missing ifname parameter";
		ret=Dispatcher::Failed;
	}

	this->send_jsonvalue(con,res);

	return ret;
}


Dispatcher::Result Dispatcher::getdefaultroute(EUtils::UnixClientSocket *con, const Json::Value & v){
	Json::Value res(Json::objectValue);
	res["status"]=true;
	res["gateway"]=Route::Instance().Default()["gateway"];
	this->send_jsonvalue(con,res);

	return Dispatcher::Done;
}

Dispatcher::Result Dispatcher::getroutes(EUtils::UnixClientSocket *con, const Json::Value & v){
	Json::Value res(Json::objectValue);
	res["status"]=true;

	const list<map<string,string> >& routes=Route::Instance().Routes();

	for(list<map<string,string> >::const_iterator rIt=routes.begin();rIt!=routes.end();rIt++){
		res["routes"].append(JsonUtils::toObject(*rIt));
	}

	this->send_jsonvalue(con,res);

	return Dispatcher::Done;
}

Dispatcher::Result Dispatcher::setnameservers(EUtils::UnixClientSocket *con, const Json::Value & v){
	Json::Value res(Json::objectValue);
	res["status"]=true;

	if(!v.isMember("resolv") || !v["resolv"].isObject()){
		res["status"]=false;
		res["error"]="Missing mode parameter";
		this->send_jsonvalue(con,res);
		return Dispatcher::Failed;
	}

	Resolv& r=Resolv::Instance();
	bool modified=false;
	if(v["resolv"].isMember("domain") && v["resolv"]["domain"].isString()){
		r.SetDomain(v["resolv"]["domain"].asString());
		modified=true;
	}

	if(v["resolv"].isMember("search") && v["resolv"]["search"].isString()){
		r.SetSearch(v["resolv"]["search"].asString());
		modified=true;
	}

	if(v["resolv"].isMember("domain") && v["resolv"]["servers"].isArray()){
		r.SetNS(JsonUtils::ArrayToList(v["resolv"]["servers"]));
		modified=true;
	}

	if(modified){
		r.Write();
	}

	this->send_jsonvalue(con,res);

	return Dispatcher::Done;
}

Dispatcher::Result Dispatcher::getnameservers(EUtils::UnixClientSocket *con, const Json::Value & v){
	Json::Value res(Json::objectValue);
	res["status"]=true;
	Resolv& r=Resolv::Instance();
	res["resolv"]["domain"]=r.Domain();
	res["resolv"]["search"]=r.Search();
	res["resolv"]["servers"]=JsonUtils::toArray(r.NS());
	this->send_jsonvalue(con,res);

	return Dispatcher::Done;
}


Dispatcher::Result Dispatcher::getmtu(EUtils::UnixClientSocket* con,const Json::Value& v){
	Dispatcher::Result retval=Dispatcher::Done;
	Json::Value res(Json::objectValue);
	res["status"]=true;

	if(v.isMember("ifname") && v["ifname"].isString()){
		try{
			res["mtu"]=InterfaceController::Instance().GetMtu(v["ifname"].asString());
		}catch(std::runtime_error& err){
			res["status"]=false;
			res["error"]=string("Operation failed: ")+err.what();
			retval=Dispatcher::Failed;
		}
	}else{
		res["status"]=false;
		res["error"]="Missing parameter";
		retval=Dispatcher::Failed;
	}

	this->send_jsonvalue(con,res);

	return retval;
}

Dispatcher::Result Dispatcher::setmtu(EUtils::UnixClientSocket* con,const Json::Value& v){
	Dispatcher::Result retval=Dispatcher::Done;
	Json::Value res(Json::objectValue);
	res["status"]=true;

	if((v.isMember("ifname") && v["ifname"].isString()) && (v.isMember("mtu")&&v["mtu"].isInt() )){
		try{
			InterfaceController::Instance().SetMtu(v["ifname"].asString(),v["mtu"].asInt());
		}catch(std::runtime_error& err){
			res["status"]=false;
			res["error"]=string("Operation failed: ")+err.what();
			retval=Dispatcher::Failed;
		}
	}else{
		res["status"]=false;
		res["error"]="Missing parameter";
		retval=Dispatcher::Failed;
	}

	this->send_jsonvalue(con,res);

	return retval;
}

Dispatcher::Result Dispatcher::setstatic(EUtils::UnixClientSocket* con,const Json::Value& v){
	Dispatcher::Result retval=Dispatcher::Done;
	Json::Value res(Json::objectValue);
	res["status"]=true;

	if((v.isMember("ifname") && v["ifname"].isString()) && (v.isMember("config")&&v["config"].isObject() )){
		try{
			InterfaceController::Instance().SetStaticCfg(v["ifname"].asString(),v["config"]);
		}catch(std::runtime_error& err){
			res["status"]=false;
			res["error"]=string("Operation failed: ")+err.what();
			retval=Dispatcher::Failed;
		}
	}else{
		res["status"]=false;
		res["error"]="Missing parameter";
		retval=Dispatcher::Failed;
	}


	this->send_jsonvalue(con,res);

	return retval;
}

Dispatcher::Result Dispatcher::setdynamic(EUtils::UnixClientSocket* con,const Json::Value& v){
	Dispatcher::Result retval=Dispatcher::Done;
	Json::Value res(Json::objectValue);
	res["status"]=true;

	if((v.isMember("ifname") && v["ifname"].isString()) && (v.isMember("config")&&v["config"].isObject() )){
		try{
			InterfaceController::Instance().SetDynamicCfg(v["ifname"].asString(),v["config"]);
		}catch(std::runtime_error& err){
			res["status"]=false;
			res["error"]=string("Operation failed: ")+err.what();
			retval=Dispatcher::Failed;
		}
	}else{
		res["status"]=false;
		res["error"]="Missing parameter";
		retval=Dispatcher::Failed;
	}

	this->send_jsonvalue(con,res);

	return retval;
}

Dispatcher::Result Dispatcher::setraw(EUtils::UnixClientSocket* con,const Json::Value& v){
	Dispatcher::Result retval=Dispatcher::Done;
	Json::Value res(Json::objectValue);
	res["status"]=true;

	if((v.isMember("ifname") && v["ifname"].isString()) ){
		try{
			Json::Value arg(Json::nullValue);
			if(v.isMember("config")&&v["config"].isObject()){
				arg=v["config"];
			}
			InterfaceController::Instance().SetRawCfg(v["ifname"].asString(),arg);
		}catch(std::runtime_error& err){
			res["status"]=false;
			res["error"]=string("Operation failed: ")+err.what();
			retval=Dispatcher::Failed;
		}
	}else{
		res["status"]=false;
		res["error"]="Missing parameter";
		retval=Dispatcher::Failed;
	}

	this->send_jsonvalue(con,res);

	return retval;
}

Dispatcher::Result Dispatcher::setapif(EUtils::UnixClientSocket* con,const Json::Value& v){
	Dispatcher::Result retval=Dispatcher::Done;
	Json::Value res(Json::objectValue);
	res["status"]=true;

	try{
		if(v.isMember("ifname") && v["ifname"].isString()){
			WlanController::Instance().SetApInterface(v["ifname"].asString());
		}else{
			res["status"]=false;
			res["error"]=string("Missing interface parameter");
			retval=Dispatcher::Failed;
		}
	}catch(std::runtime_error& err){
		res["status"]=false;
		res["error"]=string("Operation failed: ")+err.what();
		retval=Dispatcher::Failed;
	}

	this->send_jsonvalue(con,res);

	return retval;
}

Dispatcher::Result Dispatcher::haswlan(EUtils::UnixClientSocket* con,const Json::Value& v){
	Dispatcher::Result retval=Dispatcher::Done;
	Json::Value res(Json::objectValue);
	res["status"]=true;

	try{
		res["wlan"]=WlanController::Instance().HasWlan();
	}catch(std::runtime_error& err){
		res["status"]=false;
		res["error"]=string("Operation failed: ")+err.what();
		retval=Dispatcher::Failed;
	}

	this->send_jsonvalue(con,res);

	return retval;
}

Dispatcher::Result Dispatcher::setapcfg(EUtils::UnixClientSocket* con,const Json::Value& v){
	Dispatcher::Result retval=Dispatcher::Done;
	Json::Value res(Json::objectValue);
	res["status"]=true;

	if(!v.isMember("config") || !v["config"].isObject()){
		res["status"]=false;
		res["error"]="Missing config parameter";
		this->send_jsonvalue(con,res);
		return Dispatcher::Failed;
	}

	if((v["config"].isMember("addressing") && v["config"]["addressing"].isString() && (v["config"]["addressing"].asString()=="ap")) ){
		try{
			WlanController::Instance().SetApCfg(v["ifname"].asString(),v["config"]);
		}catch(std::runtime_error& err){
			res["status"]=false;
			res["error"]=string("Operation failed: ")+err.what();
			retval=Dispatcher::Failed;
		}
	}else{
		res["status"]=false;
		res["error"]="Missing parameter";
		retval=Dispatcher::Failed;
	}


	this->send_jsonvalue(con,res);

	return retval;
}

Dispatcher::Result Dispatcher::setapssid(EUtils::UnixClientSocket* con,const Json::Value& v){
	Dispatcher::Result retval=Dispatcher::Done;
	Json::Value res(Json::objectValue);
	res["status"]=true;

	if(!v.isMember("ssid") || !v["ssid"].isString()){
		res["status"]=false;
		res["error"]="Missing ssid parameter";
		this->send_jsonvalue(con,res);
		return Dispatcher::Failed;
	}

	if((v.isMember("ifname") && v["ifname"].isString()) ){
		try{
			WlanController::Instance().SetApSSID(v["ifname"].asString(),v["ssid"].asString());
		}catch(std::runtime_error& err){
			res["status"]=false;
			res["error"]=string("Operation failed: ")+err.what();
			retval=Dispatcher::Failed;
		}
	}else{
		res["status"]=false;
		res["error"]="Missing parameter";
		retval=Dispatcher::Failed;
	}


	this->send_jsonvalue(con,res);

	return retval;
}

Dispatcher::Result Dispatcher::setapmode(EUtils::UnixClientSocket* con,const Json::Value& v){
	Dispatcher::Result retval=Dispatcher::Done;
	Json::Value res(Json::objectValue);
	res["status"]=true;

	if(!v.isMember("mode") || !v["mode"].isString()){
		res["status"]=false;
		res["error"]="Missing mode parameter";
		this->send_jsonvalue(con,res);
		return Dispatcher::Failed;
	}

	if(!v.isMember("ifname") || !v["ifname"].isString()){
		res["status"]=false;
		res["error"]="Missing mode parameter";
		this->send_jsonvalue(con,res);
		return Dispatcher::Failed;
	}

	try{
		WlanController::Instance().SetApMode(v["ifname"].asString(),v["mode"].asString());
	}catch(std::runtime_error& err){
		res["status"]=false;
		res["error"]=string("Operation failed: ")+err.what();
		retval=Dispatcher::Failed;
	}


	this->send_jsonvalue(con,res);

	return retval;
}

Dispatcher::Result Dispatcher::setapchannel(EUtils::UnixClientSocket* con,const Json::Value& v){
	Dispatcher::Result retval=Dispatcher::Done;
	Json::Value res(Json::objectValue);
	res["status"]=true;

	if(!v.isMember("channel") || !v["channel"].isInt()){
		res["status"]=false;
		res["error"]="Missing mode parameter";
		this->send_jsonvalue(con,res);
		return Dispatcher::Failed;
	}

	if(!v.isMember("ifname") || !v["ifname"].isString()){
		res["status"]=false;
		res["error"]="Missing mode parameter";
		this->send_jsonvalue(con,res);
		return Dispatcher::Failed;
	}

	try{
		WlanController::Instance().SetAPChannel(v["ifname"].asString(),v["channel"].asInt());
	}catch(std::runtime_error& err){
		res["status"]=false;
		res["error"]=string("Operation failed: ")+err.what();
		retval=Dispatcher::Failed;
	}


	this->send_jsonvalue(con,res);

	return retval;
}
Dispatcher::Result Dispatcher::setapauthnone(EUtils::UnixClientSocket* con,const Json::Value& v){
	Dispatcher::Result retval=Dispatcher::Done;
	Json::Value res(Json::objectValue);
	res["status"]=true;

	if(!v.isMember("ifname") || !v["ifname"].isString()){
		res["status"]=false;
		res["error"]="Missing mode parameter";
		this->send_jsonvalue(con,res);
		return Dispatcher::Failed;
	}

	try{
		WlanController::Instance().SetAPAuthNone(v["ifname"].asString());
	}catch(std::runtime_error& err){
		res["status"]=false;
		res["error"]=string("Operation failed: ")+err.what();
		retval=Dispatcher::Failed;
	}

	this->send_jsonvalue(con,res);

	return retval;
}

Dispatcher::Result Dispatcher::setapauthwep(EUtils::UnixClientSocket* con,const Json::Value& v){
	Dispatcher::Result retval=Dispatcher::Done;
	Json::Value res(Json::objectValue);
	res["status"]=true;

	if(!v.isMember("ifname") || !v["ifname"].isString()){
		res["status"]=false;
		res["error"]="Missing ifname parameter";
		this->send_jsonvalue(con,res);
		return Dispatcher::Failed;
	}

	if(!v.isMember("config") || !v["config"].isObject()){
		res["status"]=false;
		res["error"]="Missing config parameterblock";
		this->send_jsonvalue(con,res);
		return Dispatcher::Failed;
	}

	if(!v["config"].isMember("defaultkey") || !v["config"]["defaultkey"].isInt()){
		res["status"]=false;
		res["error"]="Missing defaultkey parameter";
		this->send_jsonvalue(con,res);
		return Dispatcher::Failed;
	}

	if(!v["config"].isMember("keys") || !v["config"]["keys"].isArray()){
		res["status"]=false;
		res["error"]="Missing key parameter";
		this->send_jsonvalue(con,res);
		return Dispatcher::Failed;
	}

	try{
		WlanController::Instance().SetAPAuthWep(v["ifname"].asString(),v["config"]);
	}catch(std::runtime_error& err){
		res["status"]=false;
		res["error"]=string("Operation failed: ")+err.what();
		retval=Dispatcher::Failed;
	}


	this->send_jsonvalue(con,res);

	return retval;
}

Dispatcher::Result Dispatcher::setapauthwpa(EUtils::UnixClientSocket* con,const Json::Value& v){
	Dispatcher::Result retval=Dispatcher::Done;
	Json::Value res(Json::objectValue);
	res["status"]=true;

	if(!v.isMember("ifname") || !v["ifname"].isString()){
		res["status"]=false;
		res["error"]="Missing ifname parameter";
		this->send_jsonvalue(con,res);
		return Dispatcher::Failed;
	}

	if(!v.isMember("config") || !v["config"].isObject()){
		res["status"]=false;
		res["error"]="Missing config parameterblock";
		this->send_jsonvalue(con,res);
		return Dispatcher::Failed;
	}

	if(!v["config"].isMember("mode") || !v["config"]["mode"].isString()){
		res["status"]=false;
		res["error"]="Missing mode parameter";
		this->send_jsonvalue(con,res);
		return Dispatcher::Failed;
	}

	if(!v["config"].isMember("keys") || !v["config"]["keys"].isArray()){
		res["status"]=false;
		res["error"]="Missing keys parameter";
		this->send_jsonvalue(con,res);
		return Dispatcher::Failed;
	}

	try{
		WlanController::Instance().SetAPAuthWpa(v["ifname"].asString(),v["config"]);
	}catch(std::runtime_error& err){
		res["status"]=false;
		res["error"]=string("Operation failed: ")+err.what();
		retval=Dispatcher::Failed;
	}


	this->send_jsonvalue(con,res);

	return retval;
}

Dispatcher::~Dispatcher() {
}
