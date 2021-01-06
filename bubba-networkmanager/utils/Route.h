/*

    bubba-networkmanager - http://www.excito.com/

    Route.h - this file is part of bubba-networkmanager.

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

#ifndef MY_ROUTE_H

#include <iostream>
#include <list>
#include <string>
#include <map>

using namespace std;

namespace NetworkManager{

class Route{
private:
	typedef map<string,string> Hash;
	typedef list<Hash> Entries;
	Entries entries;
	
	string iptostring(const string& ip);
	void parse_route();
	void dump_map(const Hash& p);
	
	Route();

	Route(const Route& r);
	Route& operator=(const Route& r);
	
public:

	static Route& Instance();

	void Dump();

	map<string,string> Default();

	const list<map<string,string> >& Routes();

	void Refresh();

	virtual ~Route(){}
};
}
#endif
