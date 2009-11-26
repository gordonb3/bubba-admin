/*
    
    bubba-networkmanager - http://www.excito.com/
    
    JsonUtils.h - this file is part of bubba-networkmanager.
    
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

/*
 * JsonUtils.h
 *
 *  Created on: Oct 6, 2009
 *      Author: tor
 */

#ifndef JSONUTILS_H_
#define JSONUTILS_H_

#include <libeutils/json/json.h>
#include <vector>
#include <list>
#include <string>

using namespace std;

namespace JsonUtils{

Json::Value toArray(const vector<string>& v);
Json::Value toObject(const map<string,string>& m);
map<string,string> toMap(const Json::Value& val);
Json::Value toArray(const list<string>& v);
list<string> ArrayToList(const Json::Value& val);
vector<string> ArrayToVector(const Json::Value& val);

}

#endif /* JSONUTILS_H_ */
