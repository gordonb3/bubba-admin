/*

    bubba-networkmanager - http://www.excito.com/

    Resolv.h - this file is part of bubba-networkmanager.

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

#ifndef MY_RESOLV_H
#define MY_RESOLV_H

#include <string>
#include <list>

#define NSFILE "/etc/resolv.conf"
//#define NSFILE "resolv.conf"

using namespace std;

namespace NetworkManager{

class Resolv{
private:

	string domain;
	string search;
	list<string> ns;

	Resolv();
	Resolv(Resolv& r);
	Resolv& operator=(const Resolv& r);
	void parse_cfg();
public:
	static Resolv& Instance();

	void Refresh();
	void Dump();

	const string& Domain(); 
	const string& Search();
	const list<string>& NS();
	void SetNS(const list<string>& nss);
	void SetDomain(const string& domain);
	void SetSearch(const string& search);
	
	void Write();

	virtual ~Resolv();

};
}
#endif
