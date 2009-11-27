/*
    
    bubba-networkmanager - http://www.excito.com/
    
    InterfaceController.h - this file is part of bubba-networkmanager.
    
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

#ifndef INTERFACECONTROLLER_H_
#define INTERFACECONTROLLER_H_

#include <memory>
#include <list>
#include <string>
using namespace std;

#include <libeutils/json/json.h>
#include "../datamodel/Configuration.h"
#include "../datamodel/Interface.h"
using namespace NetworkManager;

class InterfaceController{
protected:
	InterfaceController();
	InterfaceController(const InterfaceController& ifc);
	const InterfaceController& operator=(const InterfaceController& ifc);
public:
	static InterfaceController& Instance();

	auto_ptr<Interface> GetInterface(const string& ifname);
	int GetMtu(const string& ifname);
	bool SetMtu(const string& ifname, int mtu);

	list<string> GetInterfaces(const string& itype="");
	string GetCurrentWlanInterface();
	string GetDefaultLanInterface();
	string GetDefaultWanInterface();

	static bool Up(const string& ifname);
	static bool Down(const string& ifname);

	void SetStaticCfg(const string& ifname, const Json::Value& cfg);
	void SetDynamicCfg(const string& ifname, const Json::Value& cfg);
	void SetRawCfg(const string& ifname, const Json::Value& cfg);

	virtual ~InterfaceController();
};


#endif /* INTERFACECONTROLLER_H_ */
