/*
    
    bubba-networkmanager - http://www.excito.com/
    
    Sockios.h - this file is part of bubba-networkmanager.
    
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
 * Sockios.h
 *
 *  Created on: Sep 30, 2009
 *      Author: tor
 */

#ifndef SOCKIOS_H_
#define SOCKIOS_H_

#include <string>
#include <list>

#include <libeutils/json/json.h>

using namespace std;
namespace NetworkManager{

namespace Sockios{

	list<string> GetInterfaceNames();
	Json::Value GetConfig(const string& device);
	bool SetConfig(const string& device, const Json::Value& value);

}

}
#endif /* SOCKIOS_H_ */
