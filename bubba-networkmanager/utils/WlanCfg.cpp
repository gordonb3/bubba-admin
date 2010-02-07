/*

    bubba-networkmanager - http://www.excito.com/

    WlanCfg.cpp - this file is part of bubba-networkmanager.

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

#include "WlanCfg.h"
#include "JsonUtils.h"
#include "../utils/SysConfig.h"

#include <libeutils/FileUtils.h>
#include <libeutils/StringTools.h>
#include <libeutils/json/json.h>

#include <map>
#include <vector>
#include <algorithm>
#include <iostream>
#include <sstream>

#include <cstdlib>
#include <csignal>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <syslog.h>

using namespace EUtils;
using namespace std;

#define TZ_MAP_FILE "tz-lc.txt"
#define TZ_FILE "/etc/timezone"

#if 0
#define HOSTAPCONF "hostapd.conf"
#define HOSTAPDPID "hostapd.pid"
#define HOSTAPACLACCEPT "acl.accept"
#define HOSTAPACLDENY "acl.deny"
#endif

#if 1
#define HOSTAPCONF "/etc/hostapd/hostapd.conf"
#define HOSTAPDPID "/var/run/hostapd.pid"
#define HOSTAPACLACCEPT "/etc/hostapd/accept"
#define HOSTAPACLDENY "/etc/hostapd/deny"
#endif

namespace NetworkManager{

class MapMod{
public:
	MapMod(map<string,string>& m):inst(m){}
	void operator()(string s){
		list<string> val=StringTools::Split(s,'\t');
		inst[val.front()]=val.back();
	}
private:
	map<string,string>& inst;
};

#if 0
static void printpair(pair<string,string> s){
	cout <<"[" <<s.first<<"] ["<<s.second<<"]" <<endl;
}
#endif

static string addnl(string s){
	return s+"\n";
}


WlanCfg& WlanCfg::Instance(){
	static WlanCfg cfg;
	return cfg;
}

WlanCfg::WlanCfg():cfg(HOSTAPCONF){
	this->ParseConfig();
}

void WlanCfg::GetDefaultCountry(){
	list<string> tz=FileUtils::GetContent(SysConfig::Instance().ValueOrDefault("tz_map_file", TZ_MAP_FILE));
	list<string> t;
	t.resize(tz.size());
	MapMod mm(tzinfo);
	for_each(tz.begin(),tz.end(),mm);
#if 0
	for_each(tzinfo.begin(),tzinfo.end(),printpair);
#endif

	string tzfile=FileUtils::GetContentAsString(SysConfig::Instance().ValueOrDefault("tz_file", TZ_FILE));
	if(tzinfo.find(tzfile)==tzinfo.end()){
		syslog( LOG_WARNING, "Timezone was not recognized, or UTC was selected. defaulting to world domain (00)" );
		this->langcode = "00";
	}else{
		this->langcode=tzinfo[tzfile];
	}
}

void WlanCfg::GetHTCapab(){
	string hw=cfg.ValueOrDefault("ht_capab","");
	string _search[] = {
		"LDPC",
		"HT40-",
		"HT40+",
		"SMPS-STATIC",
		"SMPS-DYNAMIC",
		"GF",
		"SHORT-GI-20",
		"SHORT-GI-40",
		"TX-STBC",
		"RX-STBC1",
		"RX-STBC12",
		"RX-STBC123",
		"DELAYED-BA",
		"MAX-AMSDU-7935",
		"DSSS_CCK-40",
		"PSMP",
		"LSIG-TXOP-PROT"
	};
	list<string> search (_search, _search + sizeof(_search) / sizeof(string) );
	for( list<string>::iterator it = search.begin(); it != search.end(); it++ ) {
		string test = "[";
		test += *it;
		test += "]";
		if( hw.find( test ) != string::npos ) {
			this->htcapab.push_back(*it);
		}
	}
}

void WlanCfg::GetHWMode(){
	string hw=cfg.ValueOrDefault("hw_mode","");

	if(hw=="a"){
		this->hwmode=MODE_A;
	}else if(hw=="b"){
		this->hwmode=MODE_B;
	}else if(hw=="g"){
		this->hwmode=MODE_G;
	}else{
		cerr << "Unable to parse hwmode"<<endl;
	}
}

void WlanCfg::GetACLMode(){
	string acl=cfg.ValueOrDefault("macaddr_acl","");
	if(acl==""){
		this->aclmode=ACL_NONE;
		return;
	}

	if(acl=="0"){
		this->aclmode=ACL_ACCEPT;
	}else if(acl=="1"){
		this->aclmode=ACL_DENY;
	}else{
		cerr << "Unknown MAC ACL mode"<<endl;
		this->aclmode=ACL_NONE;
		return;
	}
	string accept=cfg.ValueOrDefault("accept_mac_file",HOSTAPACLACCEPT);
	if(accept==""){
		cerr << "Missing acl accept for mac"<<endl;
	}else{
		if(Stat::FileExists(accept)){
			this->macaccept=FileUtils::GetContent(accept);
		}
	}

	string deny=cfg.ValueOrDefault("deny_mac_file",HOSTAPACLDENY);
	if(deny==""){
		cerr << "Missing acl deny for mac"<<endl;
	}else{
		if(Stat::FileExists(deny)){
			this->macdeny=FileUtils::GetContent(deny);
		}
	}

}

void WlanCfg::GetAuth(){
	string wep=cfg.ValueOrDefault("wep_default_key","");
	string wpa=cfg.ValueOrDefault("wpa","");

	if(wep=="" && wpa==""){
		this->authmode=AUTH_NONE;
		return;
	}

	if(wep!=""){
		this->authmode=AUTH_WEP;
		this->wep_defaultkey=atoi(wep.c_str());

		int i=0;
		string key;
		this->wep_keys.clear();
		do{
			stringstream ss;
			ss << "wep_key"<<i;
			key=cfg.ValueOrDefault(ss.str(),"");
			if(key!=""){
				this->wep_keys.push_back(key);
			}
			i++;
		}while(key!="");

	}else if(wpa!=""){
		this->authmode=AUTH_WPA;
		int wpamode=atoi(wpa.c_str());
		switch(wpamode){
		case 1:
			this->wpa_mode=WPA1;
			break;
		case 2:
			this->wpa_mode=WPA2;
			break;
		case 3:
			this->wpa_mode=WPA12;
			break;
		default:
			cerr << "Unknown wpa-mode"<<endl;
			break;
		}

		// Read and store keyfile
		string kfile=cfg.ValueOrDefault("wpa_psk_file","");
		this->wpa_keys.clear();
		if(kfile!=""){
			if(Stat::FileExists(kfile)){
				list<string> keys=FileUtils::GetContent(kfile);
				this->wpa_keys.resize(keys.size()+1);
				copy(keys.begin(),keys.end(),this->wpa_keys.begin());
			}
		}

		// Add "static" konfig
		string wpakey=cfg.ValueOrDefault("wpa_passphrase","");
		if(wpakey!=""){
			this->wpa_keys.insert(this->wpa_keys.begin(),wpakey);
		}

	}else{
		cerr << "Unknown auth config"<<endl;
		this->authmode=AUTH_NONE;
		return;
	}

}

void WlanCfg::ParseConfig(){
	this->ssid=cfg.ValueOrDefault("ssid","");
	this->GetDefaultCountry();
	this->GetACLMode();
	this->GetHWMode();
	this->GetHTCapab();
	this->ieee80211n=cfg.ValueOrDefault("ieee80211n","0") == "1";
	this->ssidbroadcast=cfg.ValueOrDefault("ignore_broadcast_ssid","0") == "0";
	this->channel=atoi(cfg.ValueOrDefault("channel","6").c_str());
	this->interface=cfg.ValueOrDefault("interface","");
	this->GetAuth();
}

bool WlanCfg::JsonToCapab(const Json::Value& val){
	this->htcapab=JsonUtils::ArrayToList(val);
	return true;
}


bool WlanCfg::JsonToACL(const Json::Value& val){

	if(val.isMember("mode") && val["mode"].isString()){
		string mode=val["mode"].asString();
		if(mode=="none"){
			this->aclmode=ACL_NONE;
		}else if(mode=="accept"){
			this->aclmode=ACL_ACCEPT;
		}else if(mode=="deny"){
			this->aclmode=ACL_DENY;
		}else{
			return false;
		}
	}

	if(val.isMember("deny") && val["deny"].isArray()){
		this->macdeny=JsonUtils::ArrayToList(val["deny"]);
	}

	if(val.isMember("accept") && val["accept"].isArray()){
		this->macaccept=JsonUtils::ArrayToList(val["accept"]);
	}

	return true;
}

bool WlanCfg::JsonToWep(const Json::Value& val){

	if(val.isMember("defaultkey") && val["defaultkey"].isIntegral()){
		this->wep_defaultkey=val["defaultkey"].asInt();
	}

	if(val.isMember("keys") && val["keys"].isArray()){
		this->wep_keys=JsonUtils::ArrayToVector(val["keys"]);
	}
	if((size_t)this->wep_defaultkey>this->wep_keys.size()){
		return false;
	}
	return true;
}

bool WlanCfg::JsonToWpa(const Json::Value& val){

	if(val.isMember("mode") && val["mode"].isString()){
		string mode=val["mode"].asString();
		if(mode=="wpa1"){
			this->wpa_mode=WPA1;
		}else if(mode=="wpa2"){
			this->wpa_mode=WPA2;
		}else if(mode=="wpa12"){
			this->wpa_mode=WPA12;
		}else{
			return false;
		}
	}

	if(val.isMember("keys") && val["keys"].isArray()){
		this->wpa_keys=JsonUtils::ArrayToVector(val["keys"]);
	}

	return true;
}



bool WlanCfg::JsonToAuth(const Json::Value& val){
	if(val.isMember("mode") && val["mode"].isString()){
		string mode=val["mode"].asString();
		if(mode=="none"){
			this->authmode=AUTH_NONE;
		}else if(mode=="wep"){
			if(val.isMember("wep") && val["wep"].isObject()){
				if(this->JsonToWep(val["wep"])){
					this->authmode=AUTH_WEP;
				}else{
					return false;
				}
			}else{
				return false;
			}
		}else if(mode=="wpa"){
			if(val.isMember("wpa") && val["wpa"].isObject()){
				if(this->JsonToWpa(val["wpa"])){
					this->authmode=AUTH_WPA;
				}else{
					return false;
				}
			}else{
				return false;
			}
		}else{
			return false;
		}

	}
	return true;
}

Json::Value WlanCfg::GetCFG(){
	Json::Value ret(Json::objectValue);

	ret["ssid"]=this->ssid;
	ret["country"]=this->langcode;
	ret["channel"]=this->channel;
	ret["interface"]=this->interface;

	ret["80211n"]= this->ieee80211n;
	ret["ssidbroadcast"]= this->ssidbroadcast;
	ret["ht_capab"]=JsonUtils::toArray(this->htcapab);
	switch(this->hwmode){
	case MODE_A:
		ret["mode"]="a";
		break;
	case MODE_B:
		ret["mode"]="b";
		break;
	case MODE_G:
		ret["mode"]="g";
		break;
	default:
		ret["mode"]="unknown";
		break;
	}

	ret["acl"]=Json::objectValue;
	switch(this->aclmode){
	case ACL_NONE:
		ret["acl"]["mode"]="none";
		break;
	case ACL_ACCEPT:
		ret["acl"]["mode"]="accept";
		break;
	case ACL_DENY:
		ret["acl"]["mode"]="deny";
		break;
	default:
		ret["acl"]["mode"]="unknown";
		break;
	}
	ret["acl"]["accept"]=JsonUtils::toArray(this->macaccept);
	ret["acl"]["deny"]=JsonUtils::toArray(this->macdeny);

	ret["auth"]=Json::objectValue;
	switch(this->authmode){
	case AUTH_NONE:
		ret["auth"]["mode"]="none";
		break;
	case AUTH_WEP:
		ret["auth"]["mode"]="wep";
		break;
	case AUTH_WPA:
		ret["auth"]["mode"]="wpa";
		break;
	default:
		ret["auth"]["mode"]="unknown";
	}

	if(this->authmode==AUTH_WEP){
		ret["auth"]["wep"]=Json::objectValue;
		ret["auth"]["wep"]["defaultkey"]=this->wep_defaultkey;
		ret["auth"]["wep"]["keys"]=JsonUtils::toArray(this->wep_keys);
	}

	if(this->authmode==AUTH_WPA){
		ret["auth"]["wpa"]=Json::objectValue;
		switch(this->wpa_mode){
		case WPA1:
			ret["auth"]["wpa"]["mode"]="wpa1";
			break;
		case WPA2:
			ret["auth"]["wpa"]["mode"]="wpa2";
			break;
		case WPA12:
			ret["auth"]["wpa"]["mode"]="wpa12";
			break;
		default:
			ret["auth"]["wpa"]["mode"]="unknown";
		}

		ret["auth"]["wpa"]["keys"]=JsonUtils::toArray(this->wpa_keys);
	}
	return ret;
}

bool WlanCfg::UpdateCFG(const Json::Value& val){

	if(val.isMember("interface") && val["interface"].isString()){
		this->interface=val["interface"].asString();
	}

	if(val.isMember("ssid") && val["ssid"].isString()){
		this->ssid=val["ssid"].asString();
	}

	if(val.isMember("channel") && val["channel"].isIntegral()){
		this->channel=val["channel"].asInt();
	}

	if(val.isMember("country") && val["country"].isString()){
		this->langcode=val["country"].asString();
	}

	if(val.isMember("80211n") && val["80211n"].isBool()){
		this->ieee80211n = val["80211n"].asBool();
	}

	if(val.isMember("ssidbroadcast") && val["ssidbroadcast"].isBool()){
		this->ssidbroadcast = val["ssidbroadcast"].asBool();
	}

	if(val.isMember("mode") && val["mode"].isString()){
		char mode=val["mode"].asString()[0];
		switch(mode){
		case 'a':
			this->hwmode=MODE_A;
			break;
		case 'b':
			this->hwmode=MODE_B;
			break;
		case 'g':
			this->hwmode=MODE_G;
			break;
		default:
			cerr << "Illegal hwmode"<<endl;
			return false;
			break;
		}
	}
	if(val.isMember("ht_capab") && val["ht_capab"].isArray()){
		if(!this->JsonToCapab(val["ht_capab"])){
			return false;
		}
	}
	if(val.isMember("acl") && val["acl"].isObject()){
		if(!this->JsonToACL(val["acl"])){
			return false;
		}
	}
	if(val.isMember("auth") && val["auth"].isObject()){
		if(!this->JsonToAuth(val["auth"])){
			return false;
		}
	}

	if(!this->SyncWithCfg()){
		return false;
	}

	if(!this->cfg.Writeback()){
		return false;
	}

	return true;
}

bool WlanCfg::WriteAclFiles(){

	list<string> cnt;

	string accept=cfg.ValueOrDefault("accept_mac_file",HOSTAPACLACCEPT);
	if(accept!=""){
		mode_t wmode=0664;
		if(Stat::FileExists(accept)){
			wmode=FileUtils::StatFile(accept).GetMode();
		}

		cnt.resize(this->macaccept.size());
		transform(this->macaccept.begin(),this->macaccept.end(),cnt.begin(),addnl);
		string tmpfile=accept+".new";
		if(FileUtils::Write(tmpfile,cnt,wmode)){
			if(rename(tmpfile.c_str(),accept.c_str())!=0){
				return false;
			}
		}else{
			return false;
		}
	}

	string deny=cfg.ValueOrDefault("deny_mac_file",HOSTAPACLDENY);
	if(deny!=""){
		mode_t wmode=0664;
		if(Stat::FileExists(deny)){
			wmode=FileUtils::StatFile(deny).GetMode();
		}
		cnt.resize(this->macdeny.size());
		transform(this->macdeny.begin(),this->macdeny.end(),cnt.begin(),addnl);
		string tmpfile=deny+".new";
		if(FileUtils::Write(tmpfile,cnt,wmode)){
			if(rename(tmpfile.c_str(),deny.c_str())!=0){
				return false;
			}
		}else{
			return false;
		}
	}

	return true;
}

bool WlanCfg::SyncACL(){
	switch(this->aclmode){
	case ACL_NONE:
		cfg.Remove("macaddr_acl");
		break;
	case ACL_ACCEPT:
		cfg.Update("macaddr_acl","0");
		break;
	case ACL_DENY:
		cfg.Update("macaddr_acl","1");
		break;
	default:
		return false;
	}
	if(!this->WriteAclFiles()){
		return false;
	}
	return true;
}

bool WlanCfg::SyncHWMode(){

	switch(this->hwmode){
	case MODE_A:
		cfg.Update("hw_mode","a");
		break;
	case MODE_B:
		cfg.Update("hw_mode","b");
		break;
	case MODE_G:
		cfg.Update("hw_mode","g");
		break;
	default:
		return false;
	}

	return true;
}
bool WlanCfg::SyncHTCapab(){

	list<string> capab = this->htcapab;
	string str;

	for( list<string>::iterator it = capab.begin(); it != capab.end(); ++it ) {
		str += "[";
		str += *it;
		str += "]";
	}
	cfg.Update("ht_capab",str);

	return true;
}

bool WlanCfg::SyncAuth(){

	switch(this->authmode){
	case AUTH_NONE:
		cfg.Remove("wep_default_key");
		cfg.Remove("wpa");
		cfg.Remove("wpa_passphrase");
		break;
	case AUTH_WEP:
		{
			if(this->wep_keys.size()<1){
				return false;
			}
			cfg.Remove("wpa");
			cfg.Remove("wpa_passphrase");
			stringstream ss;
			ss<<this->wep_defaultkey;
			cfg.Update("wep_default_key",ss.str());

			// Erase all old keys
			string key;
			int i=0;
			do{
				stringstream ss;
				ss << "wep_key"<<i;
				key=cfg.ValueOrDefault(ss.str(),"");
				if(key!=""){
					cfg.Remove(ss.str());
				}
				i++;
			}while(key!="");

			// Re-add
			for(size_t i=0; i<this->wep_keys.size();i++){
				stringstream wk;
				wk<<"wep_key"<<i;
				cfg.Update(wk.str(),this->wep_keys[i]);
			}
		}
		break;
	case AUTH_WPA:
		{
			if(this->wpa_keys.size()<1){
				return false;
			}
			cfg.Remove("wep_default_key");
			switch(this->wpa_mode){
			case WPA1:
				cfg.Update("wpa","1");
				break;
			case WPA2:
				cfg.Update("wpa","2");
				break;
			case WPA12:
				cfg.Update("wpa","3");
				break;
			default:
				return false;
			}
			list<string> tkeys(this->wpa_keys.size());
			copy(this->wpa_keys.begin(),this->wpa_keys.end(),tkeys.begin());
			cfg.Update("wpa_passphrase",tkeys.front());
			tkeys.pop_front();

			string kfile=cfg.ValueOrDefault("wpa_psk_file","/etc/hostapd/hostapd.wpa_psk");
			if(tkeys.size()>0){
				mode_t wmode=0640;
				if(Stat::FileExists(kfile)){
					wmode=FileUtils::StatFile(kfile).GetMode();
				}
				list<string> cnt(tkeys.size());
				transform(tkeys.begin(),tkeys.end(),cnt.begin(),addnl);

				//TODO: Update Fileutils::Write to a template...
				if(!FileUtils::Write(kfile,cnt,wmode)){
					return false;
				}
			}else{
				if(Stat::FileExists(kfile)){
					// Empty file to remove any keys
					FileUtils::Write(kfile,"",FileUtils::StatFile(kfile).GetMode());
				}
			}

		}
		break;
	default:
		return false;
	}


	return true;
}

bool WlanCfg::SyncWithCfg(){

	if(this->ssid!=""){
		cfg.Update("ssid",this->ssid);
	}

	if(this->interface!=""){
		cfg.Update("interface",this->interface);
	}

	cfg.Update("country_code",this->langcode);

	if(!this->SyncHWMode()){
		return false;
	}

	if(!this->SyncHTCapab()){
		return false;
	}

	cfg.Update("ieee80211n", this->ieee80211n ? "1" : "0" );
	cfg.Update("ignore_broadcast_ssid", this->ssidbroadcast ? "0" : "1" );

	stringstream ss;
	ss << this->channel;
	cfg.Update("channel",ss.str());

	if(!this->SyncACL()){
		return false;
	}

	if(!this->SyncAuth()){
		return false;
	}

	return true;
}

bool WlanCfg::Commit(){


	pid_t pid=atoi(FileUtils::GetContentAsString(HOSTAPDPID).c_str());
	if(pid>0){
		if(kill(pid,SIGHUP)==-1){
			return false;
		}
	}else{
		return false;
	}
	return true;

}

}
