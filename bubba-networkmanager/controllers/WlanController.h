/*

    bubba-networkmanager - http://www.excito.com/

    WlanController.h - this file is part of bubba-networkmanager.

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

#ifndef WLANCONTROLLER_H_
#define WLANCONTROLLER_H_

#include <string>

#include <libeutils/json/json.h>

using namespace std;

class WlanController {
protected:
	WlanController();
	WlanController(const WlanController& wc);
	const WlanController& operator=(const WlanController& wc);

public:
	static WlanController& Instance();

	bool HasWlan();
	void SetApCfg(const string& ifname, const Json::Value& cfg);
	void SetApInterface(const string& ifname);
	void SetApSSID(const string& ifname, const string& ssid);
	void SetApMode(const string& ifname, const string& mode);
	void SetAPChannel(const string& ifname, int channel);
	void SetAPAuthNone(const string& ifname);
	void SetAPAuthWep(const string& ifname, const Json::Value& cfg);
	void SetAPAuthWpa(const string& ifname, const Json::Value& cfg);

	virtual ~WlanController();
};

#endif /* WLANCONTROLLER_H_ */
