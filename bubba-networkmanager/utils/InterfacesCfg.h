/*
    
    bubba-networkmanager - http://www.excito.com/
    
    InterfacesCfg.h - this file is part of bubba-networkmanager.
    
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

#ifndef INTERFACESCFG_H_
#define INTERFACESCFG_H_

//#define IFSFILE "interfaces"
#define IFSFILE "/etc/network/interfaces"

#include <libeutils/json/json.h>


namespace NetworkManager{

class InterfacesCfg{
private:
	Json::Value cfg;
	InterfacesCfg();
	InterfacesCfg(InterfacesCfg& cfg);
	InterfacesCfg& operator=(const InterfacesCfg& cfg);

	void parse_cfg();
public:
	static InterfacesCfg& Instance();

	Json::Value GetCFG();
	bool UpdateCFG(const Json::Value& val);
	bool Commit();

	virtual ~InterfacesCfg();
};

}

#endif /* INTERFACESCFG_H_ */
