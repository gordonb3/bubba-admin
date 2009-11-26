/*
    
    bubba-networkmanager - http://www.excito.com/
    
    PolicyController.h - this file is part of bubba-networkmanager.
    
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

/*
 * PolicyController.h
 *
 *  Created on: Oct 12, 2009
 *      Author: tor
 */
#ifndef POLICYCONTROLLER_H
#define POLICYCONTROLLER_H

#include <string>
#include <map>

#include "../datamodel/Profile.h"

using namespace NetworkManager;
using namespace std;

class PolicyController{
private:
	map<string,string> ifprofile;
	map<string,map<string,bool> > policies;
protected:
	PolicyController();
	PolicyController(const PolicyController&);
	const PolicyController& operator=(const PolicyController&);
public:
	static PolicyController& Instance();

	string GetInterfaceType(const string& ifname);

	bool Allowed(const string& actor, const string& policy);

	virtual ~PolicyController();
};

#endif
