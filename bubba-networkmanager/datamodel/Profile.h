/*
    
    bubba-networkmanager - http://www.excito.com/
    
    Profile.h - this file is part of bubba-networkmanager.
    
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

#ifndef PROFILE_H_
#define PROFILE_H_

namespace NetworkManager{

enum Profile{
	None,
	EthRaw,
	EthStatic,
	EthDynamic,
	WlanRaw,
	WlanAP,
	BridgeStatic,
	BridgeDynamic,
	BridgeRaw
};

}


#endif /* PROFILE_H_ */
