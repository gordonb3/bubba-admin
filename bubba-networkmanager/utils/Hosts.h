/*
    
    bubba-networkmanager - http://www.excito.com/
    
    Hosts.h - this file is part of bubba-networkmanager.
    
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

#ifndef HOSTS_H_
#define HOSTS_H_

#include <list>
#include <vector>
#include <string>

using namespace std;

#define HOSTS_FILE "/etc/hosts"

namespace NetworkManager{

class Hosts{
public:
	typedef vector<string> Entry;
	typedef list<Entry> Entries;

private:

	Entries entries;
	const char* hosts_file;
	void init();

public:
	Hosts();
	Hosts( const char* hosts_file);

	/**
	 * Find all entries matching (Exactly) term
	 *
	 * @param term Term to find
	 *
	 * @return all lines that match term
	 */
	Entries Find(const string& term);

	/**
	 * Delete all entries matching term
	 *
	 * @param term Term to match
	 */
	void Delete(const string& term);

	/**
	 * Add entries to hosts file
	 *
	 * @param e Entries to add
	 */
	void Add(const Entries& e);

	/**
	 * Add one Entry to hosts
	 *
	 * @param e Entry to add
	 */
	void Add(const Entry& e);

	/**
	 * Write back hosts file
	 *
	 * @return true if operation success
	 */
	bool WriteBack();

	/**
	 * Write entries to std out
	 *
	 * @param e Entries to write
	 */
	void Dump(const Entries& e);

	/**
	 * Write all entries in hosts to stdout
	 */
	void Dump();

	/**
	 * Helper function given Entries it replaces IP numbers
	 * on every entry not being a loopback device
	 *
	 * @param e Entries to substitute IP on
	 * @param ip IP number to use when changing
	 * @param name Hostname to use if needed to recreate entries :(
	 */
	static void UpdateIP(Hosts::Entries& e, const string& ip, const string& name);

	/**
	 * Helper function, given Entries it replaces hostname on all
	 * lines. It will add the hostname and the hostname.localdomain
	 *
	 * @param e Entries to change name on
	 * @param name to change to
	 * @param ip conditional IP to use
	 */
	static void UpdateHostname(Hosts::Entries& e, const string& name, const string& ip="");

	virtual ~Hosts();
};



}


#endif /* HOSTS_H_ */
