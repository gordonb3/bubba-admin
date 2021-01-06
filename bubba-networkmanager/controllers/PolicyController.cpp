/*

    bubba-networkmanager - http://www.excito.com/

    PolicyController.cpp - this file is part of bubba-networkmanager.

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

#include "PolicyController.h"

/*
 * TODO: For now hardcode policies later think of better solution
 */

PolicyController::PolicyController(){
	// Profile types
	ifprofile["eth0"]="ether";
	ifprofile["eth1"]="ether";
	ifprofile["eth2"]="ether";
	ifprofile["eth3"]="ether";
	ifprofile["eth4"]="ether";
	ifprofile["wlan0"]="wlan";
	ifprofile["wlan1"]="wlan";
	ifprofile["wlan2"]="wlan";
	ifprofile["wlan3"]="wlan";
	ifprofile["wlan4"]="wlan";
	ifprofile["br0"]="bridge";

	policies["eth0"]["wan"]=true;
	policies["eth1"]["wan"]=false;
	policies["eth2"]["wan"]=false;
	policies["eth3"]["wan"]=false;
	policies["eth4"]["wan"]=false;
	policies["wlan0"]["wan"]=false;
	policies["wlan1"]["wan"]=false;
	policies["wlan2"]["wan"]=false;
	policies["wlan3"]["wan"]=false;
	policies["wlan4"]["wan"]=false;
	policies["br0"]["wan"]=false;

	policies["eth0"]["lan"]=false;
	policies["eth1"]["lan"]=true;
	policies["eth2"]["lan"]=true;
	policies["eth3"]["lan"]=true;
	policies["eth4"]["lan"]=true;
	policies["wlan0"]["lan"]=true;
	policies["wlan1"]["lan"]=true;
	policies["wlan2"]["lan"]=true;
	policies["wlan3"]["lan"]=true;
	policies["wlan4"]["lan"]=true;
	policies["br0"]["lan"]=true;

	policies["eth0"]["iswlan"]=false;
	policies["eth1"]["iswlan"]=false;
	policies["eth2"]["iswlan"]=false;
	policies["eth3"]["iswlan"]=false;
	policies["eth4"]["iswlan"]=false;
	policies["wlan0"]["iswlan"]=true;
	policies["wlan1"]["iswlan"]=true;
	policies["wlan2"]["iswlan"]=true;
	policies["wlan3"]["iswlan"]=true;
	policies["wlan4"]["iswlan"]=true;
	policies["br0"]["iswlan"]=false;


	policies["eth0"]["promisc"]=false;
	policies["eth1"]["promisc"]=false;
	policies["eth2"]["promisc"]=false;
	policies["eth3"]["promisc"]=false;
	policies["eth4"]["promisc"]=false;
	policies["wlan0"]["promisc"]=false;
	policies["wlan1"]["promisc"]=false;
	policies["wlan2"]["promisc"]=false;
	policies["wlan3"]["promisc"]=false;
	policies["wlan4"]["promisc"]=false;
	policies["br0"]["promisc"]=true;


}



PolicyController & PolicyController::Instance(){
	static PolicyController pc;

	return pc;
}



string PolicyController::GetInterfaceType(const string & ifname){
	// Todo, replace with some regex magic
	if(ifprofile.find(ifname)==ifprofile.end()){
		throw std::runtime_error("Unknown interface");
	}
	return ifprofile[ifname];
}



bool PolicyController::Allowed(const string & actor, const string & policy){
	if(policies.find(actor)==policies.end()){
		throw std::runtime_error("Unknown actor "+actor);
	}
	if(policies[actor].find(policy)==policies[actor].end()){
		throw std::runtime_error("Unknown policy "+policy);
	}

	return policies[actor][policy];

}



PolicyController::~PolicyController(){
}
