/*

    bubba-networkmanager - http://www.excito.com/

    WlantInterface.h - this file is part of bubba-networkmanager.

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
#ifndef WLANINTERFACE_H_
#define WLANINTERFACE_H_

#include "EthernetInterface.h"

namespace NetworkManager {


class WlanInterface: public EthernetInterface {
protected:
	virtual void read_cfg();
	virtual void read_wlan_cfg();
	virtual void SetConfiguration(const Configuration& cfg);
public:
	WlanInterface(const string& device);


	virtual ~WlanInterface();
};

}

#endif /* WLANINTERFACE_H_ */
