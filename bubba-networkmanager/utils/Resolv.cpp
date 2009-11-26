/*

    bubba-networkmanager - http://www.excito.com/

    Resolv.cpp - this file is part of bubba-networkmanager.

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
#include <iostream>

#include "Resolv.h"

#include <libeutils/FileUtils.h>
#include <libeutils/StringTools.h>

using namespace EUtils;

namespace NetworkManager{

Resolv::Resolv(){
	this->parse_cfg();
}

Resolv& Resolv::Instance(){
	static Resolv r;
	
	return r;
}

void Resolv::parse_cfg(){
	list<string> fil=FileUtils::GetContent(NSFILE);
	for(list<string>::iterator fIt=fil.begin();fIt!=fil.end();fIt++){
		if( (*fIt)[0]=='#'){
			continue;
		}
		list<string> line=StringTools::Split(*fIt,"[ \t]");
		if(line.size()>1){
			if(line.front()=="nameserver"){
				ns.push_back(line.back());
			}else if(line.front()=="domain"){
				this->domain=line.back();
			}else if(line.front()=="search"){
				this->search=line.back();
			}
		}
	}

}

const string& Resolv::Domain(){
	return this->domain;
}

const string& Resolv::Search(){
	return this->search;
}

const list<string>& Resolv::NS(){
	return this->ns;
}


void Resolv::Refresh(){
	this->ns.clear();
	this->search="";
	this->domain="";
	this->parse_cfg();
}

void Resolv::SetNS(const list<string>& nss){
	this->ns=nss;
}
void Resolv::SetDomain(const string& domain){
	this->domain=domain;
}
void Resolv::SetSearch(const string& search){
	this->search=search;
}

void Resolv::Write(){
	list<string> out;
	out.push_back("domain "+this->domain+"\n");
	out.push_back("search "+this->search+"\n");
	for(list<string>::iterator lIt=ns.begin();lIt!=ns.end();lIt++){
		out.push_back("nameserver "+(*lIt)+"\n");
	}
	if(!FileUtils::Write(NSFILE,out,0644)){
		//TODO: Errror
	}
}


Resolv::~Resolv(){
}

}
