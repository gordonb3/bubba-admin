/*

    bubba-networkmanager - http://www.excito.com/

    WlanCfg.h - this file is part of bubba-networkmanager.

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

#ifndef WLANCFG_H_
#define WLANCFG_H_

#include <map>
#include <string>
#include <list>
#include <libeutils/json/json.h>
#include <libeutils/SimpleCfg.h>
#include <libeutils/Regex.h>

using namespace std;

namespace NetworkManager{

class WlanCfg{
public:
	typedef enum {
		AUTH_NONE,
		AUTH_WEP,
		AUTH_WPA
	} AuthMode;

	typedef enum {
		MODE_A,
		MODE_B,
		MODE_G,
	} HWMode;

	typedef enum {
		ACL_NONE,
		ACL_ACCEPT,
		ACL_DENY
	} ACLMode;

	typedef enum {
		WPA1=1,
		WPA2=2,
		WPA12=3
	} WPAMode;

	Json::Value GetCFG();

	bool UpdateCFG(const Json::Value& data);

	static WlanCfg& Instance();

	bool Commit();

protected:
	WlanCfg();
	WlanCfg(const WlanCfg& cfg);
	WlanCfg& operator=(const WlanCfg& cfg);

	void GetDefaultCountry();
	void GetHWMode();
	void GetHTCapab();
	void GetACLMode();
	void GetAuth();
	void ParseConfig();
	bool WriteAclFiles();
	bool SyncACL();
	bool SyncHWMode();
	bool SyncHTCapab();
	bool SyncAuth();
	bool SyncWithCfg();
	bool JsonToCapab(const Json::Value& val);
	bool JsonToACL(const Json::Value& val);
	bool JsonToAuth(const Json::Value& val);
	bool JsonToWep(const Json::Value& val);
	bool JsonToWpa(const Json::Value& val);

private:
	EUtils::SimpleCfg cfg;
	map<string,string> tzinfo;

	string interface;

	// radio config
	string ssid;
	string langcode;
	HWMode hwmode;
	bool ieee80211n;
	bool ssidbroadcast;
	list<string> htcapab;
	int channel;

	// ACL
	ACLMode aclmode;
	list<string> macaccept;
	list<string> macdeny;

	// Auth
	AuthMode authmode;
	// Wep
	int wep_defaultkey;
	vector<string> wep_keys;
	//WPA
	WPAMode wpa_mode;
	vector<string> wpa_keys;
};

}

#endif /* WLANCFG_H_ */
