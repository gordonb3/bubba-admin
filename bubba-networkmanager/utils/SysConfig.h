/*
    
    bubba-networkmanager - http://www.excito.com/
    
    SysConfig.h - this file is part of bubba-networkmanager.
    
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
 * SysConfig.h
 *
 *  Created on: Oct 9, 2009
 *      Author: tor
 */

#ifndef SYSCONFIG_H_
#define SYSCONFIG_H_

#include <libeutils/SimpleCfg.h>

using namespace EUtils;

class SysConfig: public SimpleCfg{
protected:
	SysConfig(const string& path);
	SysConfig();

	SysConfig(const SysConfig&);
	const SysConfig& operator=(const SysConfig&);
public:
	static string ConfigFile;
	static SysConfig& Instance();

	virtual ~SysConfig();
};

#endif /* SYSCONFIG_H_ */
