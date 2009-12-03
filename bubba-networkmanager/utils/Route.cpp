/*

    bubba-networkmanager - http://www.excito.com/

    Route.cpp - this file is part of bubba-networkmanager.

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

#include "Route.h"

#include <libeutils/FileUtils.h>
#include <libeutils/StringTools.h>

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

using namespace EUtils;
namespace NetworkManager{


static int hextoval(char a, char b){
	a=toupper(a)>57?a-55:a-48;
	b=toupper(b)>57?b-55:b-48;
	return a*16+b;
}

#include <endian.h>

string Route::iptostring(const string& ip){

	char buf[128];
#if __BYTE_ORDER == __LITTLE_ENDIAN
	sprintf(buf,"%d.%d.%d.%d",
				hextoval(ip[6],ip[7]),
				hextoval(ip[4],ip[5]),
				hextoval(ip[2],ip[3]),
				hextoval(ip[0],ip[1]));
#elif __BYTE_ORDER == __BIG_ENDIAN
	sprintf(buf,"%d.%d.%d.%d",
				hextoval(ip[0],ip[1]),
				hextoval(ip[2],ip[3]),
				hextoval(ip[4],ip[5]),
				hextoval(ip[6],ip[7]));
#else
	#error "No endian defined"
#endif


	return string(buf);
}

void Route::parse_route(){
	list<string> fil=FileUtils::GetContent("/proc/net/route");
	fil.pop_front();
	for(list<string>::iterator fIt=fil.begin();fIt!=fil.end();fIt++){
		list<string> line=StringTools::Split(*fIt,'\t');
		if(line.size()==11){
			list<string>::iterator lIt=line.begin();
			Hash e;
			e["iface"]=*lIt++;
			e["destination"]=iptostring(*lIt++);
			e["gateway"]=iptostring(*lIt++);
			e["flags"]=*lIt++;
			e["refcnt"]=*lIt++;
			e["use"]=*lIt++;
			e["metric"]=*lIt++;
			e["mask"]=iptostring(*lIt++);
			e["mtu"]=*lIt++;
			e["window"]=*lIt++;
			e["irtt"]=*lIt++;
			entries.push_back(e);
		}
	}
}

void Route::dump_map(const Hash& p){
	for(Hash::const_iterator mIt=p.begin();mIt!=p.end();mIt++){
		cout << "["<<(*mIt).first<<"] ["<<(*mIt).second<<"]"<<endl;
	}
	cout << endl;
}

Route::Route(){
	parse_route();
}

Route& Route::Instance(){
	static Route r;
	return r;
}

void Route::Dump(){
	for(Entries::const_iterator eIt=entries.begin();eIt!=entries.end();eIt++){
		dump_map(*eIt);
	}
}

map<string,string> Route::Default(){
	Hash h;
	for(Entries::iterator eIt=entries.begin();eIt!=entries.end();eIt++){
		if( (*eIt)["destination"]=="0.0.0.0"){
			h=*eIt;
			break;
		}
	}
	return h;
}

const list<map<string,string> > & NetworkManager::Route::Routes(){
	return entries;
}

void Route::Refresh(){
	this->entries.clear();
	this->parse_route();
}
}
