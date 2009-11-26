/*
    
    bubba-networkmanager - http://www.excito.com/
    
    Interface.h - this file is part of bubba-networkmanager.
    
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

#ifndef INTERFACE_H_
#define INTERFACE_H_

#include <string>
#include <map>

#include "Configuration.h"

using namespace std;

namespace NetworkManager{

class Interface{
protected:
	string name;
	Interface(const string& name);
public:
	Interface();
	virtual bool HasCapability(const string& cap);

	virtual map<Profile,Configuration> GetConfigurations();
	virtual void SetConfigurations(const map<Profile, Configuration>& cfgs);
	virtual void SetConfiguration(const Configuration& cfg);

	virtual ~Interface();
};

}

#endif /* INTERFACE_H_ */
