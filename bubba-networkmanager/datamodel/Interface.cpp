/*
    
    bubba-networkmanager - http://www.excito.com/
    
    Interface.cpp - this file is part of bubba-networkmanager.
    
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

#include "Interface.h"
#include <stdexcept>

namespace NetworkManager{


Interface::Interface():name("None"){

}

Interface::Interface(const string& name):name(name){

}


bool Interface::HasCapability(const string& cap){
	return false;
}

void Interface::SetConfigurations(const map<Profile,Configuration>& cfgs){
	throw std::runtime_error("Method not available");
}

void Interface::SetConfiguration(const Configuration& cfg){
	throw std::runtime_error("Method not available");
}

map<Profile,Configuration> Interface::GetConfigurations(){
	throw std::runtime_error("Method not available");
	return map<Profile,Configuration>();
}

Interface::~Interface(){

}

}

