/*

    libeutils - http://www.excito.com/

    Services.cpp - this file is part of libeutils.

    Copyright (C) 2009 Tor Krill <tor@excito.com>

    libeutils is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2
    as published by the Free Software Foundation.

    libeutils is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    version 2 along with libeutils; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.

    $Id$
*/

#include "Services.h"
#include "FileUtils.h"
#include "StringTools.h"

#include <stdlib.h>

#include <sstream>

namespace EUtils{
namespace Services{

static int do_call(const string& cmd){
	int ret=system(cmd.c_str());
	if(ret<0){
		return ret;
	}
	return WEXITSTATUS(ret);
}

pid_t GetPid(const string& service){
	string pidfile;
	pid_t pid=0;

	//Todo: something more intelligent
	if(service=="fetchmail"){
		pidfile="/var/run/fetchmail/.fetchmail.pid";
	}else if(service=="avahi-daemon"){
		pidfile="/var/run/avahi-daemon/pid";
	}else{
		pidfile="/var/run/"+service+".pid";
	}

	if(Stat::FileExists(pidfile)){
		string s_pid=StringTools::Chomp(FileUtils::GetContentAsString(pidfile));
		if(Stat::DirExists("/proc/"+s_pid)){
			pid=atoi(s_pid.c_str());
		}
	}
	return pid;
}


bool IsRunning(const string& service){

	return GetPid(service)!=0;
}

bool IsEnabled(const string& service){
	list<string> res=FileUtils::Glob("/etc/runlevels/default/"+service);
	return res.size()>0;
}

bool Start(const string& service){
	return do_call("/etc/init.d/"+service+" start >/dev/null")==0;
}

bool Stop(const string& service){
	return do_call("/etc/init.d/"+service+" -D stop >/dev/null")==0;
}

bool Reload(const string& service){
	return do_call("/etc/init.d/"+service+" -D reload >/dev/null")==0;
}

bool Enable(const string& service, int slevel, int klevel){
	return do_call("/sbin/rc-update add "+service+" default >/dev/null")==0;
}

bool Enable(const string& service, int sprio,const list<int>& slev, int kprio, const list<int>& klev){
	return do_call("/sbin/rc-update add "+service+" default >/dev/null")==0;
}

bool Disable(const string& service){
	return do_call("/sbin/rc-update del "+service+" default >/dev/null")==0;
}

}
}
