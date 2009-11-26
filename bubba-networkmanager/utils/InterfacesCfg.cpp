/*
    
    bubba-networkmanager - http://www.excito.com/
    
    InterfacesCfg.cpp - this file is part of bubba-networkmanager.
    
    Copyright (C) 2009 Tor Krill <tor@excito.com>
    
    bubba-networkmanager is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2
    as published by the Free Software Foundation.
    
    bubba-networkmanager is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    You should have received a copy of the GNU General Public License
    version 2 along with libeutils; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
    
    $Id$
*/

#include <iostream>
#include <sstream>
using namespace std;

#include "InterfacesCfg.h"

#include <libeutils/FileUtils.h>
#include <libeutils/StringTools.h>
using namespace EUtils;

namespace NetworkManager{

InterfacesCfg::InterfacesCfg():cfg(Json::objectValue){
	this->parse_cfg();
}

InterfacesCfg& InterfacesCfg::Instance(){
	static InterfacesCfg cfg;

	return cfg;
}

void InterfacesCfg::parse_cfg(){
	list<string> fil=FileUtils::GetContent(IFSFILE);
	string curif;
	for(list<string>::iterator fIt=fil.begin();fIt!=fil.end();fIt++){
		string line=StringTools::Trimmed(*fIt," \t");
		if(line=="" or line[0]=='#'){
			continue;
		}
		list<string> words=StringTools::Split(*fIt,"[ \t]");
		if(words.size()>1){
			if(words.front()=="auto"){
				curif=words.back();
				this->cfg[curif]["auto"]=true;
			}else if(words.front()=="iface"){
				words.pop_front();
				curif=words.front();
				this->cfg[curif]["addressing"]=words.back();
			}else{
				string key=words.front();
				words.pop_front();
				for(list<string>::iterator sIt=words.begin();sIt!=words.end();sIt++){
					this->cfg[curif]["options"][key].append(*sIt);
				}
			}
		}
	}
}

Json::Value InterfacesCfg::GetCFG(){
	return this->cfg;
}

bool InterfacesCfg::UpdateCFG(const Json::Value& val){
	this->cfg=val;
	return true;
}

bool InterfacesCfg::Commit(){

	Json::Value::Members mem=this->cfg.getMemberNames();
	stringstream ss;
	list<string> res;
	for(Json::Value::Members::iterator mIt=mem.begin();mIt!=mem.end();mIt++){
		Json::Value val=this->cfg[*mIt];
		if(val.isMember("auto")){
			res.push_back("auto "+*mIt+"\n");
		}
		res.push_back("iface "+*mIt+" inet "+val["addressing"].asString()+"\n");

		Json::Value::Members opts=val["options"].getMemberNames();
		for(Json::Value::Members::iterator oIt=opts.begin();oIt!=opts.end();oIt++){

			ss << "\t"<<*oIt;
			Json::Value opval=val["options"][*oIt];
			for(size_t i=0; i<val["options"][*oIt].size();i++){
				ss << " "<< val["options"][*oIt][i].asString();
			}
			ss<<endl;
			res.push_back(ss.str());
			ss.str("");
;		}

		res.push_back("\n");
	}
	FileUtils::Write(IFSFILE,res,0644);
	return true;
}


InterfacesCfg::~InterfacesCfg(){

}

}


