/*
    
    bubba-networkmanager - http://www.excito.com/
    
    JsonUtils.cpp - this file is part of bubba-networkmanager.
    
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

#include "JsonUtils.h"

namespace JsonUtils{

Json::Value toArray(const vector<string>& v){
	Json::Value ret=Json::Value(Json::arrayValue);

	for(vector<string>::const_iterator vIt=v.begin();vIt!=v.end();vIt++){
		ret.append(*vIt);
	}

	return ret;
}

Json::Value toObject(const map<string,string>& m){
	Json::Value ret(Json::objectValue);
	for(map<string,string>::const_iterator mIt=m.begin();mIt!=m.end();mIt++){
		ret[(*mIt).first]=(*mIt).second;
	}
	return ret;
}

map<string,string> toMap(const Json::Value& val){
	map<string,string> res;
	if(val.isObject()){
		Json::Value::Members members=val.getMemberNames();
		for(Json::Value::Members::const_iterator mIt=members.begin();mIt!=members.end();mIt++){
			if(val[*mIt].isString()){
				res[*mIt]=val[*mIt].asString();
			}
		}
	}
	return res;
}

Json::Value toArray(const list<string>& v){
	Json::Value ret=Json::Value(Json::arrayValue);

	for(list<string>::const_iterator vIt=v.begin();vIt!=v.end();vIt++){
		ret.append(*vIt);
	}

	return ret;
}

list<string> ArrayToList(const Json::Value& val){
	list<string> ret;
	if(val.isArray()){
		for(unsigned int i=0; i<val.size();i++){
			if(val[i].isString()){
				ret.push_back(val[i].asString());
			}
		}
	}
	return ret;
}

vector<string> ArrayToVector(const Json::Value& val){
	vector<string> ret;
	if(val.isArray()){
		for(unsigned int i=0; i<val.size();i++){
			if(val[i].isString()){
				ret.push_back(val[i].asString());
			}
		}
	}
	return ret;
}

}
