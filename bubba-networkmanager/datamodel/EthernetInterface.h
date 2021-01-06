/*

    bubba-networkmanager - http://www.excito.com/

    EthernetInterface.h - this file is part of bubba-networkmanager.

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

#ifndef ETHERNETINTERFACE_H_
#define ETHERNETINTERFACE_H_

#include "Interface.h"

namespace NetworkManager {

class EthernetInterface: public NetworkManager::Interface {

protected:
	map<Profile,Configuration> configs;

	virtual void read_cfg();
	virtual void SetConfiguration(const Configuration& cfg);
public:
	EthernetInterface(const string& device);

	virtual map<Profile,Configuration> GetConfigurations();
	virtual void SetConfigurations(const map<Profile, Configuration>& cfgs);

	virtual ~EthernetInterface();
};

}

#endif /* ETHERNETINTERFACE_H_ */
